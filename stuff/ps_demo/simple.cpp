#include <iostream>
#include <iomanip>
#include "ps_data.h"
#include "demo.h"

void check_bbox()
{
  coord_type pp[] = 
  {
    { 1, 1 },
    { 4, 4 },
    { 7, 2 },
    { 9, 8 },
    { 3, 7 },

    { 11, 13 },
    { 14, 11 },
    { 17, 14 },
    { 16, 18 },
    { 12, 18 },
    { 13, 14 }

  };

  vertex_type ver(pp, pp + countof(pp));

  curve_type cur1;
  cur1.push_back(0);
  cur1.push_back(1);
  cur1.push_back(2);
  cur1.push_back(3);
  cur1.push_back(1);
  cur1.push_back(4);
  cur1.push_back(0);

  curve_type cur2;
  cur2.push_back(0);
  cur2.push_back(1);
  cur2.push_back(3);
  cur2.push_back(2);
  cur2.push_back(1);
  cur2.push_back(4);
  cur2.push_back(0);

  curve_type cur3;
  cur3.push_back(5);
  cur3.push_back(6);
  cur3.push_back(7);
  cur3.push_back(8);
  cur3.push_back(9);
  cur3.push_back(5);

  curve_type cur4;
  cur4.push_back(5);
  cur4.push_back(6);
  cur4.push_back(7);
  cur4.push_back(10);
  cur4.push_back(9);
  cur4.push_back(5);

  std::cout 
    << "is_wekly_simple(cur1) = " << std::boolalpha << is_weakly_simple(ver, cur1) << '\n'
    << "is_wekly_simple(cur2) = " << std::boolalpha << is_weakly_simple(ver, cur2) << '\n';

	std::cout
    << "is_convex(cur3) = " << std::boolalpha << is_convex(ver, cur3) << '\n'
    << "is_convex(cur4) = " << std::boolalpha << is_convex(ver, cur4) << '\n';

}
