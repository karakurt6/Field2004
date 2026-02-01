real(8) function split_edge(e1, e2)
!dec$ attributes stdcall, alias: '_SPLIT_EDGE@8' :: split_edge
  include 'wellgrid.fh'

  type(well_type), pointer :: e1, e2

  !if (e1%name(1:1)==""C) then
  !  print *, 'outer'
  !else
  !  print *, 'inner'
  !endif
  !print *, e1%top_x, e1%top_y, e2%top_x, e2%top_y
  
  split_edge = 0.5
end function

program main
  include 'wellgrid.fh'
  
  parameter (x1 = 1388105.0, x2 = 1428355.0, y1 = 706905.0, y2 = 784655.0)
  !parameter (x1=1456000.0, x2=1512000.0, y1=697300.0, y2=759300.0);
  parameter (cx=350, cy=650)

  external split_edge
  !dec$ attributes stdcall, alias: '_SPLIT_EDGE@8' :: split_edge

  type(well_type) a(10000)
  integer i, j, k, h
  character*8 name
  real p(2), acc(cx, cy), s, t

  h = wg_create_uniform(cx, cy, x1, x2, y1, y2)

  call random_seed()
  do i=1,996
    write(name,'(I7)') i
    a(i)%name = 'well' // trim(adjustl(name)) // ''C
    call random_number(p)
    a(i)%top_x = x1 + (x2 - x1) * p(1)
    a(i)%top_y = y1 + (y2 - y1) * p(2)
    call wg_update(h, loc(a(i)))
  end do

  t = secnds(0.0)
  call wg_compute(h, split_edge)
  print *, secnds(t)
  call wg_destroy(h)
  stop

  !acc = 0.0
  !do k=1,10
  !  do j=1,10
  !    do i=1,10
  !      s = wg_info(h, loc(a(k)), i, j)
  !      acc(i,j) = acc(i,j) + s
  !      print '(F8.4,\)', s
  !    end do
  !    print *
  !  end do
  !  print *
  !end do

  !print '(10(F8.4,\),/)', acc


end