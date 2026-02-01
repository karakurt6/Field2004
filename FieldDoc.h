#ifndef FIELDDOC_INCLUDED
#define FIELDDOC_INCLUDED

#pragma warning(disable: 4786)

#include <atldbcli.h>

class WellSite;
class FieldRef;

#include "stuff\ps_data.h"

class CFieldDoc: public CDocument
{
protected:
  CFieldDoc();
  DECLARE_DYNCREATE(CFieldDoc)

// Typedefs
public:
  struct Point 
  { 
    double x, y; 
  };

  struct TrajItem // mandatory
  {
    double d, x, y, z;
  };

  struct PerfItem // optoional
  {
    long perf_form;
		DATE perf_comp;
		DATE perf_close;
		bool perf_done;
    float perf_upper;
    float perf_lower;
  };

  struct InfoItem // optional
  {
    long bed, lit, sat;
    float a_sp, clay, f_res, kg, kng;
    float top, bot, perm, poro;
  };

  struct ProdItem // fill by query
  {
    long bed;
    unsigned char days;
    float oil, wat, gas;
  };

  struct PumpItem // fill by query
  {
    long bed;
    unsigned char days;
    float fluid;
  };

  struct DataField
  {
    char name[8];
    char info[64];
    char units[8];
    unsigned char color_mapping;
    float valid_range[2];
    int id;
  };

  struct LogtrackItem
  {
    long bore;
    unsigned char run;
    unsigned char pass;
    unsigned char ver;
    char type[16];
    char unit[64];
    float null;
    float minval;
    float maxval;
    long num_data;
    char depth_type[4];
    char depth_unit[64];
    float depth_step;
    float min_depth;
    float max_depth;
    float* value;

    BEGIN_COLUMN_MAP(LogtrackItem)
      COLUMN_ENTRY(1, bore)
      COLUMN_ENTRY(2, run)
      COLUMN_ENTRY(3, pass)
      COLUMN_ENTRY(4, ver)
      COLUMN_ENTRY(5, type)
      COLUMN_ENTRY(6, unit)
      COLUMN_ENTRY(7, null)
      COLUMN_ENTRY(8, minval)
      COLUMN_ENTRY(9, maxval)
      COLUMN_ENTRY(10, num_data)
      COLUMN_ENTRY(11, depth_type)
      COLUMN_ENTRY(12, depth_unit)
      COLUMN_ENTRY(13, depth_step)
      COLUMN_ENTRY(14, min_depth)
      COLUMN_ENTRY(15, max_depth)
    END_COLUMN_MAP()
  };

	struct DictRecord
	{
		long rec_item;
		long rec_group;
		char m_rus[16];
		char m_eng[16];
		char t_rus[16];
		char t_eng[16];
		char d_rus[64];
		char d_eng[64];

		struct Status
		{
			DBSTATUS m_rus;
			DBSTATUS m_eng;
			DBSTATUS t_rus;
			DBSTATUS t_eng;
			DBSTATUS d_rus;
			DBSTATUS d_eng;
		};

		Status status;

 	  BEGIN_COLUMN_MAP(DictRecord)
 			COLUMN_ENTRY(2, rec_group)
 			COLUMN_ENTRY(1, rec_item)
 			COLUMN_ENTRY_STATUS(3, m_rus, status.m_rus)
 			COLUMN_ENTRY_STATUS(4, m_eng, status.m_eng)
 			COLUMN_ENTRY_STATUS(5, t_rus, status.t_rus)
 			COLUMN_ENTRY_STATUS(6, t_eng, status.t_eng)
 			COLUMN_ENTRY_STATUS(7, d_rus, status.d_rus)
 			COLUMN_ENTRY_STATUS(8, d_eng, status.d_eng)
 	  END_COLUMN_MAP()
	};

// Attributes
public:
  // int hf; // old multidimensional data
	// CDaoDatabase db; // old geological database
	ATL::CSession session; // geological information
	int hdf5_file; // hierarchical data format storage
	int hdf5_model; // multidimensional data

  int cx, cy, cz; // volume dimensions
  double x1, x2, y1, y2, z1, z2; // bounding box

  int WindowQuery(const Point& a, const Point& b, int nb, long* bb) const;
  int RangeQuery(const Point& c, double r, int nb, long* bb) const;
  int ProdQuery(COleDateTime dt, int nb, long* bb) const;
  int PumpQuery(COleDateTime dt, int nb, long* bb) const;
  int FieldEnum(int nd, const DataField* dd[]) const;

  double TrajLength(long b);
  int Logtrack(long b, const char* type, int np, LogtrackItem pp[]);
  void ExcelOutput(long bore);

  int Traj(long b, int n, double *p) const;
  int Perf(long b, int n, PerfItem* p[]) const;
  int Info(long b, int n, InfoItem* p[]) const;
  int Prod(long b, int n, ProdItem* p[]) const;
  int Pump(long b, int n, PumpItem* p[]) const;
  const DataField* Field(const char* name) const;
	void DateRange(COleDateTime range[2]) const;

  enum OutsideFlag
  {
    RIGHT_SIDE = 1,
    LEFT_SIDE = 2,
    FRONT_SIDE = 4,
    BACK_SIDE = 8,
    TOP_SIDE = 16,
    BOTTOM_SIDE = 32
  };
  int Coord(const double pp[3], double xx[3]) const;
  int Cell(const double xx[3], double pp[3]) const;
  float Value(const double s[3], int id, float (*fn)(float)) const;
	float NodeValue(const double s[3], int id, float (*fn)(float)) const;

// Operations
public:
  BOOL OpenDatasetFile(const char* name, const char* group);
  BOOL OpenDatabaseFile(LPCTSTR name);
	void UpdateWellStock();

// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CFieldDoc)
public:
  virtual BOOL OnNewDocument();
  virtual void DeleteContents();
  //}}AFX_VIRTUAL

// Implementation
public:
  virtual ~CFieldDoc();

// Diagnostics
#ifdef _DEBUG
  enum DumpContext
  {
    DUMP_WELLSITE = 1,
    DUMP_WELLPROD = 2,
    DUMP_WELLPUMP = 4
  };
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext& dc) const;
#endif

private:
  int sz; // number of elements in surface arrays
  short* nz; // array of surface indices
  float* zz; // array of surface elevation grids
  WellSite* ww; // searching datastruct
  FieldRef* ff; // searching datastruct
	long base_formation; // reference surface

	struct HistDates
	{
		DATE start;
		DATE final;
		BEGIN_COLUMN_MAP(HistDates)
			COLUMN_ENTRY(3, start)
			COLUMN_ENTRY(4, final)
		END_COLUMN_MAP()
	};

// Generated message map functions
protected:
  //{{AFX_MSG(CFieldDoc)
    // NOTE - the ClassWizard will add and remove member functions here.
    //    DO NOT EDIT what you see in these blocks of generated code !
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()

  friend BOOL Dataset_ReadElevation(CFieldDoc* pDoc);
};

#define INSIDE_DOM(flag) ((flag) == 0)

long PointQuery(const CFieldDoc* pDoc, double x, double y);
void DateRange(CFieldDoc* pDoc, COleDateTime range[2]);

class CActField: public CFieldDoc
{
  CActField();
  DECLARE_DYNCREATE(CActField)

  // Attributes
public:
	typedef color_type Color;

  struct Palette
  {
    int nz;
    float* zz;
    Color* sh;
    float (*fwd)(float);
    float (*bck)(float);
  };

  const DataField* field;
  int level;
  float *func;
  Palette clut;
  short year;

  int FieldInfo(long b, int n, double *p) const;

  // Operations
public:
  void UpdateSelection();
  void SelectProp(int n);
  void InitArithmeticClut();
  void UpdateYear(short s);

// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CActField)
public:
  virtual BOOL OnNewDocument();
  virtual void DeleteContents();
  //}}AFX_VIRTUAL

// Diagnostics
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext& dc) const;
#endif

// Implementation
public:
  virtual ~CActField();

// Generated message map functions
protected:
  //{{AFX_MSG(CActField)
    // NOTE - the ClassWizard will add and remove member functions here.
    //    DO NOT EDIT what you see in these blocks of generated code !
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

void FreeLogtrack(int np, CFieldDoc::LogtrackItem* pp);
bool LookupWellName(CActField* pDoc, long bore, char* name);

#endif
