#ifndef __CALC_H__
#define __CALC_H__

#include <valarray>

class CFieldDoc;

class Expr
{
public:
  typedef std::valarray<float> farray;
  int Parse(const char* pData);
  farray Eval(int k);
  Expr(CFieldDoc* pDoc);
  ~Expr();
};

bool read_surfer_grid(const CFieldDoc* pDoc, const char* name, double* ff);

#endif