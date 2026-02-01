#ifndef __PS_DATA_INCLUDED__
#define __PS_DATA_INCLUDED__

#include <vector>
#include <list>

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

#ifndef GOLDEN_RATIO
#define GOLDEN_RATIO 1.61803398874989484820458683346563811
#define M_PHI GOLDEN_RATIO
#endif

#ifndef countof
#define countof(arr) (sizeof(arr)/sizeof(*arr))
#endif

#pragma warning(disable: 4786 4512 4127 4511 4100)
// 4100 -- unreferenced formal parameter
// 4511 -- copy constructor could not be generated
// 4127 -- conditional expression is constant
// 4512 -- assignment operator could not be generated
// 4786 -- identifier was truncated to 'number' characters in the debug information

class psstream;

struct coord_type
{
  double xcoord;
  double ycoord;
};

typedef std::list<int> curve_type;
typedef std::vector<coord_type> vertex_type;
typedef std::list<curve_type> edge_type;

struct shape_curve
{
  const curve_type* curve;
  curve_type hull;
  double bbox[4];
};

typedef std::list<shape_curve> shape_list;

struct shape_type
{
  shape_list shape;
  curve_type hull;
  double bbox[4];
};

struct color_type
{
  double r, g, b;
};

struct grid_type
{
  short cx, cy;
  double x1, x2;
  double y1, y2;
  double z1, z2;
  float *zz;
};

struct noddle_type
{
  coord_type center;
  double angle;
};

typedef std::list<noddle_type> noise_type;

struct label_site
{
  char text[16];
  double xcoord;
  double ycoord;
  double bbox[4];
  double state[4];
  double param;
  double radius;
  int cost;
  int degree;
  label_site** cfl;
};

class psstream_impl;
class psstream
{
  psstream_impl* impl;
public:
  enum Orientation { PORTRAIT, LANDSCAPE };
  enum Format { FORMAT_A4, FORMAT_A3, FORMAT_A0, FORMAT_NUM };
  enum Pattern { BDIAGONAL, CROSSHATCH, DIAGHATCH, HORIZONTAL, FDIAGONAL, VERTICAL, 
    SANDSTONE, SHALE, LIMESTONE, DOLOMITE, SILICATE, SALT, ANHYDRITE, SILTSTONE,
    BITUMEN, OILZONE, WATZONE, GASZONE, GYPSUM, DEEPBASE, GASOILZONE,
    OILWATZONE, GASWATZONE, GASOILWATZONE, WATGASZONE, WATOILZONE, OILGASZONE,
    SAND, COAL, EVALUATION, NUM_PATTERNS };
  enum Annot { ANNOT_LEFT, ANNOT_CENTER, ANNOT_RIGHT };
  enum Justify { LABEL_E, LABEL_N, LABEL_W, LABEL_S, \
  	LABEL_NE, LABEL_NW, LABEL_SE, LABEL_SW, LABEL_NONE };
  struct Scale
  {
    enum Flags
    {
      HAS_INNER_TICK = 1,
      HAS_OUTER_TICK = 2,
      HAS_AXIS_LINE = 4,
      HAS_LABEL_TEXT = 8,
      HAS_GRID_LINES = 16,
      HAS_INNER_FRAME = 32,
      HAS_OUTER_FRAME = 64,
      HAS_COLOR_BOX = 128
    };

    enum Type
    {
    	ARITHMETIC, LOGARITHMIC, PROBABILITY
    };

    Type type;
    Justify label;
    int flags;
    double range[2];
    double region[4];
    double margin;
    double tick_length;
    double line_width;
    const char* font_name;
    double font_size;
    color_type color;
    int factor;
    const char* annot;
    const char* format;
    const char* units;
  };
  psstream(const char* file);
  ~psstream();
  void newpage();
  void newpath();
  void closepath();
  void fill();
  void stroke();
  bool setfont(const char* name, double size);
  void textbox(const char* text, double *bbox);
  void moveto(double x, double y);
  void lineto(double x, double y);
  void curveto(double x1, double y1, double x2, double y2, double x3, double y3);
  void rlineto(double x, double y);
  void show(const char* text);
  // postscript procedures
  void outlineshow(const char* text);
  void label_textproc(double x, double y, double r, double d, const char* s, bool f);
  void annot_textproc(double x, double y, double a, const char* s, bool f);
  void ltext_textproc(double x, double y, double r, double a, const char* s, bool f);
  void rtext_textproc(double x, double y, double r, double a, const char* s, bool f);
  void s_tolerance(double s);
  void min_collector(double p, double r, double g, double b);
  void max_collector(double p, double r, double g, double b);
  void beginproc(const char* name);
  void invokeproc(const char* name);
  void endproc();
  void tricolor(int n, const color_type* s);
  void trifill(double x1, double y1, double g1, double o1, double w1, double x2, 
    double y2, double g2, double o2, double w2, double x3, double y3, double g3, 
    double o3, double w3);
  void quadfill(double x1, double y1, double g1, double o1, double w1, double x2, 
    double y2, double g2, double o2, double w2, double x3, double y3, double g3, 
    double o3, double w3, double x4, double y4, double g4, double o4, double w4);
  void pixelplot(int cx, int cy, const float* xx, const float* yy, 
    const float* gg, const float* oo, const float* ww);
  void horizontal_scale(int start, int step, int final, double orig, double range, 
    double dx, double x0, double y0, double sy);
  void vertical_scale(int start, int step, int final, double orig, double range, 
    double dy, double x0, double y0, double sx);
  void horizontal(const Scale& scale);
  void vertical(const Scale& scale);
  // ...
  void setdash(int n, int* pp, int o);
  void setgray(double g);
  void setrgbcolor(double r, double g, double b);
  void setcmykcolor(double c, double m, double y, double k);
  void setcolorspace(double line_width, double scale_factor);
  void setcolor(double r, double g, double b, Pattern h);
  void translate(double x, double y);
  void rotate(double a);
  void arc(double x, double y, double r, double a1, double a2);
  void arcn(double x, double y, double r, double a1, double a2);
  void setlinewidth(double w);
  void selectmedia(Format f, Orientation o);
  void annot(double x, double y, double r, double angle, \
    Annot anchor, bool show, const char* text, double bbox[8]);
  double pagewidth() const;
  double pageheight() const;
  void rectstroke(double x, double y, double w, double h);
  void rectfill(double x, double y, double w, double h);
  void colorimage(double x0, double y0, double sx, double sy, \
    int num_levels, const float* level, const color_type* color, \
    int cx, int cy, const float* func);
  void grayscaleimage(double x0, double y0, double sx, double sy, \
    int num_levels, const float* level, const double* color, \
    int cx, int cy, const float* func);
  void bilevelimage(double x0, double y0, double sx, double sy, \
    int cx, int cy, const vertex_type& ver, const edge_type& pgn);
  void grayscaleimage(double x0, double y0, double sx, double sy, \
    int cx, int cy, const vertex_type& ver, const edge_type& pgn);
  void gsave();
  void grestore();
  void clip();
};

typedef psstream::Annot annot_type;
typedef psstream::Justify label_type;

struct pgn_builder
{
  virtual int new_vertex(double x, double y) = 0;
  virtual void begin_curve() = 0;
  virtual void add_vertex(int) = 0;
  virtual void end_curve() = 0;
};

class back_polygon: public pgn_builder
{
  vertex_type ver;
  curve_type cur;
  psstream& out;
public:
  back_polygon(psstream& o);
  int new_vertex(double x, double y);
  void begin_curve();
  void add_vertex(int);
  void end_curve();
};

class clip_polygon: public pgn_builder
{
  vertex_type& ver;
  edge_type& pgn;
  curve_type* cur;
public:
  clip_polygon(vertex_type& v, edge_type& e);
  int new_vertex(double x, double y);
  void begin_curve();
  void add_vertex(int);
  void end_curve();
};

class bounding_box
{
  const coord_type* ver;
public:
  bounding_box(const coord_type* p);
  double* operator()(double* bb, int n) const;
  static double* max(double* bb);
  static double* init(const coord_type& p, double* bb);
};

void textsite(double x, double y, double r, double d, double bb[4]);
void label(psstream& out, double x, double y, double r, label_type justify, \
  bool show, const char* text, double bb[4]);
void hlabel(psstream& out, double x, double y, double r, label_type justify, \
  bool show, const char* text, double bb[4]);
void vlabel(psstream& out, double x, double y, double r, label_type justify, \
  bool show, const char* text, double bb[4]);
bool read_grid(const char* name, grid_type* data);
void histeq(int n_data, const float* data, int n_level, float* level);
void plotpath(psstream& out, const vertex_type& ver, const edge_type& ee);
void resize_box(double *bbox, double delta);
bool inside_bbox(double x, double y, const double *bb);
// bool inside_shape(double x, double y, const vertex_type& ver, const shape_type& ss);
// bool inside_curve(double x, double y, const vertex_type& ver, const curve_type& cc);
// bool inside_edge(double x, double y, const vertex_type& ver, const edge_type& ee);
// void edge_shape(const vertex_type& ver, const edge_type& ee, shape_type& ss);
void sutherland_hodgman(double a, double b, double c, vertex_type& ver, \
  const edge_type& org, edge_type& pos, edge_type& neg);
void edge_clip(double x1, double y1, double x2, double y2, vertex_type& ver, edge_type& pgn);
double edge_area(const vertex_type& ver, const edge_type& ee);
void edge_mask(const vertex_type& ver, const edge_type& pgn, grid_type* g);
void convert(double tx, double ty, double sx, double sy, vertex_type& ver);
void make_noise(double x1, double y1, double x2, double y2, \
  double r1, double r2, double dr, double a, double da, \
  vertex_type& ver, edge_type& pgn, int n_constraint, noise_type* data[], 
  const double *ext1, const double *ext2);
void edge_rand(double x0, double y0, double alpha, \
  double r1, double r2, double dr, int n, int m, vertex_type& ver, edge_type& pgn);
void edge_shuffle(double aperture, const edge_type& src, edge_type& dst);
void hatch_edge(psstream& out, double line_width, int hatch_step, \
  const vertex_type& ver, const edge_type& ee);
void annotpath(psstream& out, const vertex_type& path_ver, const edge_type& path_edge, \
  const char* text, double step, double gap, pgn_builder* pgn);
void auto_conrec(psstream& out, int i1, int i2, int j1, int j2, grid_type* g, \
  double x0, double y0, double sx, double sy);
void cyrus_beck(double* bbox, const vertex_type& clip_ver, \
  const curve_type& clip_cur, vertex_type& ver, edge_type& ee);
void clippath(vertex_type& ver, int n, edge_type* ee, \
  const vertex_type& clip_ver, const edge_type& clip_pgn);
bool rc_intersect(const double* bb, double x0, double y0, double r);
bool rr_intersect(const double *b1, const double *b2);
bool sub(grid_type* g1, grid_type* g2, grid_type* g3);
void blin(const coord_type& p, const coord_type& q, vertex_type& ver);
template <typename T>
extern const T* lookup_color(double value, int num_levels, const float* level, \
  const T* color, T* temp);
bool merge_curves(edge_type& ee);
void merge_nodes(const vertex_type& ver, edge_type& pgn);
void blankimage(const grid_type* g, int ilb, int iub, int jlb, int jub, \
	vertex_type& ver, edge_type& pgn);
void marching_squares(int cx, int cy, int ilb, int iub, int jlb, int jub, \
  const float* elev, int num_levels, float* level, 
  vertex_type& m_ver, edge_type* m_edge, edge_type* c_edge);

void voro(int np, const double xx[1], const double yy[1], \
  double x1, double x2, double y1, double y2, \
  vertex_type& ver, edge_type& pgn, curve_type* arr[1]);

enum Classify { LEFT, RIGHT, BEHIND, BEYOND, BETWEEN, ORIGIN, DESTINATION };
enum Intersect { COLLINEAR, PARALLEL, SKEW, SKEW_CROSS, SKEW_NO_CROSS };
enum Raycast { TOUCHING, CROSSING, INESSENTIAL };
enum Inpoly { INSIDE, OUTSIDE, BOUNDARY };

extern const char* classify_table[];
extern const char* intersect_table[];
extern const char* raycast_table[];
extern const char* inpoly_table[];

double perpendicular(double x1, double y1, double x2, double y2, double x, double y);
double parallel(double x1, double y1, double x2, double y2, double x, double y);
Classify classify(double x1, double y1, double x2, double y2, double x, double y);

Intersect intersect(double x1, double y1, double x2, double y2, \
  double x3, double y3, double x4, double y4, double* t);

Intersect cross(double x1, double y1, double x2, double y2, \
  double x3, double y3, double x4, double y4, double *s, double* t);

Raycast raycast(const coord_type& p, const coord_type& q, double x, double y);
Inpoly inside_curve(double x, double y, const vertex_type& ver, const curve_type& cur);
Inpoly inside_edge(double x, double y, const vertex_type& ver, const edge_type& pgn);
bool inside_convex(double x, double y, const vertex_type& ver, const curve_type& cur);
bool iscrossed(double bbox[4], const vertex_type& ver, const curve_type& cur);
// edge_shape() is obsolete, use *_shape_list() below
// void edge_shape(const vertex_type& ver, const edge_type& pgn, shape_type* init);
Inpoly inside_shape(double x, double y, const shape_type& pgn);

void clip_convex(const vertex_type& ver_convex, const curve_type& cur_convex, \
  const vertex_type& ver, const edge_type& pgn, vertex_type& ver_clipped, edge_type& pgn_clipped);

void clone(const vertex_type& in, const curve_type& src, int *xlat, \
  vertex_type& ver, edge_type& pgn);

bool is_lexord(const vertex_type& ver);
int find(const vertex_type& ver, const coord_type& p);
void lexord(const vertex_type& ver, int n, int* data, vertex_type& dict);
void graham_scan(const vertex_type& ver, const curve_type& cur, curve_type& hull);
// double* curve_bbox(const vertex_type& ver, const curve_type& cur, double bb[4]);
void min_rect(const vertex_type& ver, const curve_type& cur, double rect[8]);
void boxedge(const double *bbox, vertex_type& ver, edge_type& pgn);

double* curve_bbox(const vertex_type& ver, const curve_type& cur, double bb[4]);
double* closed_curve_bbox(const vertex_type& ver, const curve_type& cur, double bb[4]);
bool is_empty(const curve_type& cur);
bool is_point(const curve_type& cur);
bool is_segment(const curve_type& cur);
bool is_closed(const curve_type& cur);
bool has_duplicate_points(const vertex_type& ver, const curve_type& cur);
bool has_degenerate_segment(const vertex_type& ver, const curve_type& cur);
bool is_degenerate_curve(const vertex_type& ver, const curve_type& cur);
bool is_simple(const vertex_type& ver, const curve_type& cur);
bool is_self_crossed(const vertex_type& ver, const curve_type& cur);
bool is_weakly_simple(const vertex_type& ver, const curve_type& cur);
bool is_convex(const vertex_type& ver, const curve_type& cur);
bool is_generalized_polygon(const vertex_type& ver, const edge_type& pgn);

void plot_curve(const vertex_type& ver, const curve_type& cur);

void make_shape_list(const vertex_type& ver, const edge_type& pgn, shape_type* init);
void bbox_shape_list(const vertex_type& ver, shape_type* init);
void hull_shape_list(const vertex_type& ver, shape_type* init);
double area(const vertex_type& ver, const curve_type& cur);

int setup_initial_state(int n, label_site** pflp);
void compute_conflict_graph(int n, label_site** pflp);
void graphics_output(psstream& out, int n, label_site** pflp);
void schedule_annealing(int n, label_site** pflp);

#include "ps_font.h"

#endif
