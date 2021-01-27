!************************************************************************
!
! Copyright (c) 2014-2021 Barbara Geller & Ansel Sermersheim
!
! DoxyPress is free software: you can redistribute it and/or
! modify it under the terms of the GNU General Public License version 2
! as published by the Free Software Foundation.
!
! DoxyPress is distributed in the hope that it will be useful,
! but WITHOUT ANY WARRANTY; without even the implied warranty of
! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
!
! Documents produced by DoxyPress are derivative works derived from the
! input used in their production; they are not affected by this license.
!
!*************************************************************************/

!> @file
!! The fortran_sort_routines module provides several routines to sort arrays
!<
!
!> @defgroup Fortran Fortran Library
!! The fortran_sort_routines module provides several routines to sort arrays
!!
module fortran_sort_routines
    implicit none

    !> generic name for the sorting routines
    !! The sort interface allows the programmer to use a single
    !! generic name
    !<
    interface fortran_sort
        module procedure fortran_sort_int
        module procedure fortran_sort_real
    end interface

contains

!>sort array of integers
!<
subroutine fortran_sort_int( array )
    integer, dimension(:) :: array   !< Array to be sorted

end subroutine fortran_sort_int

!>sort array of integers
!<
subroutine fortran_sort_real( array )
    real, dimension(:) :: array      !< Array to be sorted

end subroutine fortran_sort_real

end module fortran_sort_routines