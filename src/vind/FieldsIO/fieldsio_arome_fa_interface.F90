!----------------------------------------------------------------------
! Module: fieldsio_arome_fa_interface
!> AROME FA reader interface
! Author: Benjamin Menetrier
! Copyright 2025 Meteorologisk Institutt
!----------------------------------------------------------------------
module fieldsio_arome_fa_interface

use atlas_module, only: atlas_functionspace_structuredcolumns,atlas_fieldset
use fckit_configuration_module, only: fckit_configuration
use fckit_mpi_module, only: fckit_mpi_comm
use fieldsio_arome_fa_mod, only: fieldsio_arome_fa_read, fieldsio_arome_fa_write
use, intrinsic :: iso_c_binding, only: c_ptr, c_f_pointer
use trans_module, only: trans_t

implicit none

private

contains
!----------------------------------------------------------------------

subroutine fieldsio_arome_fa_read_c(c_conf,c_comm,c_functionspace,c_trans,c_akbk,c_fieldset) &
 & bind(c,name='fieldsio_arome_fa_read_f90')

implicit none

! Passed variables
type(c_ptr),intent(in),value :: c_conf
type(c_ptr),intent(in),value :: c_comm
type(c_ptr),intent(in),value :: c_functionspace
type(c_ptr),intent(in),value :: c_trans
type(c_ptr),intent(in),value :: c_akbk
type(c_ptr),intent(in),value :: c_fieldset

! Local variables
type(fckit_configuration) :: f_conf
type(fckit_mpi_comm) :: f_comm
type(atlas_functionspace_structuredcolumns) :: f_functionspace
type(trans_t), pointer :: f_trans
type(atlas_fieldset) :: f_akbk
type(atlas_fieldset) :: f_fieldset

! Interface
f_conf = fckit_configuration(c_conf)
f_comm = fckit_mpi_comm(c_comm)
f_functionspace = atlas_functionspace_structuredcolumns(c_functionspace)
call c_f_pointer(c_trans,f_trans)
f_akbk = atlas_fieldset(c_akbk)
f_fieldset = atlas_fieldset(c_fieldset)

! Call Fortran
call fieldsio_arome_fa_read(f_conf,f_comm,f_functionspace,f_trans,f_akbk,f_fieldset)

end subroutine fieldsio_arome_fa_read_c

!----------------------------------------------------------------------

subroutine fieldsio_arome_fa_write_c(c_conf,c_comm,c_functionspace,c_trans,c_fieldset) &
 & bind(c,name='fieldsio_arome_fa_write_f90')

implicit none

! Passed variables
type(c_ptr),intent(in),value :: c_conf
type(c_ptr),intent(in),value :: c_comm
type(c_ptr),intent(in),value :: c_functionspace
type(c_ptr),intent(in),value :: c_trans
type(c_ptr),intent(in),value :: c_fieldset

! Local variables
type(fckit_configuration) :: f_conf
type(fckit_mpi_comm) :: f_comm
type(atlas_functionspace_structuredcolumns) :: f_functionspace
type(trans_t), pointer :: f_trans
type(atlas_fieldset) :: f_fieldset

! Interface
f_conf = fckit_configuration(c_conf)
f_comm = fckit_mpi_comm(c_comm)
f_functionspace = atlas_functionspace_structuredcolumns(c_functionspace)
call c_f_pointer(c_trans,f_trans)
f_fieldset = atlas_fieldset(c_fieldset)

! Call Fortran
call fieldsio_arome_fa_write(f_conf,f_comm,f_functionspace,f_trans,f_fieldset)

end subroutine fieldsio_arome_fa_write_c

!----------------------------------------------------------------------

end module fieldsio_arome_fa_interface
