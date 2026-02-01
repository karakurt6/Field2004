#include <algorithm>
#include <vector>
#include <iterator>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>

#include <math.h>

#include "ps_data.h"

class seqgen
{
  int n;
public:
  seqgen(int first): n(first) { }
  int operator() ()
  {
    return n++;
  }
};

class quant
{
  double d;
public:
  quant(double dd): d(dd) { }
  coord_type operator() (const coord_type& q) const
  {
    coord_type p;
    p.xcoord = d * int(q.xcoord / d + 0.5);
    p.ycoord = d * int(q.ycoord / d + 0.5);
    return p;
  }
};

std::istream& operator >> (std::istream& in, coord_type& p)
{
  std::string line;
  if (std::getline(in, line))
  {
    std::istringstream rec(line);
    rec >> p.xcoord >> p.ycoord;
  }
  return in;
}

std::ostream& operator << (std::ostream& out, const coord_type& p)
{
  return (out << p.xcoord << ' ' << p.ycoord);
}

void main()
{
  std::ifstream in("data\\samples.txt");
  vertex_type ver;
  std::copy(std::istream_iterator<coord_type>(in), \
    std::istream_iterator<coord_type>(), std::back_inserter(ver));

  std::transform(ver.begin(), ver.end(), ver.begin(), quant(1.0));

  typedef std::vector<int> int_vector;
  int_vector data(ver.size());
  std::generate(data.begin(), data.end(), seqgen(0));

  vertex_type dict;
  lexord(ver, data.size(), &data[0], dict);

  curve_type acc(dict.size()), hull;
  std::generate(acc.begin(), acc.end(), seqgen(0));
  graham_scan(dict, acc, hull);

  std::ofstream post("z1.txt");
  std::copy(dict.begin(), dict.end(), std::ostream_iterator<coord_type>(post, "\n"));

  std::ofstream blank("z1.bln");
  blank << hull.size() << ",1\n";
  for (curve_type::iterator c = hull.begin(); c != hull.end(); ++c)
  {
    blank << dict[*c] << '\n';
  }

}
