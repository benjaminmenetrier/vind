!----------------------------------------------------------------------
! Module: fieldsio_arome_fa_mod
!> AROME FA reader/writer module
! Author: Benjamin Menetrier
! Copyright 2025 Meteorologisk Institutt
!----------------------------------------------------------------------
module fieldsio_arome_fa_mod

use atlas_module, only: atlas_functionspace_structuredcolumns,atlas_fieldset,atlas_field,atlas_structuredgrid,atlas_real
use fckit_configuration_module, only: fckit_configuration
use fckit_mpi_module, only: fckit_mpi_comm
use fckit_log_module, only: fckit_log
use kinds, only: kind_int, kind_real
use mpl_module, only: mpl_init
use trans_module, only: trans_t

implicit none

! Includes
#include "edir_trans.h"
#include "edist_grid.h"
#include "edist_spec.h"
#include "egath_grid.h"
#include "egath_spec.h"
#include "einv_trans.h"
#include "esetup_trans.h"
#include "etrans_inq.h"

private
public :: fieldsio_arome_fa_read, fieldsio_arome_fa_write

contains

!----------------------------------------------------------------------

subroutine fieldsio_arome_fa_read(conf,comm,fspace,trans,akbk,fset)

implicit none

! Passed variables
type(fckit_configuration),intent(in) :: conf
type(fckit_mpi_comm),intent(in) :: comm
type(atlas_functionspace_structuredcolumns),intent(in) :: fspace
type(trans_t),intent(in) :: trans
type(atlas_fieldset),intent(inout) :: akbk
type(atlas_fieldset),intent(inout) :: fset

! Local variables
integer(kind_int),parameter :: ifile = 11
integer(kind_int) :: irep,imaxlev,imaxtrunc,imaxgl,imaxlon,inbari,ityptr,itronc,kflev
integer(kind_int) :: ivar,nlev,ilev,jlev,ingrib,inbits,istron,ipuila
integer(kind_int) :: nlon,ndgl,nmsmax,nsmax,from(1),nproma,ngpblks
integer(kind_int) :: nprgpew,nprtrv,nprtrw,nprgpns,n_regions_ns,n_regions_ew
integer(kind_int) :: nxExt,nyExt,nx,ny,igpg,ix,iy,inode
integer(kind_int),allocatable :: inlopa(:),inozpa(:),nloen(:)
integer(kind_int),allocatable :: i_regions(:)
real(kind_real) :: dx,dy,zslapo,zclopo,zslopo,zcodil,zref,zeps,zundf
real(kind_real),allocatable :: zsinla(:),zvalh(:),zvbh(:)
real(kind_real),allocatable :: zgpg(:,:),zspg(:,:),zgp(:,:,:),zsp(:,:)
real(kind_real),pointer :: ak_ptr(:),bk_ptr(:),ptr(:,:)
character(len=256) :: clfile
character(len=16) :: clframe
character(len=1024) :: message,name,prefix,suffix
character(len=:),allocatable :: str
logical :: lgard,found,lexist,lcosp,lundf
type(atlas_structuredgrid) :: grid
type(atlas_field) :: ak,bk,field
type(fckit_configuration) :: arome_var
type(fckit_configuration),allocatable :: arome_vars(:)

if (comm%rank() == 0) then
  ! Open file
  call conf%get_or_die("filepath",str)
  clfile = str
  WRITE(clframe,'(''CADRE_LECTURE'',I3.3)') 1
  call faitou(irep,ifile,.true.,clfile,'OLD',.true.,.true.,0,1,inbari,clframe)
  call lfimst(irep,ifile,.false.)

  ! Get file limits
  call falimu(imaxlev,imaxtrunc,imaxgl,imaxlon)

  ! Allocation
  allocate(inlopa(imaxgl))
  allocate(inozpa(imaxgl))
  allocate(zsinla(imaxgl))
  allocate(zvalh(0:imaxlev))
  allocate(zvbh(0:imaxlev))

  ! Read file characteristics
  kflev=imaxlev
  lgard=.false.
  call facies(clframe,ityptr,zslapo,zclopo,zslopo,zcodil,itronc,&
   & ndgl,nlon,inlopa,inozpa,zsinla,kflev,zref,zvalh,&
   & zvbh,lgard)

  ! Check number of levels
  if (kflev > imaxlev) then
    call abor1_ftn('max. number of level in model too small')
  endif

  ! Test file format
  if(zsinla(1) >= 0.0_kind_real) then
    call abor1_ftn("old eggx frame format")
  end if

  ! Get sizes
  nsmax = inozpa(1)
  nmsmax = inozpa(2)
  dx = zsinla(7)
  dy = zsinla(8)

  ! Compare file and transform sizes
  if (nlon /= trans%nlon) call abor1_ftn("inconsistent nlon")
  if (ndgl /= trans%ndgl) call abor1_ftn("inconsistent ndgl")
  if (nsmax /= trans%nsmax) call abor1_ftn("inconsistent nsmax")
  if (nmsmax /= trans%nmsmax) call abor1_ftn("inconsistent nmsmax")

  ! Copy ak/bk
  ak = atlas_field("ak",atlas_real(kind_real),(/kflev+1/))
  bk = atlas_field("bk",atlas_real(kind_real),(/kflev+1/))
  call akbk%add(ak)
  call akbk%add(bk)
  call ak%data(ak_ptr)
  call bk%data(bk_ptr)
  do ilev=0,kflev
    ak_ptr(ilev+1) = zvalh(ilev)
    bk_ptr(ilev+1) = zvbh(ilev)
  end do

  ! Get extension zone
  if (.not.conf%get("x extension", nxExt)) nxExt = 0
  if (.not.conf%get("y extension", nyExt)) nyExt = 0
  nx = trans%nlon-nxExt
  ny = trans%ndgl-nyExt
end if

! Set nproma/ngpblks
nproma = trans%ngptot
ngpblks = 1

! Allocation
if (comm%rank() == 0) then
  allocate(zgpg(trans%ngptotg,1))
  allocate(zspg(1,trans%nspec2g))
end if
allocate(zgp(trans%ngptot,1,1))
allocate(zsp(1,trans%nspec2))
from = 1

! Get ATLAS grid
grid = fspace%grid()

! Get variables to read
call conf%get_or_die("arome variables",arome_vars)

! Loop over fields
do ivar=1,size(arome_vars)
  if (comm%rank() == 0) then
    ! Get arome variable properties
    arome_var = arome_vars(ivar)
    call arome_var%get_or_die("name",str)
    name = str
    call arome_var%get_or_die("prefix",str)
    prefix = str
    call arome_var%get_or_die("levels",nlev)
    call arome_var%get_or_die("suffix",str)
    suffix = str

    ! Get field
    field = fset%field(name)

    ! Check horizontal dimension
    if (field%shape(2) /= nx*ny) call abor1_ftn("wrong horizontal dimension")

    ! Check number of levels
    if (nlev == 0) then
      if (field%levels() /= 1) call abor1_ftn("wrong vertical dimension")
    else
      if (field%levels() /= nlev) call abor1_ftn("wrong vertical dimension")
    end if
  end if

  ! Broadcast number of levels
  call comm%broadcast(nlev,0)

  ! Loop over levels
  do ilev=1,max(nlev,1)
    if (comm%rank() == 0) then
      ! Get jlev
      if (nlev == 0) then
        jlev = 0
      else
        jlev = ilev
      end if

      ! Get 2D field info
      call fanion(irep,ifile,trim(prefix),jlev,trim(suffix),lexist,lcosp,ingrib,inbits,istron,ipuila)
    end if

    ! Broadcast flags
    call comm%broadcast(lexist,0)
    call comm%broadcast(lcosp,0)

    ! Check 2D field existence
    if (.not.lexist) then
      call abor1_ftn("field does not exist")
    end if

    ! Check 2D field storage (grid-point or spectral)
    if (lcosp) then
      ! Read spectral field
      if (comm%rank() == 0) call facilo(irep,ifile,trim(prefix),jlev,trim(suffix),zspg(:,1),lcosp,lundf,zundf)

      if (comm%rank() == 0) then
        ! Scatter spectral field (send)
        call edist_spec(pspecg = zspg, &
                      & kfdistg = 1, &
                      & kfrom = from, &
                      & kresol = trans%handle, &
                      & pspec = zsp)
      else
        ! Scatter spectral field (receive)
        call edist_spec(kfdistg = 1, &
                      & kfrom = from, &
                      & kresol = trans%handle, &
                      & pspec = zsp)
      end if

      ! Inverse spectral transform
      call einv_trans(kresol = trans%handle, &
                    & kproma = nproma, &
                    & ldscders = .false., &
                    & pspscalar = zsp, &
                    & pgp = zgp)

      if (comm%rank() == 0) then
        ! Gather grid-point field (receive)
        call egath_grid(kresol = trans%handle, &
                      & kfgathg = 1, &
                      & kto = from, &
                      & kproma = nproma, &
                      & pgp = zgp, &
                      & pgpg = zgpg)
      else
        ! Gather grid-point field (send)
        call egath_grid(kresol = trans%handle, &
                      & kfgathg = 1, &
                      & kto = from, &
                      & kproma = nproma, &
                      & pgp = zgp)
      end if
    else
      ! Read grid-point field
      if (comm%rank() == 0) call facilo(irep,ifile,trim(prefix),jlev,trim(suffix),zgpg(:,1),lcosp,lundf,zundf)
    end if

    ! Copy data
    if (comm%rank() == 0) then
      call field%data(ptr)
      do igpg=1,trans%ngptotg
        iy = (igpg-1)/trans%nlon+1
        ix = igpg-(iy-1)*trans%nlon
        if ((ix <= nx).and.(iy <= ny)) then
          inode = grid%index(ix,iy)
          ptr(ilev,inode) = zgpg(igpg,1)
        end if
      end do
    end if
  end do
end do

if (comm%rank() == 0) then
  ! Release memory
  deallocate(inlopa)
  deallocate(inozpa)
  deallocate(zsinla)
  deallocate(zvalh)
  deallocate(zvbh)
  deallocate(arome_vars)
  deallocate(zgpg)
  deallocate(zspg)

  ! Close file
  call fairme(irep,ifile,'UNKNOWN')
end if

! Release memory
deallocate(zgp)
deallocate(zsp)

call comm%barrier()

end subroutine fieldsio_arome_fa_read

!----------------------------------------------------------------------

subroutine fieldsio_arome_fa_write(conf,comm,fspace,trans,fset)

implicit none

! Passed variables
type(fckit_configuration),intent(in) :: conf
type(fckit_mpi_comm),intent(in) :: comm
type(atlas_functionspace_structuredcolumns),intent(in) :: fspace
type(trans_t),intent(in) :: trans
type(atlas_fieldset),intent(inout) :: fset

! Local variables
integer(kind_int),parameter :: ifile = 11
integer(kind_int) :: irep,imaxlev,imaxtrunc,imaxgl,imaxlon,inbpdg,inbcsp,idmopl,inbari,ityptr,itronc,kflev
integer(kind_int) :: ivar,nlev,ilev,jlev,ingrib,inbits,istron,ipuila
integer(kind_int) :: nlon,ndgl,nmsmax,nsmax,from(1),nproma,ngpblks
integer(kind_int) :: nprgpew,nprtrv,nprtrw,nprgpns,n_regions_ns,n_regions_ew
integer(kind_int) :: nxExt,nyExt,nx,ny,igpg,ix,iy,inode
integer(kind_int),allocatable :: inlopa(:),inozpa(:),nloen(:)
integer(kind_int),allocatable :: i_regions(:)
real(kind_real) :: dx,dy,zslapo,zclopo,zslopo,zcodil,zref,zeps
real(kind_real),allocatable :: zsinla(:),zvalh(:),zvbh(:)
real(kind_real),allocatable :: zgpg(:,:),zspg(:,:),zgp(:,:,:),zsp(:,:)
real(kind_real),pointer :: ak_ptr(:),bk_ptr(:),ptr(:,:)
character(len=256) :: clfile
character(len=16) :: clframe
character(len=1024) :: message,name,prefix,suffix
character(len=:),allocatable :: str
logical :: lgard,found,lexist,lcosp
type(atlas_structuredgrid) :: grid
type(atlas_field) :: ak,bk,field
type(fckit_configuration) :: arome_var
type(fckit_configuration),allocatable :: arome_vars(:)

if (comm%rank() == 0) then
  ! Open file
  call conf%get_or_die("filepath",str)
  clfile = str
  WRITE(clframe,'(''CADRE_LECTURE'',I3.3)') 1
  call faitou(irep,ifile,.true.,clfile,'OLD',.true.,.true.,0,1,inbari,clframe)
  call lfimst(irep,ifile,.false.)

  ! Get file limits
  call falimu(imaxlev,imaxtrunc,imaxgl,imaxlon)

  ! Get global packing default
  call favori(ingrib,inbpdg,inbcsp,istron,ipuila,idmopl)

  ! Allocation
  allocate(inlopa(imaxgl))
  allocate(inozpa(imaxgl))
  allocate(zsinla(imaxgl))
  allocate(zvalh(0:imaxlev))
  allocate(zvbh(0:imaxlev))

  ! Read file characteristics
  kflev=imaxlev
  lgard=.false.
  call facies(clframe,ityptr,zslapo,zclopo,zslopo,zcodil,itronc,&
   & ndgl,nlon,inlopa,inozpa,zsinla,kflev,zref,zvalh,&
   & zvbh,lgard)

  ! Check number of levels
  if (kflev > imaxlev) then
    call abor1_ftn('max. number of level in model too small')
  endif

  ! Test file format
  if(zsinla(1) >= 0.0_kind_real) then
    call abor1_ftn("old eggx frame format")
  end if

  ! Get sizes
  nsmax = inozpa(1)
  nmsmax = inozpa(2)
  dx = zsinla(7)
  dy = zsinla(8)

  ! Compare file and transform sizes
  if (nlon /= trans%nlon) call abor1_ftn("inconsistent nlon")
  if (ndgl /= trans%ndgl) call abor1_ftn("inconsistent ndgl")
  if (nsmax /= trans%nsmax) call abor1_ftn("inconsistent nsmax")
  if (nmsmax /= trans%nmsmax) call abor1_ftn("inconsistent nmsmax")

  ! Get extension zone
  if (.not.conf%get("x extension", nxExt)) nxExt = 0
  if (.not.conf%get("y extension", nyExt)) nyExt = 0
  nx = trans%nlon-nxExt
  ny = trans%ndgl-nyExt
end if

! Set nproma/ngpblks
nproma = trans%ngptot
ngpblks = 1

! Allocation
if (comm%rank() == 0) then
  allocate(zgpg(trans%ngptotg,1))
  allocate(zspg(1,trans%nspec2g))
end if
allocate(zgp(trans%ngptot,1,1))
allocate(zsp(1,trans%nspec2))
from = 1

! Get ATLAS grid
!grid = fspace%grid()

! Get variables to read
call conf%get_or_die("arome variables",arome_vars)

! Loop over fields
do ivar=1,size(arome_vars)
  if (comm%rank() == 0) then
    ! Get arome variable properties
    arome_var = arome_vars(ivar)
    call arome_var%get_or_die("name",str)
    name = str
    call arome_var%get_or_die("prefix",str)
    prefix = str
    call arome_var%get_or_die("levels",nlev)
    call arome_var%get_or_die("suffix",str)
    suffix = str

    ! Get field
    field = fset%field(name)

    ! Check horizontal dimension
    if (field%shape(2) /= trans%ngptotg) call abor1_ftn("wrong horizontal dimension")

    ! Check number of levels
    if (nlev == 0) then
      if (field%levels() /= 1) call abor1_ftn("wrong vertical dimension")
    else
      if (field%levels() /= nlev) call abor1_ftn("wrong vertical dimension")
    end if
  end if

  ! Broadcast number of levels
  call comm%broadcast(nlev,0)

  ! Loop over levels
  do ilev=1,max(nlev,1)
    if (comm%rank() == 0) then
      ! Get jlev
      if (nlev == 0) then
        jlev = 0
      else
        jlev = ilev
      end if

      ! Get 2D field info
      call fanion(irep,ifile,trim(prefix),jlev,trim(suffix),lexist,lcosp,ingrib,inbits,istron,ipuila)
    end if

    ! Broadcast flags
    call comm%broadcast(lexist,0)
    call comm%broadcast(lcosp,0)

    ! Check 2D field existence
    if (.not.lexist) then
      call abor1_ftn("field does not exist")
    end if

    ! Copy data
    if (comm%rank() == 0) then
      call field%data(ptr)
      do igpg=1,trans%ngptotg
        iy = (igpg-1)/trans%nlon+1
        ix = igpg-(iy-1)*trans%nlon
        if ((ix <= nx).and.(iy <= ny)) then
          inode = grid%index(ix,iy)
          zgpg(igpg,1) = ptr(ilev,inode)
        end if
      end do
    end if

    ! Transfer packing info
    if (comm%rank() == 0) call fagote(irep,ifile,ingrib,inbits,inbits,istron,ipuila,idmopl)

    ! Check 2D field storage (grid-point or spectral)
    if (lcosp) then
      if (comm%rank() == 0) then
        ! Scatter grid-point field (send)
        call edist_grid(kresol = trans%handle, &
                      & kfdistg = 1, &
                      & kfrom = from, &
                      & kproma = nproma, &
                      & pgp = zgp, &
                      & pgpg = zgpg)
      else
        ! Scatter grid-point field (receive)
        call edist_grid(kresol = trans%handle, &
                      & kfdistg = 1, &
                      & kfrom = from, &
                      & kproma = nproma, &
                      & pgp = zgp)
      end if

      ! Direct spectral transform
      call edir_trans(kresol = trans%handle, &
                    & kproma = nproma, &
                    & pspscalar = zsp, &
                    & pgp = zgp)

      ! Spectral field
      if (comm%rank() == 0) then
        ! Gather spectral field (receive)
        call egath_spec(pspecg = zspg, &
                      & kfgathg = 1, &
                      & kto = from, &
                      & kresol = trans%handle, &
                      & pspec = zsp)
      else
        ! Gather spectral field (send)
        call egath_spec(kfgathg = 1, &
                      & kto = from, &
                      & kresol = trans%handle, &
                      & pspec = zsp)
      end if

      ! Write spectral field
      if (comm%rank() == 0) call faienc(irep,ifile,trim(prefix),jlev,trim(suffix),zspg(:,1),lcosp)
    else
      ! Write grid-point field
      if (comm%rank() == 0) call faienc(irep,ifile,trim(prefix),jlev,trim(suffix),zgpg(:,1),lcosp)
    end if
  end do
end do

if (comm%rank() == 0) then
  ! Release memory
  deallocate(inlopa)
  deallocate(inozpa)
  deallocate(zsinla)
  deallocate(zvalh)
  deallocate(zvbh)
  deallocate(arome_vars)
  deallocate(zgpg)
  deallocate(zspg)

  ! Close file
  call fairme(irep,ifile,'KEEP')
end if

! Release memory
deallocate(zgp)
deallocate(zsp)

call comm%barrier()

end subroutine fieldsio_arome_fa_write

!----------------------------------------------------------------------

end module fieldsio_arome_fa_mod
