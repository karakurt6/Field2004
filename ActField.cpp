#include "stdafx.h"

#include <math.h>
#include <libh5/hdf5.h>
#include <algorithm>
#include <valarray>

#include "Field.h"
#include "FieldDoc.h"
#include "CumulativeS.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CActField, CFieldDoc)

BEGIN_MESSAGE_MAP(CActField, CFieldDoc)
  //{{AFX_MSG_MAP(CActField)
    // NOTE - the ClassWizard will add and remove mapping macros here.
    //    DO NOT EDIT what you see in these blocks of generated code!
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

CActField::CActField(): func(0)
{
  clut.sh = 0;
  clut.zz = 0;
  year = 0;
}

CActField::~CActField()
{
  delete[] func;
  delete[] clut.zz;
  delete[] clut.sh;
}

static float exp_x_minus_1(float x)
{
  return (float) exp(x) - 1;
}

static float log_1_plus_x(float x)
{
  return (float) log(1 + x);
}

static float id(float x)
{
  return x;
}

void CActField::InitArithmeticClut()
{
  delete[] clut.zz; clut.zz = 0;
  delete[] clut.sh; clut.sh = 0;

  const int n = 5;
  clut.nz = n;
  clut.zz = new float[n+2];
  clut.sh = new CActField::Color[n+2];

  clut.sh[0].r = 0.0, clut.sh[0].g = 0.0, clut.sh[0].b = 0.5;
  clut.sh[1].r = 0.0, clut.sh[1].g = 0.0, clut.sh[1].b = 1.0;
  clut.sh[2].r = 0.0, clut.sh[2].g = 1.0, clut.sh[2].b = 1.0;
  clut.sh[3].r = 0.0, clut.sh[3].g = 1.0, clut.sh[3].b = 0.0;
  clut.sh[4].r = 1.0, clut.sh[4].g = 1.0, clut.sh[4].b = 0.0;
  clut.sh[5].r = 1.0, clut.sh[5].g = 0.0, clut.sh[5].b = 0.0;
  clut.sh[6].r = 1.0, clut.sh[6].g = 0.5, clut.sh[6].b = 0.5;

  if (field->color_mapping == 1)
  {
    clut.fwd = log_1_plus_x;
    clut.bck = exp_x_minus_1;
  }
  else
  {
    clut.fwd = clut.bck = id;
  }

  float f1 = clut.fwd(field->valid_range[0]);
  float f2 = clut.fwd(field->valid_range[1]);
  for (int i = 0; i <= (n+1); ++i) 
    clut.zz[i] = f1 + i * (f2 - f1) / (n+1);
}

BOOL CActField::OnNewDocument()
{
  if (!CFieldDoc::OnNewDocument())
    return FALSE;

  FieldEnum(1, &field);
  level = 0;
  func = new float[cx*cy];
  InitArithmeticClut();
  UpdateSelection();

  return TRUE;
}

void CActField::DeleteContents()
{
  delete[] func; func = 0;
  delete[] clut.zz; clut.zz = 0;
  delete[] clut.sh; clut.sh = 0;

  CFieldDoc::DeleteContents();
}

void CActField::UpdateSelection()
{
  CCumulativeS* pSat = GetCumulativeS();
  if (pSat && pSat->IsWindowVisible())
  {
    pSat->UpdateSelection();
    return;
  }

  hid_t f_space, m_space;
  hsize_t start[4];
  hsize_t count[4];
  herr_t err;
  int nn;

  f_space = H5Dget_space(field->id);
  int n_dims = H5Sget_simple_extent_dims(f_space, count, NULL);
  if (n_dims == 3)
  {
    start[0] = level;
    start[1] = start[2] = 0;
    count[0] = 1;
    err = H5Sselect_hyperslab(f_space, H5S_SELECT_SET, start, NULL, count, NULL);
    nn = cx * cy;
    count[0] = nn;
    m_space = H5Screate_simple(1, count, NULL);
    err = H5Dread(field->id, H5T_NATIVE_FLOAT, m_space, f_space, H5P_DEFAULT, func);
    err = H5Sclose(m_space);
    err = H5Sclose(f_space);
  }
  else if (n_dims == 4)
  {
    hid_t t_attr = H5Aopen_name(field->id, "time");
    int nt = (int) count[0];
    short* tt = new short[nt];
    err = H5Aread(t_attr, H5T_NATIVE_SHORT, tt);
    err = H5Aclose(t_attr);
    
    if (year < tt[0])
    {
      start[0] = 0;
    }
    else if (year > tt[nt-1])
    {
      start[0] = nt-1;
    }
    else
    {
      start[0] = std::lower_bound(tt, tt+nt, year) - tt;
    }
    delete[] tt;

    start[1] = level;
    start[2] = start[3] = 0;
    count[0] = count[1] = 1;
    err = H5Sselect_hyperslab(f_space, H5S_SELECT_SET, start, NULL, count, NULL);
    nn = cx * cy;
    count[0] = nn;
    m_space = H5Screate_simple(1, count, NULL);
    err = H5Dread(field->id, H5T_NATIVE_FLOAT, m_space, f_space, H5P_DEFAULT, func);
    err = H5Sclose(m_space);
    err = H5Sclose(f_space);
  }
  else
  {
    ASSERT(FALSE);
  }

  std::transform(func, func+nn, func, clut.fwd);
  UpdateAllViews(0);
}

void CActField::UpdateYear(short s)
{
  if (year != s)
  {
    year = s;
    UpdateSelection();
  }
  year = s;
}

void CActField::SelectProp(int index)
{
  const int nn = 20;
  const DataField* pp[nn];
  int n = FieldEnum(nn, pp);
  ASSERT(n <= nn);

  field = pp[index];
  InitArithmeticClut();
  UpdateSelection();
  UpdateAllViews(0, 1);
}

int CActField::FieldInfo(long b, int n, double *p) const
{
  ASSERT(n >= 0);

  int nn = Info(b, 0, 0);
  if (nn <= 0)
    return 0;

  const float InfoItem::* ptr;
  if (stricmp(field->name, "a_sp") == 0)
  {
    ptr = &InfoItem::a_sp;
  }
  else if (stricmp(field->name, "perm") == 0)
  {
    ptr = &InfoItem::perm;
  }
  else if (stricmp(field->name, "poro") == 0)
  {
    ptr = &InfoItem::poro;
  }
  else
  {
    return 0;
  }
  
  std::valarray<InfoItem*> q(nn);
  Info(b, nn, &q[0]);
  for (int i = 0; i < nn; ++i)
  {
    if (!n--)
      break;

    *p++ = q[i]->top;
    *p++ = q[i]->bot;
    *p++ = q[i]->*ptr;
  }
  return nn;
}

#ifdef _DEBUG
void CActField::AssertValid() const
{
  CFieldDoc::AssertValid();
}

void CActField::Dump(CDumpContext& dc) const
{
  CFieldDoc::Dump(dc);

  dc << "level = " << level << "\ncolor map entries = " << clut.nz << "\n";
  for (int i = 0; i < clut.nz; ++i)
  {
    dc << clut.zz[i] << " (" << clut.sh[i].r << ", " \
      << clut.sh[i].g << ", " << clut.sh[i].b << ")\n";
  }
}
#endif
