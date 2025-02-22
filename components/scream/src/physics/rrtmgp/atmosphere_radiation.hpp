#ifndef SCREAM_RRTMGP_RADIATION_HPP
#define SCREAM_RRTMGP_RADIATION_HPP

#include "cpp/rrtmgp/mo_gas_concentrations.h"
#include "physics/rrtmgp/scream_rrtmgp_interface.hpp"
#include "share/atm_process/atmosphere_process.hpp"
#include "ekat/ekat_parameter_list.hpp"
#include "ekat/util/ekat_string_utils.hpp"
#include <string>

namespace scream {
/*
 * Class responsible for atmosphere radiative transfer. The AD should store
 * exactly ONE instance of this class in its list of subcomponents.
 */

class RRTMGPRadiation : public AtmosphereProcess {
public:
  using field_type       = Field<      Real>;
  using const_field_type = Field<const Real>;
  using view_1d_real     = typename ekat::KokkosTypes<DefaultDevice>::template view_1d<Real>;
  using view_2d_real     = typename ekat::KokkosTypes<DefaultDevice>::template view_2d<Real>;
  using ci_string        = ekat::CaseInsensitiveString;

  using KT               = ekat::KokkosTypes<DefaultDevice>;
  template<typename ScalarT>
  using uview_1d         = Unmanaged<typename KT::template view_1d<ScalarT>>;

  // Constructors
  RRTMGPRadiation (const ekat::Comm& comm, const ekat::ParameterList& params);

  // The type of the subcomponent
  AtmosphereProcessType type () const { return AtmosphereProcessType::Physics; }

  // The name of the subcomponent
  std::string name () const { return "Radiation"; }

  // Required grid for the subcomponent (??)
  std::set<std::string> get_required_grids () const {
      static std::set<std::string> s;
      s.insert(m_params.get<std::string>("Grid"));
      return s;
  }

  // Set the grid
  void set_grids (const std::shared_ptr<const GridsManager> grid_manager);

// NOTE: cannot use lambda functions for CUDA devices if these are protected!
public:
  // The three main interfaces for the subcomponent
  void initialize_impl ();
  void run_impl        (const int dt);
  void finalize_impl   ();

  // Keep track of number of columns and levels
  int m_ncol;
  int m_nlay;
  view_1d_real m_lat;
  view_1d_real m_lon;

  // The orbital year, used for zenith angle calculations:
  // If > 0, use constant orbital year for duration of simulation
  // If < 0, use year from timestamp for orbital parameters
  Int m_orbital_year;

  // Fixed solar zenith angle to use for shortwave calculations
  // This is only used if a positive value is supplied
  Real m_fixed_solar_zenith_angle;

  // Need to hard-code some dimension sizes for now. 
  // TODO: find a better way of configuring this
  const int m_nswbands = 14;
  const int m_nlwbands = 16;

  // These are the gases that we keep track of
  int m_ngas;
  std::vector<ci_string>   m_gas_names;
  view_1d_real             m_gas_mol_weights;
  GasConcs gas_concs;

  // Structure for storing local variables initialized using the ATMBufferManager
  struct Buffer {
    static constexpr int num_1d_ncol        = 6;
    static constexpr int num_2d_nlay        = 14;
    static constexpr int num_2d_nlay_p1     = 7;
    static constexpr int num_2d_nswbands    = 2;

    // 1d size (ncol)
    real1d mu0;
    real1d sfc_alb_dir_vis;
    real1d sfc_alb_dir_nir;
    real1d sfc_alb_dif_vis;
    real1d sfc_alb_dif_nir;
    uview_1d<Real> cosine_zenith;

    // 2d size (ncol, nlay)
    real2d p_lay;
    real2d t_lay;
    real2d p_del;
    real2d qc;
    real2d qi;
    real2d cldfrac_tot;
    real2d eff_radius_qc;
    real2d eff_radius_qi;
    real2d tmp2d;
    real2d lwp;
    real2d iwp;
    real2d sw_heating;
    real2d lw_heating;
    real2d rad_heating;

    // 2d size (ncol, nlay+1)
    real2d p_lev;
    real2d t_lev;
    real2d sw_flux_up;
    real2d sw_flux_dn;
    real2d sw_flux_dn_dir;
    real2d lw_flux_up;
    real2d lw_flux_dn;

    // 2d size (ncol, nswbands)
    real2d sfc_alb_dir;
    real2d sfc_alb_dif;
  };

protected:

  // Computes total number of bytes needed for local variables
  int requested_buffer_size_in_bytes() const;

  // Set local variables using memory provided by
  // the ATMBufferManager
  void init_buffers(const ATMBufferManager &buffer_manager);

  // Struct which contains local variables
  Buffer m_buffer;
};  // class RRTMGPRadiation

}  // namespace scream

#endif  // SCREAM_RRTMGP_RADIATION_HPP
