#define NOMINMAX

#include <windows.h>
#include <algorithm>
#include <map>
#include <list>
#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <assert.h>
#include <zlib.h>
#include <math.h>

#include "ps_data.h"
#include "ps_priv.h"

#ifndef PSSTERAM_MODULE
#define PSSTREAM_MODULE "ps_font.dll"
#endif

static const char* media_name[psstream::FORMAT_NUM] =
{
  "A4", "A3", "A0"
};
static const int media_width[psstream::FORMAT_NUM] =
{
  595, 842, 2384
};
static const int media_height[psstream::FORMAT_NUM] =
{
  842, 1191, 3370
};

/////////////////////////////////////////////////////////////////////

static char* temp_name(const char* prefix)
{
  TCHAR path[_MAX_PATH];
  if (!GetTempPath(_MAX_PATH, path))
    return 0;

  TCHAR spec[_MAX_PATH];
  if (!GetTempFileName(path, prefix, 0, spec))
    return 0;

  char* ch = new char[lstrlen(spec)+1];
  lstrcpy(ch, spec);
  return ch;
}

double modulo_range(double x, double modulo)
{
  if (x < 0.0)
  {
    x += modulo * ceil(-x / modulo);
  }
  else if (x >= modulo)
  {
    x -= modulo * floor(x / modulo);
  }
  return x;
}

static void append(std::ostream& out, const char* file)
{
  std::ifstream in(file);
  std::string rec;
  while (std::getline(in, rec))
  {
    out << rec << '\n';
  }
}

static std::ostream& init_doc(const char* name, std::ofstream& file)
{
  if (*name != 0 && *name != '\0' && stricmp(name, "con") != 0 
  	&& stricmp(name, "con:") != 0 && stricmp(name, "conout$") != 0)
  {
    file.open(name);
    return file;
  }
  return std::cout;
}

/////////////////////////////////////////////////////////////////////

psstream_impl::psstream_impl(const char* file)
{
	output_file = strcpy(new char[strlen(file)+1], file);
  font_resource = temp_name("PSS");
  setup_section = temp_name("PSS");
  out.open(setup_section);
  page = 0;
  glyph_data = 0;
  font_size = line_width = 1.0;
  format = psstream::FORMAT_A4;
  orientation = psstream::PORTRAIT;
  has_outlineshow = false;
  has_textproc = false;
  has_lerpproc = false;
  has_initarray = false;
  is_procedure = false;

  std::fill(has_pattern, has_pattern + psstream::NUM_PATTERNS, false);
  pattern_line_width = 0.0;
  pattern_scale_factor = 2.0;
}

psstream_impl::~psstream_impl()
{
  if (out.is_open())
    out.close();

  std::ofstream of;
  std::ostream& doc = init_doc(output_file, of);

  int page_number = script.size();
  if (page_number > 0)
  {
    int bb[4];
    page_list::iterator it = script.begin();
    page_type *page = *it;
    bb[0] = (int) floor(page->bbox[0]);
    bb[1] = (int) ceil(page->bbox[1]);
    bb[2] = (int) floor(page->bbox[2]);
    bb[3] = (int) ceil(page->bbox[3]);
    while (++it != script.end())
    {
      page = *it;
      int x1 = (int) floor(page->bbox[0]);
      int x2 = (int) ceil(page->bbox[1]);
      int y1 = (int) floor(page->bbox[2]);
      int y2 = (int) ceil(page->bbox[3]);
      if (bb[0] > x1) bb[0] = x1;
      if (bb[1] < x2) bb[1] = x2;
      if (bb[2] > y1) bb[2] = y1;
      if (bb[3] < y2) bb[3] = y2;
    }

    doc << "%!PS-Adobe-3.0\n%%LanguageLevel: 2\n%%BoundingBox: "; 
			if (orientation == psstream::PORTRAIT)
    {
      doc << bb[0] << ' ' << bb[2] << ' ' << bb[1] << ' ' << bb[3];
    }
    else
    {
      doc << pageheight() - bb[2] << ' ' << bb[0] << ' ' 
				<< pageheight() - bb[3] << ' ' << bb[1];
    }
    doc << "\n%%Pages: " << page_number << "\n%%DocumentMedia: "
      << media_name[format] << ' ' << media_width[format] << ' '
      << media_height[format] << " 80 white ( )\n%%Orientation: " 
      << (orientation == psstream::LANDSCAPE? "Landscape": "Portrait") 
      << "\n%%EndComments\n%%BeginProlog\n";
    append(doc, font_resource);
    doc << "%%EndProlog\n%%BeginSetup\n";
    append(doc, setup_section);
    doc << "%%EndSetup\n";

    page_number = 1;
    for (it = script.begin(); it != script.end(); ++it)
    {
      page = *it;
      doc << "%%Page: " << page_number << ' ' << page_number << '\n';
      if (page->has_bbox)
      {
				if (orientation == psstream::PORTRAIT)
        {
          doc << "%%PageBoundingBox: " << floor(page->bbox[0]) << ' ' 
            << floor(page->bbox[2]) << ' ' << ceil(page->bbox[1]) << ' ' 
            << ceil(page->bbox[3]) << '\n';
        }
        else
        {
          doc << "%%PageBoundingBox: " << pageheight() - floor(page->bbox[2]) << ' ' 
            << floor(page->bbox[0]) << ' ' << pageheight() - ceil(page->bbox[3]) << ' ' 
            << ceil(page->bbox[1]) << '\n';
        }
      }
      doc << "%%BeginPageSetup\nsave\n";
      if (orientation == psstream::LANDSCAPE)
        doc << "90 rotate 0 " << -media_width[format] << " translate\n";
      doc << "%%EndPageSetup\n";
      append(doc, page->code);
      doc << "restore\nshowpage\n";
      ++page_number;
      DeleteFile(page->code);
      delete[] page->code;
      delete page;
    }
    doc << "%%Trailer\n%%EOF\n";
  }
  DeleteFile(setup_section);
  DeleteFile(font_resource);
  delete[] setup_section;
  delete[] font_resource;
  delete[] output_file;
}

void psstream_impl::updatebb(double x1, double x2, double y1, double y2)
{
  if (page->has_bbox)
  {
    if (page->bbox[0] > x1) page->bbox[0] = x1;
    if (page->bbox[1] < x2) page->bbox[1] = x2;
    if (page->bbox[2] > y1) page->bbox[2] = y1;
    if (page->bbox[3] < y2) page->bbox[3] = y2;
  }
  else 
  {
    page->bbox[0] = x1;
    page->bbox[1] = x2;
    page->bbox[2] = y1;
    page->bbox[3] = y2;
    page->has_bbox = true;
  }
}

void psstream_impl::updatecp(double x, double y)
{
  page->curr[0] = x;
  page->curr[1] = y;
  page->has_curr = true;
}

inline void psstream_impl::newpage()
{
  page = new page_type;
  script.push_back(page);
  page->code = temp_name("PSS");
  if (out.is_open())
    out.close();
  out.open(page->code);
  page->has_bbox = false;
  page->has_curr = false;
}

inline void psstream_impl::moveto(double x, double y)
{
  out << x << ' ' << y << " moveto\n";
  updatecp(x, y);
  updatebb(x, x, y, y);
}

inline void psstream_impl::lineto(double x, double y)
{
  out << x << ' ' << y << " lineto\n";
  updatecp(x, y);
  updatebb(x, x, y, y);
}

inline void psstream_impl::curveto(double x1, double y1, double x2, double y2, double x3, double y3)
{
  out << x1 << ' ' << y1 << ' ' << x2 << ' ' << y2 << ' ' << x3 << ' ' << y3 << " curveto\n";
  updatecp(x3, y3);
  updatebb(std::min(std::min(x1, x2), x3), std::max(std::max(x1, x2), x3), \
  	std::min(std::min(y1, y2), y3), std::max(std::max(y1, y2), y3));
}

inline void psstream_impl::rlineto(double x, double y)
{
  out << x << ' ' << y << " rlineto\n";
  if (page->has_curr)
  {
    page->curr[0] += x;
    page->curr[1] += y;
    updatebb(page->curr[0], page->curr[0], page->curr[1], page->curr[1]);
  }
}

inline void psstream_impl::show(const char* text)
{
  if (!page->has_curr)
    return;
#if 1
  out << '<';
  int k = 0;
  for (const char *ch = text; *ch; ++ch)
  {
  	out << std::hex << std::setfill('0') << std::setw(2) << (int) (unsigned char) *ch;
  	if (++k % 36 == 0) out << '\n';
  }
  out << "> show\n" << std::dec;
#else
  out << '(' << text << ") show\n";
#endif
  double bb[4];
  textbox(text, bb);
  updatebb(page->curr[0]+bb[0], page->curr[0]+bb[2], \
    page->curr[1]+bb[1], page->curr[1]+bb[3]);
  updatecp(page->curr[0]+bb[2]-bb[0], page->curr[1]);
}

inline void psstream_impl::setgray(double g)
{
  out << g << " setgray\n";
}

inline void psstream_impl::setrgbcolor(double r, double g, double b)
{
  out << r << ' ' << g << ' ' << b << " setrgbcolor\n";
}

inline void psstream_impl::setcmykcolor(double c, double m, double y, double k)
{
	out << c << ' ' << m << ' ' << y << ' ' << k << " setcmykcolor\n";
}

inline void psstream_impl::translate(double x, double y)
{
  out << x << ' ' << y << " translate\n";
}

inline void psstream_impl::rotate(double a)
{
  out << a << " rotate\n";
}

inline void psstream_impl::command(const char* cmd)
{
  out << cmd << '\n';
}

static void unit_arc_bbox(double a1, double a2, double bb[4])
{
  if (a1 > a2)
    a2 += 2*M_PI;
  bb[0] = bb[1] = cos(a1);
  bb[2] = bb[3] = sin(a1);
  for (double a = 0.5*M_PI*ceil(a1/(0.5*M_PI)); a < a2; a += 0.5*M_PI)
  {
    double x = cos(a), y = sin(a);
    if (bb[0] > x) bb[0] = x;
    if (bb[1] < x) bb[1] = x;
    if (bb[2] > y) bb[2] = y;
    if (bb[3] < y) bb[3] = y;
  }
  double x = cos(a2), y = sin(a2);
  if (bb[0] > x) bb[0] = x;
  if (bb[1] < x) bb[1] = x;
  if (bb[2] > y) bb[2] = y;
  if (bb[3] < y) bb[3] = y;
}

inline void psstream_impl::arc(double x, double y, double r, double a1, double a2)
{
  out << x << ' ' << y << ' ' << r << ' ' << a1 << ' ' << a2 << " arc\n";
  a1 = modulo_range(a1*M_PI/180.0, 2*M_PI);
  a2 = modulo_range(a2*M_PI/180.0, 2*M_PI);
  double bb[4];
  unit_arc_bbox(a1, a2, bb);
  updatebb(x+r*bb[0], x+r*bb[1], y+r*bb[2], y+r*bb[3]);
  updatecp(x+r*cos(a2), y+r*sin(a2));
}

inline void psstream_impl::arcn(double x, double y, double r, double a1, double a2)
{
  out << x << ' ' << y << ' ' << r << ' ' << a1 << ' ' << a2 << " arcn\n";
  a1 = modulo_range(a1*M_PI/180.0, 2*M_PI);
  a2 = modulo_range(a2*M_PI/180.0, 2*M_PI);
  double bb[4];
  unit_arc_bbox(a2, a1, bb);
  updatebb(x+r*bb[0], x+r*bb[1], y+r*bb[2], y+r*bb[3]);
  updatecp(x+r*cos(a2), y+r*sin(a2));
}

inline void psstream_impl::setlinewidth(double w)
{
  out << w << " setlinewidth\n";
  line_width = w;  
}

bool psstream_impl::setfont(const char* name, double scale)
{
  font_table::iterator it = font_data.find(name);
  if (it == font_data.end())
  {
    HMODULE hh = GetModuleHandle(PSSTREAM_MODULE);
    if (!hh)
    {
      hh = LoadLibrary(PSSTREAM_MODULE);
      if (!hh)
        return false;
    }

    HRSRC rc = FindResource(hh, name, "pfmz");
    if (!rc)
      return false;

    HGLOBAL data = LoadResource(hh, rc);
    if (!data)
      return false;

    std::ofstream app(font_resource, std::ios::app);
    short *addr = (short*) LockResource(data) - 32*5; 
    DWORD size = SizeofResource(hh, rc);
    if (size > sizeof(short)*(256-32)*5)
    {
      app << "%%BeginResource: font " << name << '\n';
      z_stream zz;
      zz.zalloc = Z_NULL;
      zz.zfree = Z_NULL;
      zz.opaque = 0;
      zz.next_in = (Bytef*) (addr + 5 * 256);
      zz.avail_in = size - sizeof(short)*(256-32)*5;
      int code = inflateInit(&zz);
      if (code != Z_OK)
        return false;

      char buff[512];
      zz.next_out = (Bytef*) buff;
      zz.avail_out = sizeof(buff);
      code = inflate(&zz, Z_NO_FLUSH);
      while (code == Z_OK)
      {
        app.write(buff, sizeof(buff) - zz.avail_out);
        zz.next_out = (Bytef*) buff;
        zz.avail_out = sizeof(buff);
        code = inflate(&zz, Z_NO_FLUSH);
      }

      if (code != Z_STREAM_END)
      {
        code = inflateEnd(&zz);
        return false;
      }

      app.write(buff, sizeof(buff) - zz.avail_out);
      code = inflateEnd(&zz);
      app << "%%EndResource\n";
    }
    
    font_table::key_type key = name;
    font_table::value_type val = std::make_pair(key, addr);
    it = font_data.insert(val).first;
  }
  out << '/' << name << " findfont " << scale << " scalefont setfont\n";
  glyph_data = it->second;
  font_size = scale;
  return true;
}

void resize_box(double *bbox, double delta)
{
  bbox[0] -= delta;
  bbox[1] -= delta;
  bbox[2] += delta;
  bbox[3] += delta;
}

void psstream_impl::textbox(const char* text, double* bbox)
{
  const short* bb = glyph_data;
  double s = font_size;
  const unsigned char* ch = (unsigned char*) text;
  if (*ch)
  {
    int c = *ch++;
    const short *b = bb + 5 * c;
    if (*ch)
    {
      bbox[0] = s * b[1] * 0.001;
      bbox[1] = s * b[2] * 0.001;
      bbox[2] = s * b[0] * 0.001;
      bbox[3] = s * b[4] * 0.001;

      for (c = *ch++; *ch; c = *ch++)
      {
        b = bb + 5 * c;
        bbox[1] = std::min(bbox[1], s * b[2] * 0.001);
        bbox[2] = bbox[2] + s * b[0] * 0.001;
        bbox[3] = std::max(bbox[3], s * b[4] * 0.001);
      }

      b = bb + 5 * c;
      bbox[1] = std::min(bbox[1], s * b[2] * 0.001);
      bbox[2] = bbox[2] + s * b[3] * 0.001;
      bbox[3] = std::max(bbox[3], s * b[4] * 0.001);
    }
    else
    {
      bbox[0] = s * b[1] * 0.001;
      bbox[1] = s * b[2] * 0.001;
      bbox[2] = s * b[0] * 0.001;
      bbox[3] = s * b[4] * 0.001;
    }
  }
  // resize_box(bbox, line_width);
}

void psstream_impl::selectmedia(psstream::Format f, psstream::Orientation o)
{
  if (page != 0)
    return;

  format = f;
  orientation = o;
  out << "<< /PageSize [" << media_width[f] << ' ' << media_height[f] << "] >> setpagedevice\n";
}

double psstream_impl::pagewidth() const
{
  if (orientation == psstream::PORTRAIT)
  {
    return media_width[format];
  }
  return media_height[format];
}

double psstream_impl::pageheight() const
{
  if (orientation == psstream::LANDSCAPE)
  {
    return media_width[format];
  }
  return media_height[format];
}

void psstream_impl::rectstroke(double x, double y, double w, double h)
{
  out << x << ' ' << y << ' ' << w << ' ' << h << " rectstroke\n";
  updatebb(x, x+w, y, y+h);
}

void psstream_impl::rectfill(double x, double y, double w, double h)
{
  out << x << ' ' << y << ' ' << w << ' ' << h << " rectfill\n";
  updatebb(x, x+w, y, y+h);
}

bool psstream_impl::getbbox(double *bb)
{
  if (!page->has_bbox)
    return false;

  bb[0] = page->bbox[0];
  bb[1] = page->bbox[1];
  bb[2] = page->bbox[2];
  bb[3] = page->bbox[3];
  return true;
}

void psstream_impl::setbbox(bool f, double *bb)
{
  page->has_bbox = f;
  page->bbox[0] = bb[0];
  page->bbox[1] = bb[1];
  page->bbox[2] = bb[2];
  page->bbox[3] = bb[3];
}

void psstream_impl::setdash(int n, int* pp, int o)
{
  out << "[";
  for (int i = 0; i < n; ++i)
  {
    out << ((i+1) % 8 == 0? '\n': ' ') << pp[i];
  }
  out << " ] " << o << " setdash\n";
}

/////////////////////////////////////////////////////////////////////

psstream::psstream(const char* file)
{
  impl = new psstream_impl(file);
}

psstream::~psstream()
{
  delete impl;
}

bool psstream::setfont(const char* name, double size)
{
  return impl->setfont(name, size);
}

void psstream::textbox(const char* text, double *bbox)
{
  impl->textbox(text, bbox);
}

void psstream::newpage()
{
  impl->newpage();
}

void psstream::moveto(double x, double y)
{
	assert(!_isnan(x) && !_isnan(y));
  impl->moveto(x, y);
}

void psstream::lineto(double x, double y)
{
	assert(!_isnan(x) && !_isnan(y));
  impl->lineto(x, y);
}

void psstream::curveto(double x1, double y1, double x2, double y2, double x3, double y3)
{
  impl->curveto(x1, y1, x2, y2, x3, y3);
}

void psstream::rlineto(double x, double y)
{
  impl->rlineto(x, y);
}

void psstream::show(const char* text)
{
  impl->show(text);
}

void psstream::setgray(double g)
{
  impl->setgray(g);
}

void psstream::setrgbcolor(double r, double g, double b)
{
  impl->setrgbcolor(r, g, b);
}

void psstream::setcmykcolor(double c, double m, double y, double k)
{
	impl->setcmykcolor(c, m, y, k);
}

void psstream::translate(double x, double y)
{
  impl->translate(x, y);
}

void psstream::rotate(double a)
{
  impl->rotate(a);
}

void psstream::newpath()
{
  impl->command("newpath");
}

void psstream::closepath()
{
  impl->command("closepath");
}

void psstream::fill()
{
  impl->command("fill");
}

void psstream::stroke()
{
  impl->command("stroke");
}

void psstream::arc(double x, double y, double r, double a1, double a2)
{
  impl->arc(x, y, r, a1, a2);
}

void psstream::arcn(double x, double y, double r, double a1, double a2)
{
  impl->arcn(x, y, r, a1, a2);
}

void psstream::setlinewidth(double w)
{
  impl->setlinewidth(w);
}

void psstream::annot(double x, double y, double r, double angle, \
  annot_type anchor, bool show, const char* text, double bbox[8])
{
  impl->textbox(text, bbox);
  angle = modulo_range(angle, 360.0);
  double dx = bbox[2] - bbox[0];
  double dy = bbox[3] - bbox[1];
  double sx, sy = -0.5 * dy;

  if (anchor == ANNOT_RIGHT)
  {
    sx = r;
  }
  else if (anchor == ANNOT_LEFT)
  {
    sx = -dx-r;
  }
  else
  {
    sx = -0.5*dx;
  }

  if (show)
  {
    double bb[4];
    bool is_valid = impl->getbbox(bb);
    impl->command("matrix currentmatrix");
    impl->translate(x, y);
    impl->rotate(angle);
    impl->moveto(sx-bbox[0], sy-bbox[1]);
    impl->show(text);
    impl->command("setmatrix");
    impl->setbbox(is_valid, bb);
  }

  bbox[0] = bbox[6] = sx;
  bbox[1] = bbox[3] = sy;
  bbox[2] = bbox[4] = sx+dx;
  bbox[5] = bbox[7] = sy+dy;

  angle = 2*M_PI*angle/360.0;
  dx = cos(angle);
  dy = sin(angle);
  sx = bbox[0];
  sy = bbox[1];
  bbox[0] = x + sx * dx - sy * dy;
  bbox[1] = y + sx * dy + sy * dx; 
  sx = bbox[2];
  sy = bbox[3];
  bbox[2] = x + sx * dx - sy * dy;
  bbox[3] = y + sx * dy + sy * dx; 
  sx = bbox[4];
  sy = bbox[5];
  bbox[4] = x + sx * dx - sy * dy;
  bbox[5] = y + sx * dy + sy * dx; 
  sx = bbox[6];
  sy = bbox[7];
  bbox[6] = x + sx * dx - sy * dy;
  bbox[7] = y + sx * dy + sy * dx; 

  if (show)
  {
    impl->updatebb(bbox[0], bbox[0], bbox[1], bbox[1]);
    impl->updatebb(bbox[2], bbox[2], bbox[3], bbox[3]);
    impl->updatebb(bbox[4], bbox[4], bbox[5], bbox[5]);
    impl->updatebb(bbox[6], bbox[6], bbox[7], bbox[7]);
  }
}

void textsite(double x, double y, double r, double d, double bb[4])
{
  double dx = bb[2] - bb[0];
  double dy = bb[3] - bb[1];
  double x0, y0, ds = M_PI * r;
  d = modulo_range(d, 2.0) * (dx + dy + ds);
  if (d < 0.5*dy)
  {
    x0 = r;
    y0 = -0.5*dy + d;
  }
  else if (d < 0.5*dy + 0.5*ds)
  {
    double a = (d - 0.5*dy) / r;
    x0 = r * cos(a);
    y0 = r * sin(a);
  }
  else if (d < 0.5*dy + 0.5*ds + dx)
  {
    x0 = 0.5*dy + 0.5*ds - d;
    y0 = r;
  }
  else if (d < 0.5*dy + ds + dx)
  {
    double a = (d - 0.5*dy - dx) / r;
    x0 = r * cos(a) - dx;
    y0 = r * sin(a);
  }
  else if (d < 1.5*dy + ds + dx)
  {
    x0 = -r - dx;
    y0 = 0.5*dy + ds + dx - d;
  }
  else if (d < 1.5*dy + 1.5*ds + dx)
  {
    double a = (d - 1.5*dy - dx) / r;
    x0 = r * cos(a) - dx;
    y0 = r * sin(a) - dy;
  }
  else if (d < 1.5*dy + 1.5*ds + 2.0*dx)
  {
    x0 = -dx + (d - 1.5*dy - 1.5*ds - dx);
    y0 = -r - dy;
  }
  else if (d < 1.5*dy + 2.0*ds + 2.0*dx)
  {
    double a = (d - 1.5*dy - 2.0*dx) / r;
    x0 = r * cos(a);
    y0 = r * sin(a) - dy;
  }
  else
  {
    x0 = r;
    y0 = -dy + (d - 1.5*dy - 2.0*ds - 2.0*dx);
  }
  bb[0] = x+x0;
  bb[1] = y+y0;
  bb[2] = x+x0+dx;
  bb[3] = y+y0+dy;
}

void label(psstream& out, double x, double y, double r, \
  label_type justify, bool show, const char* text, double bb[4])
{
  out.textbox(text, bb);
  double dx = bb[2] - bb[0];
  double dy = bb[3] - bb[1];
  double ss = 2.0 * M_PI * r;
  double dd = 2.0 * (dx + dy) + ss;
  double x0 = bb[0];
  double y0 = bb[1];
  switch (justify)
  {
  case psstream::LABEL_E:
    textsite(x, y, r, 0.0, bb);
    break;
  case psstream::LABEL_N:
    textsite(x, y, r, 0.5, bb);
    break;
  case psstream::LABEL_W:
    textsite(x, y, r, 1.0, bb);
    break;
  case psstream::LABEL_S:
    textsite(x, y, r, 1.5, bb);
    break;
  case psstream::LABEL_NE:
    textsite(x, y, r, 2.0 * (0.5 * dy + 0.125 * ss) / dd, bb);
    break;
  case psstream::LABEL_NW:
    textsite(x, y, r, 2.0 * (0.5 * dy + dx + 0.375 * ss) / dd, bb);
    break;
  case psstream::LABEL_SW:
    textsite(x, y, r, 2.0 * (1.5 * dy + dx + 0.625 * ss) / dd, bb);
    break;
  case psstream::LABEL_SE:
    textsite(x, y, r, 2.0 * (1.5 * dy + 2.0 * dx + 0.875 * ss) / dd, bb);
    break;
  }
  if (show)
  {
    out.moveto(bb[0]-x0, bb[1]-y0);
    out.show(text);
  }
}

void hlabel(psstream& out, double x, double y, double r, label_type justify,
	bool show, const char* text, double bbox[4])
{
  if (justify == psstream::LABEL_N || justify == psstream::LABEL_S)
  {
  	label(out, x, y, r, justify, show, text, bbox);
  }
  else if (justify == psstream::LABEL_E || justify == psstream::LABEL_NE)
  {
  	double rect[8];
  	out.annot(x, y, r, 270.0, psstream::ANNOT_LEFT, show, text, rect);
  	bbox[0] = rect[2];
  	bbox[1] = rect[3];
  	bbox[2] = rect[6];
  	bbox[3] = rect[7];
  }
  else if (justify == psstream::LABEL_W || justify == psstream::LABEL_NW)
  {
  	double rect[8];
  	out.annot(x, y, r, 90.0, psstream::ANNOT_RIGHT, show, text, rect);
  	bbox[0] = rect[6];
  	bbox[1] = rect[7];
  	bbox[2] = rect[2];
  	bbox[3] = rect[3];
  }
  else if (justify == psstream::LABEL_SE)
  {
  	double rect[8];
  	out.annot(x, y, r, 270.0, psstream::ANNOT_RIGHT, show, text, rect);
  	bbox[0] = rect[2];
  	bbox[1] = rect[3];
  	bbox[2] = rect[6];
  	bbox[3] = rect[7];
  }
  else if (justify == psstream::LABEL_SW)
  {
  	double rect[8];
  	out.annot(x, y, r, 90.0, psstream::ANNOT_LEFT, show, text, rect);
  	bbox[0] = rect[6];
  	bbox[1] = rect[7];
  	bbox[2] = rect[2];
  	bbox[3] = rect[3];
  }
}

void vlabel(psstream& out, double x, double y, double r, label_type justify, \
	bool show, const char* text, double bbox[4])
{
	if (justify == psstream::LABEL_E || justify == psstream::LABEL_W)
	{
		label(out, x, y, r, justify, show, text, bbox);
	}
	else if (justify == psstream::LABEL_S \
		|| justify == psstream::LABEL_SW || justify == psstream::LABEL_SE)
	{
	  double rect[8];
		out.annot(x, y, 0.0, 90.0, psstream::ANNOT_CENTER, false, text, rect);
		out.annot(x + (justify != psstream::LABEL_SE? -0.5:0.5) * (rect[2]-rect[6]+2.0*r), \
			y, 0.0, 90.0, psstream::ANNOT_CENTER, show, text, rect);
		bbox[0] = rect[6];
		bbox[1] = rect[7];
		bbox[2] = rect[2];
		bbox[3] = rect[3];
	}
	else if (justify == psstream::LABEL_N \
		|| justify == psstream::LABEL_NW || justify == psstream::LABEL_NE)
	{
		double rect[8];
		out.annot(x, y, 0.0, 270.0, psstream::ANNOT_CENTER, false, text, rect);
		out.annot(x + (justify != psstream::LABEL_NE? -0.5:0.5) * (rect[6]-rect[2]+2.0*r), \
			y, 0.0, 270.0, psstream::ANNOT_CENTER, show, text, rect);
		bbox[0] = rect[2];
		bbox[1] = rect[3];
		bbox[2] = rect[6];
		bbox[3] = rect[7];
	}
}

void psstream::selectmedia(Format f, Orientation o)
{
  impl->selectmedia(f, o);
}

double psstream::pagewidth() const
{
  return impl->pagewidth();
}

double psstream::pageheight() const
{
  return impl->pageheight();
}

void psstream::rectstroke(double x, double y, double w, double h)
{
  impl->rectstroke(x, y, w, h);
}

void psstream::rectfill(double x, double y, double w, double h)
{
  impl->rectfill(x, y, w, h);
}

void psstream::gsave()
{
  impl->command("gsave");
}

void psstream::grestore()
{
  impl->command("grestore");
}

void psstream::clip()
{
  impl->command("clip");
}

void psstream::setdash(int n, int* pp, int o)
{
  impl->setdash(n, pp, o);
}

#if 0

void driver_1(psstream& out, const char* text)
{
  double bb[4];
  for (int i = 0; i < 200; ++i)
  {
    out.newpage();
    textsite(out, 200, 200, 10, 2.0*i/100.0, false, text, bb);
    
    out.setgray(0.5);
    out.moveto(bb[0], bb[1]);
    out.lineto(bb[2], bb[1]);
    out.lineto(bb[2], bb[3]);
    out.lineto(bb[0], bb[3]);
    out.lineto(bb[0], bb[1]);
    out.fill();

    out.setgray(0);
    out.arc(200, 200, 10, 0, 360);
    out.stroke();
    
    out.setgray(1);
    textsite(out, 200, 200, 10, 2.0*i/100.0, true, text, bb);
  }
}

void main()
{
  double bb[4];
  psstream out("zz.ps");
  out.selectmedia(psstream::FORMAT_A4, psstream::LANDSCAPE);
  out.setfont(FONT_22, 10.0);

  const char* text = "Проверка";

  out.newpage();
  out.setlinewidth(0.0);
  out.newpath();
  out.arc(200, 200, 5.0, 0.0, 360.0);
  out.stroke();
  label(out, 200, 200, 5.0, LABEL_E, true, text, bb);
  out.newpath();
  out.moveto(bb[0], bb[1]);
  out.lineto(bb[2], bb[1]);
  out.lineto(bb[2], bb[3]);
  out.lineto(bb[0], bb[3]);
  out.lineto(bb[0], bb[1]);
  out.stroke();

  label(out, 200, 200, 5.0, LABEL_N, true, text, bb);
  out.newpath();
  out.moveto(bb[0], bb[1]);
  out.lineto(bb[2], bb[1]);
  out.lineto(bb[2], bb[3]);
  out.lineto(bb[0], bb[3]);
  out.lineto(bb[0], bb[1]);
  out.stroke();

  label(out, 200, 200, 5.0, LABEL_W, true, text, bb);
  out.newpath();
  out.moveto(bb[0], bb[1]);
  out.lineto(bb[2], bb[1]);
  out.lineto(bb[2], bb[3]);
  out.lineto(bb[0], bb[3]);
  out.lineto(bb[0], bb[1]);
  out.stroke();

  label(out, 200, 200, 5.0, LABEL_S, true, text, bb);
  out.newpath();
  out.moveto(bb[0], bb[1]);
  out.lineto(bb[2], bb[1]);
  out.lineto(bb[2], bb[3]);
  out.lineto(bb[0], bb[3]);
  out.lineto(bb[0], bb[1]);
  out.stroke();

  out.newpage();
  out.setlinewidth(0.0);
  out.newpath();
  out.arc(200, 200, 5.0, 0.0, 360.0);
  out.stroke();
  label(out, 200, 200, 5.0, LABEL_NE, true, text, bb);
  out.newpath();
  out.moveto(bb[0], bb[1]);
  out.lineto(bb[2], bb[1]);
  out.lineto(bb[2], bb[3]);
  out.lineto(bb[0], bb[3]);
  out.lineto(bb[0], bb[1]);
  out.stroke();

  label(out, 200, 200, 5.0, LABEL_NW, true, text, bb);
  out.newpath();
  out.moveto(bb[0], bb[1]);
  out.lineto(bb[2], bb[1]);
  out.lineto(bb[2], bb[3]);
  out.lineto(bb[0], bb[3]);
  out.lineto(bb[0], bb[1]);
  out.stroke();

  label(out, 200, 200, 5.0, LABEL_SW, true, text, bb);
  out.newpath();
  out.moveto(bb[0], bb[1]);
  out.lineto(bb[2], bb[1]);
  out.lineto(bb[2], bb[3]);
  out.lineto(bb[0], bb[3]);
  out.lineto(bb[0], bb[1]);
  out.stroke();

  label(out, 200, 200, 5.0, LABEL_SE, true, text, bb);
  out.newpath();
  out.moveto(bb[0], bb[1]);
  out.lineto(bb[2], bb[1]);
  out.lineto(bb[2], bb[3]);
  out.lineto(bb[0], bb[3]);
  out.lineto(bb[0], bb[1]);
  out.stroke();

}

#endif