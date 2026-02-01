      subroutine srand
      call random_seed
      end subroutine

      double precision function drand
      call random_number(drand)
      end function

      real function seconds(x)
      real x
      seconds = secnds(x)
      end function