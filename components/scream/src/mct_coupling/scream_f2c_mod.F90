module scream_f2c_mod

  implicit none

  ! This module has the sole purpose of listing the f90<->C interfaces.
  ! Other modules can simply "use" the interfaces from here, so that
  ! the code can be slimmer there.

interface

  ! This subroutine performs the first part of scream *internal* initialization,
  ! namely creating the grids, the atm processes, and the fields.
  ! It does *NOT* initialize the fields and the atm processes, nor it initializes
  ! any structure related to the component coupler. Other subroutines
  ! will have to be called *after* this one, to achieve that.
  subroutine scream_create_atm_instance (f_comm,yaml_fname) bind(c)
    use iso_c_binding, only: c_int, c_char
    !
    ! Input(s)
    !
    integer (kind=c_int),   intent(in) :: f_comm
    character(kind=c_char), target, intent(in) :: yaml_fname(*)
  end subroutine scream_create_atm_instance

  subroutine scream_get_cols_latlon (lat, lon) bind(c)
    use iso_c_binding, only: c_ptr
    !
    ! arguments
    !
    type(c_ptr), intent(in) :: lat, lon
  end subroutine scream_get_cols_latlon

  subroutine scream_get_cols_area (area) bind(c)
    use iso_c_binding, only: c_ptr
    !
    ! arguments
    !
    type(c_ptr), intent(in) :: area
  end subroutine scream_get_cols_area

  ! This subroutine initializes the C++ structures in the AD that are
  ! responsible to handle import/export operation from/into the component
  ! coupler surface fluxes/state structures
  subroutine scream_setup_surface_coupling (x2a_names, x2a_indices, x2a_ptr, vec_comp_x2a, &
                                            num_cpl_imports, num_scream_imports, &
                                            a2x_names, a2x_indices, a2x_ptr, vec_comp_a2x, &
                                            can_be_exported_during_init, num_cpl_exports) bind(c)
    use iso_c_binding, only: c_ptr, c_int
    !
    ! Input(s)
    !
    type(c_ptr),         intent(in) :: x2a_indices, x2a_names, x2a_ptr, vec_comp_x2a
    type(c_ptr),         intent(in) :: a2x_indices, a2x_names, a2x_ptr, vec_comp_a2x, can_be_exported_during_init
    integer(kind=c_int), intent(in) :: num_cpl_imports, num_scream_imports, num_cpl_exports
  end subroutine scream_setup_surface_coupling

  ! This subroutine performs completes the initialization of the atm instance.
  ! In particular, this routine must be called *after* scream_create_atm_instance,
  ! and *after* scream_setup_surface_coupling.
  ! During this call, all fields are initialized (i.e., initial conditions are
  ! loaded), as well as the atm procs (which might use some initial conditions
  ! to further initialize internal structures), and the output manager.
  subroutine scream_init_atm (start_ymd,start_tod) bind(c)
    use iso_c_binding, only: c_int
    !
    ! Input(s)
    !
    integer (kind=c_int),   intent(in) :: start_tod, start_ymd
  end subroutine scream_init_atm

  ! This subroutine will run the whole atm model for one atm timestep
  subroutine scream_run (dt) bind(c)
    use iso_c_binding, only: c_double
    !
    ! arguments
    !
    real(kind=c_double), intent(in) :: dt
  end subroutine scream_run

  subroutine scream_finalize () bind(c)
  end subroutine scream_finalize

  function scream_get_num_local_cols () result(nlcols) bind(c)
    use iso_c_binding, only: c_int
    !
    ! Result
    !
    integer(kind=c_int) :: nlcols
  end function scream_get_num_local_cols

  function scream_get_num_global_cols () result(ngcols) bind(c)
    use iso_c_binding, only: c_int
    !
    ! Result
    !
    integer(kind=c_int) :: ngcols
  end function scream_get_num_global_cols

  subroutine scream_get_local_cols_gids (gids) bind(c)
    use iso_c_binding, only: c_ptr
    !
    ! Input(s)
    !
    type(c_ptr), intent(in), VALUE :: gids
    ! integer(kind=c_int), intent(inout) :: gids (:)
  end subroutine scream_get_local_cols_gids
end interface

end module scream_f2c_mod
