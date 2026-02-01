#include <map>
#include <list>
#include <fstream>

class psstream_impl
{
  typedef std::map<std::string, const short*> font_table;
  
  struct page_type 
  { 
    char* code;
    double bbox[4];
    bool has_bbox;
    double curr[2];
    bool has_curr;
  };

  typedef std::list<page_type*> page_list;

  page_list script;
  char* font_resource;
  char* setup_section;
  char* output_file;
  std::ofstream out;
  page_type* page;

  font_table font_data;
  const short* glyph_data;
  double font_size;
  double line_width;

  psstream::Format format;
  psstream::Orientation orientation;

  bool has_outlineshow;
  bool has_textproc;
  bool has_lerpproc;
  bool has_initarray;
  bool is_procedure;

  bool has_pattern[psstream::NUM_PATTERNS];
  double pattern_line_width;
  double pattern_scale_factor;

  void textproc();
  void lerpproc();
  void initarray();

public:
  psstream_impl(const char* file);
  ~psstream_impl();
  bool setfont(const char* name, double size);
  void textbox(const char* text, double *bbox);
  void newpage();
  void moveto(double x, double y);
  void lineto(double x, double y);
  void curveto(double x1, double y1, double x2, double y2, double x3, double y3);
  void rlineto(double x, double y);
  void show(const char* text);
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
  void setdash(int n, int* pp, int o);
  void setgray(double g);
  void setrgbcolor(double r, double g, double b);
  void setcmykcolor(double c, double m, double y, double k);
  void setcolorspace(double line_width, double scale_factor);
  void setcolor(double r, double g, double b, psstream::Pattern h);
  void translate(double x, double y);
  void rotate(double a);
  void command(const char* cmd);
  void arc(double x, double y, double r, double a1, double a2);
  void arcn(double x, double y, double r, double a1, double a2);
  void setlinewidth(double w);
  void updatebb(double x1, double x2, double y1, double y2);
  void updatecp(double x, double y);
  void selectmedia(psstream::Format f, psstream::Orientation o);
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
  bool getbbox(double *bb);
  void setbbox(bool f, double *bb);

  void horizontal_scale(int start, int step, int final, double orig, double range, 
    double dx, double x0, double y0, double sx);
  void vertical_scale(int start, int step, int final, double orig, double range, 
    double dy, double x0, double y0, double sx);
};

double modulo_range(double x, double modulo);
