/*************************************************************************
*
* Copyright (C) 2012-2018 Barbara Geller & Ansel Sermersheim
* All rights reserved.
*
* GNU Free Documentation License
*
*************************************************************************/

!> @file
!! The fortran_sort_routines module provides several routines to sort arrays
!<
!
!> @defgroup Fortran library
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