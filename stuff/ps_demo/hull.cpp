#include <iostream>
#include <iomanip>
#include "ps_data.h"
#include "demo.h"

void check_hull()
{
  vertex_type v;
  coord_type p;
  curve_type c;

  p.xcoord = 0.0;
  p.ycoord = 0.0;
  v.push_back(p);
  c.push_back(0);

  p.xcoord = 1.0;
  p.ycoord = 0.0;
  v.push_back(p);
  c.push_back(1);

  p.xcoord = 1.0;
  p.ycoord = 1.0;
  v.push_back(p);
  c.push_back(2);

  p.xcoord = 0.0;
  p.ycoord = 1.0;
  v.push_back(p);
  c.push_back(3);

  c.push_back(0);

  std::cout << std::boolalpha << inside_curve(0.5, 0.5, v, c) << std::endl;
  std::cout << std::boolalpha << inside_curve(0.0, 0.5, v, c) << std::endl;

  std::cout << std::boolalpha << inside_curve(0.0, 0.0, v, c) << std::endl;
  std::cout << std::boolalpha << inside_curve(0.0, 1.0, v, c) << std::endl;
  std::cout << std::boolalpha << inside_curve(1.0, 1.0, v, c) << std::endl;
  std::cout << std::boolalpha << inside_curve(1.0, 0.0, v, c) << std::endl;

}

