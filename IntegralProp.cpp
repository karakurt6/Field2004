// IntegralProp.cpp : implementation file
//

#include "stdafx.h"
#include <fstream>
#include <string>
#include <sstream>
#include <iterator>
#include <map>
#include <numeric>
#include <algorithm>

#include "field.h"
#include "FieldDoc.h"
#include "FieldView.h"
#include "FieldForm.h"
#include "IntegralProp.h"
#include "stuff\calc.h"
#include "stuff\wellgrid.h"
#include "stuff\ps_data.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

class IntegralProp_Impl
{
public:
  IntegralProp_Impl(CIntegralProp* pp);
  ~IntegralProp_Impl();

  bool Update();
  bool Remove();
  void Print() const;
  void Fill(const char* str) const;
  bool Write(std::ostream& out) const;
  void Populate() const;
  bool Read(std::istream& in);

  float* GetData(const char*);
  CActField::Palette* GetPalette(const char*);

private:
  struct Formation
  {
    char* upperfile;
    char* lowerfile;
    double *uppergrid;
    double *lowergrid;
    char* notation;
    int refcount;
  };

  struct Constraint
  {
    int n_points;
    double *x_coord;
    double *y_coord;
    char* filename;
    char* feature;
    double* gridmask;
    int refcount;
  };

  typedef CActField::Palette Palette;

  struct Gridfunc
  {
    Palette palette;
    Constraint* constraint;
    Formation* formation;
    char* labeling;
    char* expression;
    char* description;
    float* datavalue;
    float datarange[2];
    float integral;
  };

  bool Make(Constraint** ptr);
  bool Update(Constraint** ptr);
  bool Update(Formation** ptr);
  bool Eval(Gridfunc* ptr);

  typedef std::string Key_type;
  typedef std::map<Key_type, Formation*> F_prop;
  typedef std::map<Key_type, Constraint*> C_prop;
  typedef std::map<Key_type, Gridfunc*> G_prop;

  F_prop fProp;
  C_prop cProp;
  G_prop gProp;
  CFieldDoc* pDoc;
  CIntegralProp* pDlg;
};

static int read_string(std::istream& in, char* &ch, int &nc)
{
  int n = 0;
  if (!in.read(&ch[n], 1))
    return -1;

  while (ch[n])
  {
    if (++n > nc)
    {
      char *tmp = new char[2*nc+1];
      std::copy(ch, ch+nc+1, tmp);
      delete[] ch;
      ch = tmp;
      nc = 2*nc;
    }

    if (!in.read(&ch[n], 1))
      return -1;
  }

  return n;
}

static bool read_surfer_grid(const CFieldDoc* pDoc, const char* name, double* ff)
{
  char ch[4];
  std::ifstream in(name, std::ios::binary);
  if (!in.read(ch, sizeof(ch)))
    return false;

  if (strncmp(ch, "DSBB", 4) == 0)
  {
    short cx, cy;
    double x1, x2, y1, y2, z1, z2;
    if (!in.read((char*) &cx, sizeof(cx)))
      return false;
    if (cx != pDoc->cx)
      return false;
    if (!in.read((char*) &cy, sizeof(cy)))
      return false;
    if (cy != pDoc->cy)
      return false;
    if (!in.read((char*) &x1, sizeof(x1)))
      return false;
    if (x1 != pDoc->x1)
      return false;
    if (!in.read((char*) &x2, sizeof(x2)))
      return false;
    if (x2 != pDoc->x2)
      return false;
    if (!in.read((char*) &y1, sizeof(y1)))
      return false;
    if (y1 != pDoc->y1)
      return false;
    if (!in.read((char*) &y2, sizeof(y2)))
      return false;
    if (y2 != pDoc->y2)
      return false;
    if (!in.read((char*) &z1, sizeof(z1)))
      return false;
    if (!in.read((char*) &z2, sizeof(z2)))
      return false;
    
    for (int nn = cx * cy; nn; --nn)
    {
      float dd;
      if (!in.read((char*) &dd, sizeof(dd)))
        return false;

      *ff++ = dd;
    }
  }
  else if (strncmp(ch, "DSAA", 4) == 0)
  {
    std::ifstream in(name);
    std::string ch;
    short cx, cy;
    double x1, x2, y1, y2, z1, z2;
    if (in >> ch >> cx >> cy >> x1 >> x2 >> y1 >> y2 >> z1 >> z2)
    {
      if (cx == pDoc->cx && cy == pDoc->cy && x1 == pDoc->x1 
        && x2 == pDoc->x2 && y1 == pDoc->y1 && y2 == pDoc->y2)
      {
        int n = std::copy(std::istream_iterator<double>(in), \
          std::istream_iterator<double>(), ff) - ff;
        return (n == cx * cy);
      }
    }
    return false;
  }
  else if (strncmp(ch, "DSRB", 4) == 0)
  {
    long nn;
    if (!in.read((char*) &nn, sizeof(nn)) || nn != 4)
      return false;

    long ver;
    if (!in.read((char*) &ver, sizeof(ver)) || ver != 1)
      return false;

    if (!in.read((char*) ch, sizeof(ch)) || strncmp(ch, "GRID", 4) != 0)
      return false;

    if (!in.read((char*) &nn, sizeof(nn)) || nn != 9*8)
      return false;

    int cy;
    if (!in.read((char*) &cy, sizeof(cy)) || pDoc->cy != cy)
      return false;

    long cx;
    if (!in.read((char*) &cx, sizeof(cx)) || pDoc->cx != cx)
      return false;

    double x1;
    if (!in.read((char*) &x1, sizeof(x1)) || pDoc->x1 != x1)
      return false;

    double y1;
    if (!in.read((char*) &y1, sizeof(y1)) || pDoc->y1 != y1)
      return false;

    const double eps = 1.0e-7;

    double dx;
    if (!in.read((char*) &dx, sizeof(dx)) || fabs(pDoc->x2 - (x1 + dx * (cx-1))) > eps)
      return false;

    double dy;
    if (!in.read((char*) &dy, sizeof(dy)) || fabs(pDoc->y2 - (y1 + dy * (cy-1))) > eps)
      return false;

    double z1;
    if (!in.read((char*) &z1, sizeof(z1)))
      return false;

    double z2;
    if (!in.read((char*) &z2, sizeof(z2)))
      return false;

    double rot;
    if (!in.read((char*) &rot, sizeof(rot)))
      return false;

    double bln;
    if (!in.read((char*) &bln, sizeof(bln)))
      return false;

    if (!in.read(ch, sizeof(ch)) || strncmp(ch, "DATA", 4) != 0)
      return false;

    if (!in.read((char*) &nn, sizeof(nn)) || nn != (long) (cx * cy * sizeof(double)))
      return false;

    if (!in.read((char*) ff, nn))
      return false;
  }
  else
    return false;

  return true;
}

static bool compute_node_range(const CFieldDoc* pDoc, double *top, double *bot)
{
  int i, j, n, nn = pDoc->cx * pDoc->cy;
  for (j = n = 0; j < pDoc->cy; ++j)
  {
    double xx[3], pp[3];
    xx[1] = pDoc->y1 + (pDoc->y2 - pDoc->y1) * j / (pDoc->cy - 1);
    for (i = 0; i < pDoc->cx; ++i, ++n)
    {
      xx[0] = pDoc->x1 + (pDoc->x2 - pDoc->x1) * i / (pDoc->cx - 1);
      if (top)
      {
        xx[2] = top[n];
        int ff = pDoc->Cell(xx, pp);
        if (INSIDE_DOM(ff))
        {
          top[n] = pp[2];
        }
        else if (ff == CFieldDoc::TOP_SIDE)
        {
          top[n] = 0.0;
        }
        else
        {
          return false;
        }
      }
      if (bot)
      {
        xx[2] = bot[n];
        int ff = pDoc->Cell(xx, pp);
        if (INSIDE_DOM(ff))
        {
          bot[n] = pp[2];
        }
        else if (ff == CFieldDoc::BOTTOM_SIDE)
        {
          bot[n] = pDoc->cz-1;
        }
        else
        {
          return false;
        }
      }
    }
  }
  return true;
}

bool have_access(const char* name)
{
  HANDLE hh = CreateFile(name, GENERIC_READ, 0, NULL, \
    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  BOOL ok = CloseHandle(hh);
  return (hh != INVALID_HANDLE_VALUE);
}

static float id(float x)
{
  return x;
}

static void make_default_palette(float datarange[2], CActField::Palette* pal)
{
  delete[] pal->zz;
  delete[] pal->sh;

  const int nz = 5;
  float *zz = new float[nz+2];
  CActField::Color *sh = new CActField::Color[nz+2];

  float z1 = datarange[0];
  float z2 = datarange[1];
  zz[0] = z1;
  zz[6] = z2;
  for (int i = 1; i <= nz; ++i)
  {
    zz[i] = z1 + (z2 - z1) * (i - 0.5f) / nz;
  }

  sh[0].r = 0.0, sh[0].g = 0.0, sh[0].b = 0.5;
  sh[1].r = 0.0, sh[1].g = 0.0, sh[1].b = 1.0;
  sh[2].r = 0.0, sh[2].g = 1.0, sh[2].b = 1.0;
  sh[3].r = 0.0, sh[3].g = 1.0, sh[3].b = 0.0;
  sh[4].r = 1.0, sh[4].g = 1.0, sh[4].b = 0.0;
  sh[5].r = 1.0, sh[5].g = 0.0, sh[5].b = 0.0;
  sh[6].r = 1.0, sh[6].g = 0.5, sh[6].b = 0.5;

  pal->nz = nz;
  pal->sh = sh;
  pal->zz = zz;
  pal->fwd = pal->bck = id;
}

IntegralProp_Impl::IntegralProp_Impl(CIntegralProp* pp): fProp(), cProp(), gProp(), \
  pDoc(GetFieldView()->GetDocument()), pDlg(pp)
{
}

IntegralProp_Impl::~IntegralProp_Impl()
{
  for (F_prop::iterator fIter = fProp.begin(); fIter != fProp.end(); ++fIter)
  {
    Formation* val = fIter->second;
    delete[] val->upperfile;
    delete[] val->lowerfile;
    delete[] val->uppergrid;
    delete[] val->lowergrid;
    delete[] val->notation;
    delete val;
  }

  for (C_prop::iterator cIter = cProp.begin(); cIter != cProp.end(); ++cIter)
  {
    Constraint* val = cIter->second;
    delete[] val->x_coord;
    delete[] val->y_coord;
    delete[] val->filename;
    delete[] val->feature;
    delete[] val->gridmask;
    delete val;
  }

  for (G_prop::iterator gIter = gProp.begin(); gIter != gProp.end(); ++gIter)
  {
    Gridfunc* val = gIter->second;
    delete[] val->labeling;
    delete[] val->expression;
    delete[] val->description;
    delete[] val->datavalue;
    Palette* pal = &val->palette;
    delete[] pal->sh;
    delete[] pal->zz;
    delete val;
  }
}

void IntegralProp_Impl::Print() const
{
  static int n = 0;
  char name[8];
  sprintf(name, "f.%d", n);
  std::ofstream fOut(name);
  for (F_prop::const_iterator fIter = fProp.begin(); fIter != fProp.end(); ++fIter)
  {
    const Formation* val = fIter->second;
    fOut << val->notation << ' ' << val->refcount << "\n\t" 
      << val->upperfile << "\n\t" << val->lowerfile << "\n\n";
  }
  sprintf(name, "c.%d", n);
  std::ofstream cOut(name);
  for (C_prop::const_iterator cIter = cProp.begin(); cIter != cProp.end(); ++cIter)
  {
    const Constraint* val = cIter->second;
    cOut << val->feature << ' ' << val->refcount << "\n\t" << val->filename << "\n\n";
  }
  sprintf(name, "g.%d", n);
  std::ofstream gOut(name);
  for (G_prop::const_iterator gIter = gProp.begin(); gIter != gProp.end(); ++gIter)
  {
    const Gridfunc* val = gIter->second;
    gOut << val->labeling << "\n\t" << val->expression << "\n\t" << val->description << "\n\t";

    const Constraint* cVal = val->constraint;
    gOut << (cVal? cVal->feature: ".") << "\n\t";
    
    const Formation* fVal = val->formation;
    gOut << (fVal? fVal->notation: ".") << "\n\n";
  }
  ++n;
}

bool IntegralProp_Impl::Eval(Gridfunc* ptr)
{
  int nn = pDoc->cx * pDoc->cy;
  std::valarray<double> z1(0.0, nn), z2(double(pDoc->cz - 1), nn);
  if (ptr->formation)
  {
    Formation* src = ptr->formation;
    std::copy(src->uppergrid, src->uppergrid+nn, &z1[0]);
    std::copy(src->lowergrid, src->lowergrid+nn, &z2[0]);
  }

  std::valarray<double> ww(1.0, nn);
  if (ptr->constraint)
  {
    Constraint *src = ptr->constraint;
    std::copy(src->gridmask, src->gridmask+nn, &ww[0]);
  }

  Expr ex(pDoc);
  if (ex.Parse(ptr->expression) == 1)
  {
    return false;
  }

  int k = (int) floor(z1.min()), kk = (int) ceil(z2.max());
  Expr::farray gg = ex.Eval(k);
  if ((int) gg.size() != nn)
  {
    AfxMessageBox("Trivial expression defined\n");
    pDlg->m_wndEdit2.SetFocus();
    return false;
  }

  TRACE0("Computing a target grid...");
  float* ff = ptr->datavalue;
  double dz = 0.4;
  for (int n = 0; n < nn; ++n)
  {
    double ww_n = ww[n];
    double gg_n = gg[n];
    if (z1[n] <= k && k <= z2[n])
    {
      ff[n] = float(dz * gg_n * ww_n);
    }
    else
    {
      ff[n] = 0.0;
    }
  }
  while (++k <= kk)
  {
    gg = ex.Eval(k);
    for (n = 0; n < nn; ++n)
    {
      double ww_n = ww[n];
      double gg_n = gg[n];
      if (z1[n] <= k && k <= z2[n])
      {
        ff[n] += float(dz * gg_n * ww_n);
      }
    }
  }
  double sx = (pDoc->x2 - pDoc->x1) / (pDoc->cx - 1);
  double sy = (pDoc->y2 - pDoc->y1) / (pDoc->cy - 1);
  ptr->datarange[0] = *std::min_element(ff, ff+nn);
  ptr->datarange[1] = *std::max_element(ff, ff+nn);
  ptr->integral = float(std::accumulate(ff, ff+nn, 0.0f) * sx * sy);
  make_default_palette(ptr->datarange, &ptr->palette);
  TRACE0("done\n");
  return true;
}

bool IntegralProp_Impl::Make(Constraint** ptr)
{
  CString file, name;
  pDlg->m_wndEdit8.GetWindowText(file);
  pDlg->m_wndEdit7.GetWindowText(name);
  std::ifstream in(file);

  int nn;
  std::string s;
  if (std::getline(in, s))
  {
    std::istringstream ss(s);
    if (!(ss >> nn))
    {
      return false;
    }
  }
  else
  {
    return false;
  }

  std::valarray<double> xx(nn), yy(nn);
  if (std::getline(in, s))
  {
    double x, y;
    std::istringstream ss(s);
    if (!(ss >> x >> y))
    {
      return false;
    }
    xx[0] = x;
    yy[0] = y;
  }
  else
  {
    return false;
  }

  int n = 0;
  while (std::getline(in, s))
  {
    double x, y;
    std::istringstream ss(s);
    if (!(ss >> x >> y))
    {
      break;
    }
    if (xx[n] != x && yy[n] != y)
    {
      if (++n >= nn-1)
      {
        break;
      }
      xx[n] = x;
      yy[n] = y;
    }
  }
  
  if (xx[0] != xx[n] || yy[0] != yy[n])
    ++n;

#if 0
  std::ofstream out("dump.bln");
  out << n << '\n';
  for (int i = 0; i < n; ++i)
  {
    out << xx[i] << ' ' << yy[i] << '\n';
  }
  out.close();
#endif

  int cx = pDoc->cx, cy = pDoc->cy;
  double x1 = pDoc->x1, x2 = pDoc->x2, y1 = pDoc->y1, y2 = pDoc->y2;
  int h = pg_create_uniform(cx, cy, x1, x2, y1, y2);
  if (!h) return false;

  Constraint* val = new Constraint;
  val->n_points = n;
  val->x_coord = new double[n];
  std::reverse_copy(&xx[0], &xx[0]+n, val->x_coord);
  val->y_coord = new double[n];
  std::reverse_copy(&yy[0], &yy[0]+n, val->y_coord);
  
  n = strlen(file)+1;
  val->filename = new char[n];
  strcpy(val->filename, file);

  n = strlen(name)+1;
  val->feature = new char[n];
  strcpy(val->feature, name);
  
  n = cx * cy;
  double *ww = new double[n];
  pg_populate(h, val->n_points, val->x_coord, val->y_coord);
  for (int j = n = 0; j < cy; ++j)
  {
    for (int i = 0; i < cx; ++i, ++n)
    {
      ww[n] = pg_info(h, i, j);
    }
  }
  pg_destroy(h);

  val->gridmask = ww;
  val->refcount = 0;
  *ptr = val;
  return true;
}

bool IntegralProp_Impl::Update(Constraint** ptr)
{
  CString argv6, argv7;
  pDlg->m_wndEdit7.GetWindowText(argv6);
  pDlg->m_wndEdit8.GetWindowText(argv7);

  if (argv6.IsEmpty())
  {
    *ptr = 0; 
    return false;
  }

  Key_type key = argv6;
  C_prop::iterator cIter = cProp.find(key);
  if (cIter != cProp.end())
  {
    Constraint* val = cIter->second;
    if (argv7.IsEmpty())
    {
      pDlg->m_wndEdit8.SetWindowText(val->filename);
      *ptr = val;
      return false;
    }
    else if (stricmp(argv7, val->filename) == 0)
    {
      *ptr = val;
      return false;
    }
    if (!have_access(argv7))
    {
      AfxMessageBox("You should give a valid file name\n");
      pDlg->m_wndEdit8.SetFocus();
      *ptr = 0; 
      return true;
    }
    Constraint* tmp = 0;
    if (!Make(&tmp))
    {
      AfxMessageBox("Invalid file format\n");
      pDlg->m_wndEdit8.SetFocus();
      *ptr = 0; 
      return true;
    }
    delete[] val->x_coord;
    delete[] val->y_coord;
    delete[] val->filename;
    delete[] val->feature;
    delete[] val->gridmask;
    val->n_points = tmp->n_points;
    val->x_coord = tmp->x_coord;
    val->y_coord = tmp->y_coord;
    val->filename = tmp->filename;
    val->feature = tmp->feature;
    val->gridmask = tmp->gridmask;
    delete tmp;
    *ptr = val;
    return true;
  }
  if (argv7.IsEmpty() || !have_access(argv7))
  {
    AfxMessageBox("You should give a valid file name\n");
    pDlg->m_wndEdit8.SetFocus();
    *ptr = 0; 
    return true;
  }
  Constraint* val = 0;
  if (!Make(&val))
  {
    AfxMessageBox("Invalid file format\n");
    pDlg->m_wndEdit8.SetFocus();
    *ptr = 0; 
    return true;
  }
  cProp.insert(std::make_pair(key, val));
  *ptr = val; 
  return true;
}

bool IntegralProp_Impl::Update(Formation** ptr)
{
  CString argv3, argv4, argv5;
  pDlg->m_wndEdit4.GetWindowText(argv3);
  pDlg->m_wndEdit5.GetWindowText(argv4);
  pDlg->m_wndEdit6.GetWindowText(argv5);

  if (argv3.IsEmpty())
  {
    *ptr = 0;
    return false;
  }

  Key_type key = (const char*) argv3;
  F_prop::iterator fIter = fProp.find(key);
  int nn = pDoc->cx * pDoc->cy;
  if (fIter != fProp.end())
  {
    Formation *val = fIter->second;

    if (argv4.IsEmpty())
    {
      pDlg->m_wndEdit5.SetWindowText(val->upperfile);
    }
    if (argv5.IsEmpty())
    {
      pDlg->m_wndEdit6.SetWindowText(val->lowerfile);
    }
    double *uppergrid = 0;
    if (stricmp(argv4, val->upperfile) != 0)
    {
      uppergrid = new double[nn];
      if (!read_surfer_grid(pDoc, argv4, uppergrid))
      {
        AfxMessageBox("Invalid file format\n");
        pDlg->m_wndEdit5.SetFocus();
        *ptr = 0;
        return true;
      }
    }
    double *lowergrid = 0;
    if (stricmp(argv5, val->lowerfile) != 0)
    {
      lowergrid = new double[nn];
      if (!read_surfer_grid(pDoc, argv5, lowergrid))
      {
        AfxMessageBox("Invalid file format\n");
        pDlg->m_wndEdit6.SetFocus();
        *ptr = 0;
        return true;
      }
    }
    if (!uppergrid && !lowergrid)
    {
      *ptr = val;
      return false;
    }
    if (!compute_node_range(pDoc, uppergrid, lowergrid))
    {
      AfxMessageBox("Grid data is out of range\n");
      pDlg->m_wndEdit5.SetFocus();
      delete[] uppergrid;
      delete[] lowergrid;
      *ptr = 0;
      return true;
    }
    if (uppergrid)
    {
      delete[] val->upperfile;
      delete[] val->uppergrid;
      val->upperfile = new char[strlen(argv4)+1];
      strcpy(val->upperfile, argv4);
      val->uppergrid = uppergrid;
    }
    if (lowergrid)
    {
      delete[] val->lowerfile;
      delete[] val->lowergrid;
      val->lowerfile = new char[strlen(argv5)+1];
      strcpy(val->lowerfile, argv5);
      val->lowergrid = lowergrid;
    }
    *ptr = val;
    return true;
  }
  
  double* uppergrid = new double[nn];
  if (argv4.IsEmpty() || !have_access(argv4) \
    || !read_surfer_grid(pDoc, argv4, uppergrid))
  {
    AfxMessageBox("You should define a valid filename\n");
    pDlg->m_wndEdit5.SetFocus();
    delete[] uppergrid;
    *ptr = 0;
    return true;
  }

  double* lowergrid = new double[nn];
  if (argv5.IsEmpty() || !have_access(argv5) \
    || !read_surfer_grid(pDoc, argv5, lowergrid))
  {
    AfxMessageBox("You should define a valid file name\n");
    pDlg->m_wndEdit6.SetFocus();
    delete[] uppergrid;
    delete[] lowergrid;
    *ptr = 0;
    return true;
  }

  if (!compute_node_range(pDoc, uppergrid, lowergrid))
  {
    AfxMessageBox("Grid data is out of range\n");
    pDlg->m_wndEdit5.SetFocus();
    delete[] uppergrid;
    delete[] lowergrid;
    *ptr = 0;
    return true;
  }

  Formation* val = new Formation;
  val->upperfile = new char[strlen(argv4)+1];
  strcpy(val->upperfile, argv4);
  val->lowerfile = new char[strlen(argv5)+1];
  strcpy(val->lowerfile, argv5);
  val->notation = new char[strlen(argv3)+1];
  strcpy(val->notation, argv3);
  val->uppergrid = uppergrid;
  val->lowergrid = lowergrid;
  val->refcount = 0;
  fProp.insert(std::make_pair(key, val));
  *ptr = val;
  return true;
}

bool IntegralProp_Impl::Update()
{
  CString argv0, argv1, argv2;
  pDlg->m_wndEdit1.GetWindowText(argv0);
  pDlg->m_wndEdit2.GetWindowText(argv1);
  pDlg->m_wndEdit3.GetWindowText(argv2);

  if (argv0.IsEmpty())
  {
    AfxMessageBox("Please define labeling and units for the dataset\n");
    pDlg->m_wndEdit1.SetFocus();
    return false;
  }

  Constraint *cPtr = 0;
  bool cChg = Update(&cPtr);
  if (cChg && !cPtr)
    return false;

  Formation *fPtr = 0;
  bool fChg = Update(&fPtr);
  if (fChg && !fPtr)
    return false;

  Key_type key = argv0;
  G_prop::iterator gIter = gProp.find(key);
  if (gIter != gProp.end())
  {
    Gridfunc* val = gIter->second;
    bool gChg = false;

    if (argv1.IsEmpty()) 
    {
      pDlg->m_wndEdit2.SetWindowText(val->expression);
    }

    if (argv2.IsEmpty())
    {
      pDlg->m_wndEdit3.SetWindowText(val->description);
    }

    if (stricmp(argv1, val->expression) != 0)
    {
      Expr ex(pDoc);
      if (ex.Parse(argv1) == 1)
        return false;

      Expr::farray tmp = ex.Eval(0);
      if ((int) tmp.size() != pDoc->cx * pDoc->cy)
      {
        AfxMessageBox("Trivial expression defined\n");
        pDlg->m_wndEdit2.SetFocus();
        return false;
      }

      gChg = true;
      delete[] val->expression;
      val->expression = new char[strlen(argv1)+1];
      strcpy(val->expression, argv1);
    }

    if (stricmp(argv2, val->description) != 0)
    {
      delete[] val->description;
      val->description = new char[strlen(argv2)+1];
      strcpy(val->description, argv2);
    }

    if (cPtr != val->constraint)
    {
      if (val->constraint)
      {
        Constraint* tmp = val->constraint;
        if (--tmp->refcount <= 0)
        {
          Key_type key = tmp->feature;
          cProp.erase(key);
          delete[] tmp->x_coord;
          delete[] tmp->y_coord;
          delete[] tmp->filename;
          delete[] tmp->feature;
          delete[] tmp->gridmask;
          delete tmp;
        }
      }
      gChg = true;
      val->constraint = cPtr;
      if (cPtr) ++cPtr->refcount;
    }

    if (fPtr != val->formation)
    {
      if (val->formation)
      {
        Formation* tmp = val->formation;
        if (--tmp->refcount <= 0)
        {
          Key_type key = tmp->notation;
          fProp.erase(key);
          delete[] tmp->lowerfile;
          delete[] tmp->upperfile;
          delete[] tmp->lowergrid;
          delete[] tmp->uppergrid;
          delete[] tmp->notation;
          delete tmp;
        }
      }
      gChg = true;
      val->formation = fPtr;
      if (fPtr) ++fPtr->refcount;
    }

    if (cChg || fChg)
    {
      for (gIter = gProp.begin(); gIter != gProp.end(); ++gIter)
      {
        Gridfunc* ptr = gIter->second;
        if (cChg && cPtr == ptr->constraint || fChg && fPtr == ptr->formation)
        {
          Eval(ptr);
          
          LVFINDINFO info;
          info.flags = LVFI_STRING;
          info.psz = ptr->labeling;
          int n = pDlg->m_wndList1.FindItem(&info);
          ASSERT(n != -1);

          CString text;
          text.Format("%f", ptr->integral);
          pDlg->m_wndList1.SetItemText(n, 1, text);
          if (ptr == val) pDlg->SelectItem(n);
        }
      }
    }
    else if (gChg)
    {
      Eval(val);

      LVFINDINFO info;
      info.flags = LVFI_STRING;
      info.psz = val->labeling;
      int n = pDlg->m_wndList1.FindItem(&info);
      ASSERT(n != -1);

      CString text;
      text.Format("%f", val->integral);
      pDlg->m_wndList1.SetItemText(n, 1, text);
      pDlg->SelectItem(n);
    }
    return true;
  }

  if (argv1.IsEmpty())
  {
    AfxMessageBox("Please define an element of integration\n");
    pDlg->m_wndEdit2.SetFocus();
    return false;
  }
  else
  {
    Expr ex(pDoc);
    if (ex.Parse(argv1) == 1)
      return false;

    Expr::farray tmp = ex.Eval(0);
    if ((int) tmp.size() != pDoc->cx * pDoc->cy)
    {
      AfxMessageBox("Trivial expression defined\n");
      pDlg->m_wndEdit2.SetFocus();
      return false;
    }
  }

  if (argv2.IsEmpty())
  {
    AfxMessageBox("Please define non-empty description of dataset\n");
    pDlg->m_wndEdit3.SetFocus();
    return false;
  }

  Gridfunc* val = new Gridfunc;
  val->constraint = cPtr;
  if (cPtr) ++cPtr->refcount;
  val->formation = fPtr;
  if (fPtr) ++fPtr->refcount;
  val->labeling = new char[strlen(argv0)+1];
  strcpy(val->labeling, argv0);
  val->expression = new char[strlen(argv1)+1];
  strcpy(val->expression, argv1);
  val->description = new char[strlen(argv2)+1];
  strcpy(val->description, argv2);
  int nn = pDoc->cx * pDoc->cy;
  val->datavalue = new float[nn];
  Palette* pal = &val->palette;
  pal->zz = 0;
  pal->sh = 0;
  gProp.insert(std::make_pair(key, val));
  int n = pDlg->m_wndList1.GetItemCount();
  n = pDlg->m_wndList1.InsertItem(n, val->labeling);

  if (cChg || fChg)
  {
    for (gIter = gProp.begin(); gIter != gProp.end(); ++gIter)
    {
      Gridfunc* ptr = gIter->second;
      if (cChg && cPtr == ptr->constraint || fChg && fPtr == ptr->formation)
      {
        Eval(ptr);
        
        LVFINDINFO info;
        info.flags = LVFI_STRING;
        info.psz = ptr->labeling;
        int n = pDlg->m_wndList1.FindItem(&info);
        ASSERT(n != -1);

        CString text;
        text.Format("%f", ptr->integral);
        pDlg->m_wndList1.SetItemText(n, 1, text);
      }
    }
  }
  else
  {
    Eval(val);

    LVFINDINFO info;
    info.flags = LVFI_STRING;
    info.psz = val->labeling;
    int n = pDlg->m_wndList1.FindItem(&info);
    ASSERT(n != -1);

    CString text;
    text.Format("%f", val->integral);
    pDlg->m_wndList1.SetItemText(n, 1, text);
  }

  pDlg->SelectItem(n);
  return true;
}

bool IntegralProp_Impl::Remove()
{
  CString argv0;
  pDlg->m_wndEdit1.GetWindowText(argv0);

  if (argv0.IsEmpty())
    return false;

  Key_type key = argv0;
  G_prop::iterator gIter = gProp.find(key);
  if (gIter == gProp.end())
    return false;

  Gridfunc* val = gIter->second;
  gProp.erase(gIter);
  {          
    LVFINDINFO info;
    info.flags = LVFI_STRING;
    info.psz = val->labeling;
    int n = pDlg->m_wndList1.FindItem(&info);
    ASSERT(n != -1);
    pDlg->m_wndList1.DeleteItem(n);
    int nn = pDlg->m_wndList1.GetItemCount();
    if (nn > 0) pDlg->SelectItem(std::min(n, nn-1));
  }

  if (val->constraint)
  {
    Constraint* tmp = val->constraint;
    if (--tmp->refcount <= 0)
    {
      key = tmp->feature;
      cProp.erase(key);
      delete[] tmp->x_coord;
      delete[] tmp->y_coord;
      delete[] tmp->filename;
      delete[] tmp->feature;
      delete[] tmp->gridmask;
      delete tmp;
    }
  }
  if (val->formation)
  {
    Formation* tmp = val->formation;
    if (--tmp->refcount <= 0)
    {
      key = tmp->notation;
      fProp.erase(key);
      delete[] tmp->upperfile;
      delete[] tmp->lowerfile;
      delete[] tmp->uppergrid;
      delete[] tmp->lowergrid;
      delete[] tmp->notation;
      delete tmp;
    }
  }
  delete[] val->labeling;
  delete[] val->expression;
  delete[] val->description;
  delete[] val->datavalue;
  delete[] val->palette.zz;
  delete[] val->palette.sh;
  delete val;

  return true;
}

void IntegralProp_Impl::Fill(const char* str) const
{
  Key_type key = str;
  G_prop::const_iterator gIter = gProp.find(key);
  if (gIter == gProp.end())
    return;

  Gridfunc* val = gIter->second;
  pDlg->m_wndEdit1.SetWindowText(val->labeling);
  pDlg->m_wndEdit2.SetWindowText(val->expression);
  pDlg->m_wndEdit3.SetWindowText(val->description);
  if (val->formation)
  {
    Formation* ptr = val->formation;
    pDlg->m_wndEdit4.SetWindowText(ptr->notation);
    pDlg->m_wndEdit5.SetWindowText(ptr->upperfile);
    pDlg->m_wndEdit6.SetWindowText(ptr->lowerfile);
  }
  else
  {
    pDlg->m_wndEdit4.SetWindowText("");
    pDlg->m_wndEdit5.SetWindowText("");
    pDlg->m_wndEdit6.SetWindowText("");
  }
  if (val->constraint)
  {
    Constraint* ptr = val->constraint;
    pDlg->m_wndEdit7.SetWindowText(ptr->feature);
    pDlg->m_wndEdit8.SetWindowText(ptr->filename);
  }
  else
  {
    pDlg->m_wndEdit7.SetWindowText("");
    pDlg->m_wndEdit8.SetWindowText("");
  }
}

bool IntegralProp_Impl::Write(std::ostream& out) const
{
  char zz = '\0';
  int nn = pDoc->cx * pDoc->cy;

  for (F_prop::const_iterator fIter = fProp.begin(); fIter != fProp.end(); ++fIter)
  {
    const Formation* val = fIter->second;
    out.write(val->notation, strlen(val->notation)+1);
    out.write(val->upperfile, strlen(val->upperfile)+1);
    out.write((const char*) val->uppergrid, sizeof(double) * nn);
    out.write(val->lowerfile, strlen(val->lowerfile)+1);
    out.write((const char*) val->lowergrid, sizeof(double) * nn);
    out.write((const char*) &val->refcount, sizeof(val->refcount));
  }
  out.write(&zz, 1);

  for (C_prop::const_iterator cIter = cProp.begin(); cIter != cProp.end(); ++cIter)
  {
    const Constraint* val = cIter->second;
    out.write(val->feature, strlen(val->feature)+1);
    out.write(val->filename, strlen(val->filename)+1);
    out.write((const char*) val->gridmask, sizeof(double) * nn);
    out.write((const char*) &val->n_points, sizeof(val->n_points));
    out.write((const char*) val->x_coord, sizeof(double) * val->n_points);
    out.write((const char*) val->y_coord, sizeof(double) * val->n_points);
    out.write((const char*) &val->refcount, sizeof(val->refcount));
  }
  out.write(&zz, 1);

  for (G_prop::const_iterator gIter = gProp.begin(); gIter != gProp.end(); ++gIter)
  {
    const Gridfunc* val = gIter->second;
    out.write(val->labeling, strlen(val->labeling)+1);
    out.write(val->expression, strlen(val->expression)+1);
    out.write(val->description, strlen(val->description)+1);
    if (val->formation)
    {
      const Formation* ptr = val->formation;
      out.write(ptr->notation, strlen(ptr->notation)+1);
    }
    else
    {
      out.write(&zz, 1);
    }
    if (val->constraint)
    {
      const Constraint* ptr = val->constraint;
      out.write(ptr->feature, strlen(ptr->feature)+1);
    }
    else
    {
      out.write(&zz, 1);
    }
    out.write((const char*) &val->integral, sizeof(val->integral));
    out.write((const char*) val->datarange, sizeof(val->datarange));
    out.write((const char*) val->datavalue, sizeof(float) * nn);

    const Palette* pal = &val->palette;
    out.write((const char*) &pal->nz, sizeof(pal->nz));
    out.write((const char*) pal->zz, (pal->nz+2) * sizeof(pal->zz[0]));
    out.write((const char*) pal->sh, (pal->nz+2) * sizeof(pal->sh[0]));
  }
  out.write(&zz, 1);
  return true;
}

bool IntegralProp_Impl::Read(std::istream& in)
{
  char* ch = new char[41];
  int nc = 40, nn = pDoc->cx * pDoc->cy;
  int n = read_string(in, ch, nc);
  if (n < 0) 
  {
    delete[] ch;
    return false;
  }

  while (n > 0) // formation
  {
    char* notation = new char[n+1];
    strcpy(notation, ch);

    n = read_string(in, ch, nc);
    if (n <= 0)
    {
      delete[] ch;
      delete[] notation;
      return false;
    }
    char *upperfile = new char[n+1];
    strcpy(upperfile, ch);

    double *uppergrid = new double[nn];
    if (!in.read((char*) uppergrid, sizeof(double) * nn))
    {
      delete[] ch;
      delete[] notation;
      delete[] upperfile;
      delete[] uppergrid;
      return false;
    }

    n = read_string(in, ch, nc);
    if (n <= 0)
    {
      delete[] ch;
      delete[] notation;
      delete[] upperfile;
      delete[] uppergrid;
      return false;
    }
    char* lowerfile = new char[n+1];
    strcpy(lowerfile, ch);

    double* lowergrid = new double[nn];
    if (!in.read((char*) lowergrid, sizeof(double) * nn))
    {
      delete[] ch;
      delete[] notation;
      delete[] upperfile;
      delete[] uppergrid;
      delete[] lowerfile;
      delete[] lowergrid;
      return false;
    }

    int refcount;
    if (!in.read((char*) &refcount, sizeof(refcount)))
    {
      delete[] ch;
      delete[] notation;
      delete[] upperfile;
      delete[] uppergrid;
      delete[] lowerfile;
      delete[] lowergrid;
      return false;
    }

    Key_type key = notation;
    F_prop::iterator fIter = fProp.find(key);
    if (fIter != fProp.end())
    {
      delete[] ch;
      delete[] notation;
      delete[] upperfile;
      delete[] uppergrid;
      delete[] lowerfile;
      delete[] lowergrid;
      return false;
    }

    Formation* val = new Formation;
    val->notation = notation;
    val->upperfile = upperfile;
    val->lowerfile = lowerfile;
    val->uppergrid = uppergrid;
    val->lowergrid = lowergrid;
    val->refcount = refcount;
    fProp.insert(std::make_pair(key, val));

    n = read_string(in, ch, nc);
    if (n < 0)
    {
      delete[] ch;
      return false;
    }
  }

  n = read_string(in, ch, nc);
  if (n < 0)
  {
    delete[] ch;
    return false;
  }

  while (n > 0) // constraint
  {
    char *feature = new char[n+1];
    strcpy(feature, ch);

    n = read_string(in, ch, nc);
    if (n <= 0)
    {
      delete[] ch;
      delete[] feature;
      return false;
    }
    char* filename = new char[n+1];
    strcpy(filename, ch);

    double* gridmask = new double[nn];
    if (!in.read((char*) gridmask, sizeof(double) * nn))
    {
      delete[] ch;
      delete[] feature;
      delete[] gridmask;
      return false;
    }

    int n_points;
    if (!in.read((char*) &n_points, sizeof(n_points)))
    {
      delete[] ch;
      delete[] feature;
      delete[] gridmask;
      return false;
    }

    double *x_coord = new double[n_points];
    if (!in.read((char*) x_coord, sizeof(double) * n_points))
    {
      delete[] ch;
      delete[] feature;
      delete[] gridmask;
      delete[] x_coord;
      return false;
    }

    double *y_coord = new double[n_points];
    if (!in.read((char*) y_coord, sizeof(double) * n_points))
    {
      delete[] ch;
      delete[] feature;
      delete[] gridmask;
      delete[] x_coord;
      delete[] y_coord;
      return false;
    }

    int refcount;
    if (!in.read((char*) &refcount, sizeof(refcount)))
    {
      delete[] ch;
      delete[] feature;
      delete[] gridmask;
      delete[] x_coord;
      delete[] y_coord;
      return false;
    }

    Key_type key = feature;
    C_prop::iterator cIter = cProp.find(key);
    if (cIter != cProp.end())
    {
      delete[] ch;
      delete[] feature;
      delete[] gridmask;
      delete[] x_coord;
      delete[] y_coord;
      return false;
    }

    Constraint *val = new Constraint;
    val->feature = feature;
    val->filename = filename;
    val->gridmask = gridmask;
    val->x_coord = x_coord;
    val->y_coord = y_coord;
    val->n_points = n_points;
    val->refcount = refcount;

    cProp.insert(std::make_pair(key, val));

    n = read_string(in, ch, nc);
    if (n < 0)
    {
      delete[] ch;
      return false;
    }
  }

  n = read_string(in, ch, nc);
  if (n < 0)
  {
    delete[] ch;
    return false;
  }

  while (n > 0) // gridfunc
  {
    char* labeling = new char[n+1];
    strcpy(labeling, ch);

    n = read_string(in, ch, nc);
    if (n <= 0)
    {
      delete[] ch;
      delete[] labeling;
      return false;
    }
    char* expression = new char[n+1];
    strcpy(expression, ch);

    n = read_string(in, ch, nc);
    if (n <= 0)
    {
      delete[] ch;
      delete[] labeling;
      delete[] expression;
      return false;
    }
    char* description = new char[n+1];
    strcpy(description, ch);

    Formation* formation = 0;
    n = read_string(in, ch, nc);
    if (n < 0)
    {
      delete[] ch;
      delete[] labeling;
      delete[] expression;
      delete[] description;
      return false;
    }
    else if (n > 0)
    {
      Key_type key = ch;
      F_prop::iterator fIter = fProp.find(key);
      if (fIter == fProp.end())
      {
        delete[] ch;
        delete[] labeling;
        delete[] expression;
        delete[] description;
        return false;
      }
      formation = fIter->second;
    }

    Constraint* constraint = 0;
    n = read_string(in, ch, nc);
    if (n < 0)
    {
      delete[] ch;
      delete[] labeling;
      delete[] expression;
      delete[] description;
      return false;
    }
    else if (n > 0)
    {
      Key_type key = ch;
      C_prop::iterator cIter = cProp.find(key);
      if (cIter == cProp.end())
      {
        delete[] ch;
        delete[] labeling;
        delete[] expression;
        delete[] description;
        return false;
      }
      constraint = cIter->second;
    }

    float integral;
    if (!in.read((char*) &integral, sizeof(integral)))
    {
      delete[] ch;
      delete[] labeling;
      delete[] expression;
      delete[] description;
      return false;
    }

    float datarange[2];
    if (!in.read((char*) datarange, sizeof(datarange)))
    {
      delete[] ch;
      delete[] labeling;
      delete[] expression;
      delete[] description;
      return false;
    }

    float* datavalue = new float[nn];
    if (!in.read((char*) datavalue, sizeof(float) * nn))
    {
      delete[] ch;
      delete[] labeling;
      delete[] expression;
      delete[] description;
      delete[] datavalue;
      return false;
    }

    Palette pal;
    if (!in.read((char*) &pal.nz, sizeof(pal.nz)))
    {
      delete[] ch;
      delete[] labeling;
      delete[] expression;
      delete[] description;
      delete[] datavalue;
      return false;
    }
    pal.zz = new float[pal.nz+2];
    if (!in.read((char*) pal.zz, (pal.nz+2) * sizeof(pal.zz[0])))
    {
      delete[] ch;
      delete[] labeling;
      delete[] expression;
      delete[] description;
      delete[] datavalue;
      delete[] pal.zz;
      return false;
    }
    pal.sh = new CActField::Color[pal.nz+2];
    if (!in.read((char*) pal.sh, (pal.nz+2) * sizeof(pal.sh[0])))
    {
      delete[] ch;
      delete[] labeling;
      delete[] expression;
      delete[] description;
      delete[] datavalue;
      delete[] pal.zz;
      delete[] pal.sh;
      return false;
    }

    Key_type key = labeling;
    G_prop::iterator gIter = gProp.find(key);
    if (gIter != gProp.end())
    {
      delete[] ch;
      delete[] labeling;
      delete[] expression;
      delete[] description;
      delete[] datavalue;
      return false;
    }

    Gridfunc* val = new Gridfunc;
    val->labeling = labeling;
    val->expression = expression;
    val->description = description;
    val->constraint = constraint;
    val->formation = formation;
    val->integral = integral;
    val->datarange[0] = datarange[0];
    val->datarange[1] = datarange[1];
    val->datavalue = datavalue;
    val->palette.nz = pal.nz;
    val->palette.zz = pal.zz;
    val->palette.sh = pal.sh;
    val->palette.fwd = id;
    val->palette.bck = id;

    gProp.insert(std::make_pair(key, val));

    n = read_string(in, ch, nc);
    if (n < 0)
    {
      delete[] ch;
      return false;
    }
  }

  delete[] ch;
  return true;
}

void IntegralProp_Impl::Populate() const
{
  pDlg->m_wndList1.DeleteAllItems();
  for (G_prop::const_iterator gIter = gProp.begin(); gIter != gProp.end(); ++gIter)
  {
    Gridfunc* val = gIter->second;
    int n = pDlg->m_wndList1.InsertItem(0, val->labeling);

    CString text;
    text.Format("%f", val->integral);
    pDlg->m_wndList1.SetItemText(n, 1, text);
  }

  if (pDlg->m_wndList1.GetItemCount() > 0)
  {
    pDlg->SelectItem(0);
  }
}

float* IntegralProp_Impl::GetData(const char* str)
{
  Key_type key = str;
  G_prop::iterator gIter = gProp.find(key);
  if (gIter == gProp.end())
    return 0;

  return gIter->second->datavalue;
}

CActField::Palette* IntegralProp_Impl::GetPalette(const char* str)
{
  Key_type key = str;
  G_prop::iterator gIter = gProp.find(key);
  if (gIter == gProp.end())
    return 0;

  return &gIter->second->palette;
}

/////////////////////////////////////////////////////////////////////////////
// CIntegralProp dialog

CIntegralProp::CIntegralProp(): CDialog(CIntegralProp::IDD, NULL), m_pImpl(0)
{
	//{{AFX_DATA_INIT(CIntegralProp)
	//}}AFX_DATA_INIT
  m_nItem = -1;
  m_bActive = FALSE;
}

CIntegralProp::~CIntegralProp()
{
  delete m_pImpl;
}


void CIntegralProp::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CIntegralProp)
	DDX_Control(pDX, ID_DIGITIZE, m_wndContour);
	DDX_Control(pDX, IDC_LIST1, m_wndList1);
	// DDX_Control(pDX, IDC_EDIT8, m_wndEdit8);
	DDX_Control(pDX, IDC_EDIT7, m_wndEdit7);
	// DDX_Control(pDX, IDC_EDIT6, m_wndEdit6);
	// DDX_Control(pDX, IDC_EDIT5, m_wndEdit5);
	DDX_Control(pDX, IDC_EDIT4, m_wndEdit4);
	DDX_Control(pDX, IDC_EDIT3, m_wndEdit3);
	DDX_Control(pDX, IDC_EDIT2, m_wndEdit2);
	DDX_Control(pDX, IDC_EDIT1, m_wndEdit1);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CIntegralProp, CDialog)
	//{{AFX_MSG_MAP(CIntegralProp)
	ON_BN_CLICKED(ID_UPDATE, OnUpdate)
	ON_BN_CLICKED(ID_REMOVE, OnRemove)
	ON_NOTIFY(NM_CLICK, IDC_LIST1, OnClickList1)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
	ON_WM_SHOWWINDOW()
	ON_WM_CLOSE()
	ON_BN_CLICKED(ID_DIGITIZE, OnDigitize)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIntegralProp message handlers

int CIntegralProp::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

  if (!m_wndDigitize.Create(CDigitizeDlg::IDD, this))
    return -1;

	return 0;
}

BOOL CIntegralProp::OnInitDialog() 
{
	CDialog::OnInitDialog();

  m_wndList1.SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);
  m_wndList1.InsertColumn(0, "Data");
  m_wndList1.InsertColumn(1, "Value");

  m_imgState.Create(IDB_RADIO_BUTTON, 16, 1, RGB(255, 0, 0));
  m_wndList1.SetImageList(&m_imgState, LVSIL_STATE);

  CRect rc;
  int n = 100;
  m_wndList1.GetClientRect(&rc);
  m_wndList1.SetColumnWidth(0, n);
  m_wndList1.SetColumnWidth(1, rc.Width() - n);

	m_wndEdit8.Initialize(IDC_EDIT8, this);
	m_wndEdit5.Initialize(IDC_EDIT5, this);
	m_wndEdit6.Initialize(IDC_EDIT6, this);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CIntegralProp::OnUpdate() 
{
  CWaitCursor wait;
  if (!UpdateData())
    return;

  if (!m_pImpl)
    m_pImpl = new IntegralProp_Impl(this);

  m_pImpl->Update();
}

void CIntegralProp::OnRemove() 
{
  if (!m_pImpl)
    m_pImpl = new IntegralProp_Impl(this);

  m_pImpl->Remove();
}

void CIntegralProp::SelectItem(int nItem)
{
  int n = m_wndList1.GetItemCount();
  for (int i = 0; i < n; ++i)
  {
    int nState = (i != nItem? 1: 2);
    m_wndList1.SetItemState(i, INDEXTOSTATEIMAGEMASK(nState), LVIS_STATEIMAGEMASK);
  }

  if (m_nItem != nItem)
  {
    m_nItem = nItem;
    if (IsWindowVisible() && m_wndList1.GetItemCount() > 0)
    {
      GetFieldView()->GetDocument()->UpdateAllViews(0, IDD);
    }
    else
    {
      GetFieldView()->GetDocument()->UpdateAllViews(0, 0);
    }
  }
}

void CIntegralProp::OnClickList1(NMHDR* pNMHDR, LRESULT* pResult) 
{
  LPNMLISTVIEW p = (LPNMLISTVIEW) pNMHDR;
  if (p->iItem != -1)
  {
    UINT flags;
    int n = m_wndList1.HitTest(p->ptAction, &flags);
    if (flags & LVHT_ONITEMSTATEICON)
    {
      SelectItem(p->iItem);
    }

    m_pImpl->Fill(m_wndList1.GetItemText(p->iItem, 0));
  }

	*pResult = 0;
}

void CIntegralProp::OnFileOpen() 
{
  CFileDialog dlg(TRUE);
  if (dlg.DoModal() == IDOK)
  {
    IntegralProp_Impl *pImpl = new IntegralProp_Impl(this);
    std::ifstream in(dlg.GetPathName(), std::ios::binary);
    if (pImpl->Read(in))
    {
      delete m_pImpl;
      m_pImpl = pImpl;
      pImpl->Populate();
    }
    else
    {
      AfxMessageBox("Failed to load integral properties");
      delete pImpl;
    }
  }
}

void CIntegralProp::OnFileSave() 
{
  CFileDialog dlg(FALSE);
  if (dlg.DoModal() == IDOK)
  {
    std::ofstream out(dlg.GetPathName(), std::ios::binary);
    if (!m_pImpl->Write(out))
    {
      AfxMessageBox("Failed to store integral properties");
    }
  }
}

void CIntegralProp::OnFileClose() 
{
  SendMessage(WM_CLOSE);
}

float* CIntegralProp::GetData()
{
  if (!m_bActive || !m_wndList1.GetItemCount())
    return 0;

  CString str = m_wndList1.GetItemText(m_nItem, 0);
  return m_pImpl->GetData(str);
}

CActField::Palette* CIntegralProp::GetPalette()
{
  if (!m_bActive || !IsWindow(m_wndList1) || !m_wndList1.GetItemCount())
    return 0;

  CString str = m_wndList1.GetItemText(m_nItem, 0);
  return m_pImpl->GetPalette(str);
}


void CIntegralProp::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);

  CFieldForm* pFieldForm = GetFieldForm();
  pFieldForm->GetDlgItem(IDC_SLIDER1)->EnableWindow(!bShow);
  pFieldForm->GetDlgItem(IDC_EDIT1)->EnableWindow(!bShow);
  pFieldForm->GetDlgItem(IDC_SPIN1)->EnableWindow(!bShow);
  pFieldForm->GetDlgItem(IDC_COMBO1)->EnableWindow(!bShow);

  m_bActive = bShow;

  if (bShow)
  {
    if (m_wndList1.GetItemCount() > 0)
    {
      GetFieldView()->GetDocument()->UpdateAllViews(0, IDD);
    }
    else
    {
      GetFieldView()->GetDocument()->UpdateAllViews(0, 0);
    }

    if (m_wndContour.GetCheck())
    {
      m_wndDigitize.ShowWindow(SW_SHOW);
    }
  }
  else
  {
    GetFieldView()->GetDocument()->UpdateAllViews(0, 0);
    m_wndDigitize.ShowWindow(SW_HIDE);
  }
}

void CIntegralProp::OnClose() 
{
  OnShowWindow(FALSE, 0);
	CDialog::OnClose();
}

static bool LoadContour(const char* name, CFieldDoc* pDoc, CDigitizeDlg::Coord_list& data)
{
  std::ifstream in(name);

  int nn;
  std::string s;
  if (std::getline(in, s))
  {
    std::istringstream ss(s);
    if (!(ss >> nn))
    {
      return false;
    }
  }
  else
  {
    return false;
  }

  std::valarray<double> xx(nn), yy(nn);
  if (std::getline(in, s))
  {
    double x, y;
    std::istringstream ss(s);
    if (!(ss >> x >> y))
    {
      return false;
    }
    xx[0] = x;
    yy[0] = y;
  }
  else
  {
    return false;
  }

  int n = 0;
  while (std::getline(in, s))
  {
    double x, y;
    std::istringstream ss(s);
    if (!(ss >> x >> y))
    {
      break;
    }
    if (xx[n] != x && yy[n] != y)
    {
      if (++n >= nn-1)
      {
        break;
      }
      xx[n] = x;
      yy[n] = y;
    }
  }
  
  if (xx[0] != xx[n] || yy[0] != yy[n])
    ++n;

  for (int i = 0; i < n; ++i)
  {
    coord_type p;
    p.xcoord = (pDoc->cx - 1) * (xx[i] - pDoc->x1) / (pDoc->x2 - pDoc->x1);
    p.ycoord = (pDoc->cy - 1) * (yy[i] - pDoc->y1) / (pDoc->y2 - pDoc->y1);
    data.push_back(p);
  }
  return true;
}

void CIntegralProp::OnDigitize() 
{
  if (m_wndContour.GetCheck())
  {
    m_wndDigitize.data.clear();
    CString str;
    m_wndEdit8.GetWindowText(str);
    if (!str.IsEmpty())
    {
      if (!LoadContour(str, GetFieldView()->GetDocument(), m_wndDigitize.data))
      {
        m_wndDigitize.data.clear();
      }
    }
    m_wndDigitize.ShowWindow(SW_SHOW);
  }
  else
  {
    m_wndDigitize.ShowWindow(SW_HIDE);
  }
}

void CIntegralProp::OnCancel()
{
  OnShowWindow(FALSE, 0);
  CDialog::OnCancel();
}