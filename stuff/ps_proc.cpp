#include <fstream>
#include <algorithm>
#include <math.h>
#include "ps_data.h"
#include "ps_priv.h"

inline void psstream_impl::beginproc(const char* name)
{
	std::ofstream app(setup_section, std::ios::app);
	app << '/' << name << " {\n";
	is_procedure = true;
}

inline void psstream_impl::invokeproc(const char* name)
{
	out << name << '\n';
}

inline void psstream_impl::endproc()
{
	std::ofstream app(setup_section, std::ios::app);
	app << "} bind def\n";
	is_procedure = false;
}

inline void psstream_impl::outlineshow(const char* text)
{
  if (!has_outlineshow && !is_procedure)
  {
    std::ofstream app(setup_section, std::ios::app);
    app << "/ws (X) def /spreadproc { ws false charpath gsave 1 setlinecap 1\n"
      "setlinejoin 2 setlinewidth 1 setgray stroke grestore currentpoint\n"
      "newpath exch 2 add exch moveto } def /letterproc { ws false charpath\n"
      "gsave 0 setgray fill grestore gsave 0.25 setlinewidth stroke grestore\n"
      "currentpoint newpath exch 2 add exch moveto } def /outlineshow { dup\n"
      "gsave { ws exch 0 exch put spreadproc } forall grestore gsave { ws exch\n"
      "0 exch put letterproc } forall grestore } def\n";
    has_outlineshow = true;
  }
  if (!is_procedure)
  {
    out << '(' << text << ") outlineshow\n";
  }
  else
  {
    std::ofstream app(setup_section, std::ios::app);
    app << '(' << text << ") outlineshow\n";
  }
}

inline void psstream_impl::lerpproc()
{
  if (!has_lerpproc && !is_procedure)
  {
    std::ofstream app(setup_section, std::ios::app);

    app << "/c2_r 0.4 def /c2_g 0.9 def /c2_b 0.4 def /c1_r 0.0 def /c1_g 0.5 def\n"
	    "/c1_b 0.0 def /min_collector 0.001 def\n"
	    "/max_collector 10.0 def /s_tolerance 3.0 def /max { dup 2 index gt\n"
	    "{ exch } if pop } bind def /min { dup 2 index lt { exch } if pop } bind def\n"
	    "/hypot { dup mul exch dup mul add sqrt } bind def /tricolor [ 0.5 0.25 0.0\n"
	    "0.4 0.3 0.7 0.0 0.0 1.0 1.0 1.0 0.0 ] def /x_oil 100.0 def /y_oil 100.0 def\n"
	    "/x_gas 250.0 def /y_gas 400.0 def /x_wat 400.0 def /y_wat 100.0 def\n"
	    "/trilinear_to_cartesian { 19 dict begin /t3 exch def /t2 exch def /t1 exch\n"
	    "def /y3 exch def /x3 exch def /y2 exch def /x2 exch def /y1 exch def /x1 exch\n"
	    "def /c1 x1 x2 sub def /c2 y1 y2 sub def /c c1 c2 hypot def /a1 x2 x3 sub def\n"
    	"/a2 y2 y3 sub def /a a1 a2 hypot def /f x3 x1 sub def /g y3 y1 sub def /k c1\n"
    	"g mul c2 f mul sub a t1 mul f g hypot t2 mul add c t3 mul add div def /a1 a1\n"
    	"a div def /a2 a2 a div def /c1 c1 c div def /c2 c2 c div def /s k t1 mul k t3\n"
    	"mul a2 c2 mul a1 c1 mul add mul sub a2 x1 x2 sub mul add a1 y2 y1 sub mul add\n"
    	"c2 a1 mul c1 a2 mul sub div def x1 s c1 mul add k t3 mul c2 mul sub y1 s c2\n"
    	"mul add k t3 mul c1 mul add end } bind def /cartesian_to_trilinear { 15 dict\n"
    	"begin /x exch def /y exch def /y3 exch def /x3 exch def /y2 exch def /x2 exch\n"
    	"def /y1 exch def /x1 exch def /a x3 x2 sub y3 y2 sub hypot def /b x1 x3 sub y1\n"
    	"y3 sub hypot def /c x2 x1 sub y2 y1 sub hypot def /t1 x x2 sub y3 y2 sub mul y\n"
    	"y2 sub x3 x2 sub mul sub a div def /t2 x x3 sub y1 y3 sub mul y y3 sub x1 x3\n"
    	"sub mul sub b div def /t3 x x1 sub y2 y1 sub mul y y1 sub x2 x1 sub mul sub c\n"
    	"div def /tt x1 x2 sub y3 y2 sub mul y1 y2 sub x3 x2 sub mul sub a div def t1 tt\n"
    	"div t2 tt div t3 tt div end } bind def /cartesian_to_index { 30 dict begin /s\n"
    	"exch def /y exch def /x exch def /y3 exch def /x3 exch def /y2 exch def /x2 exch\n"
    	"def /y1 exch def /x1 exch def /e11 x2 x1 sub def /e12 y2 y1 sub def /e1 e11 e12\n"
    	"hypot def /e21 x3 x1 sub def /e22 y3 y1 sub def /e2 e21 e22 hypot def /e11 e11\n"
    	"e1 div def /e12 e12 e1 div def /e21 e21 e2 div def /e22 e22 e2 div def /sx s e1\n"
    	"div def /sy s e2 div def /xx x x1 sub def /yy y y1 sub def /ee e11 e22 mul e21\n"
    	"e12 mul sub def /tol 1.0e-5 def /row e22 xx mul e21 yy mul sub sx mul ee div def\n"
    	"/col e11 yy mul e12 xx mul sub sy mul ee div def tol neg row lt row 0.0 lt and\n"
    	"{ /row 0.0 def } if tol neg col lt col 0.0 lt and { /col 0.0 def } if s row le\n"
    	"row s tol add lt and { /row s 1 sub def } if s col le col s tol add lt and {\n"
    	"/col s 1 sub def } if row col add s ge { /row row floor def /col col floor def\n"
    	"} if 0.0 row gt 0.0 col gt or row col add s gt or { x1 y1 x2 y2 x3 y3 x y\n"
    	"cartesian_to_trilinear /t3 exch def /t2 exch def /t1 exch def t1 0.0 lt { /t1\n"
    	"0.0 def } if t2 0.0 lt { /t2 0.0 def } if t3 0.0 lt { /t3 0.0 def } if /tt t1\n"
    	"t2 t3 add add def /t1 t1 tt div def /t2 t2 tt div def /t3 t3 tt div def x1 y1\n"
    	"x2 y2 x3 y3 t1 t2 t3 trilinear_to_cartesian /y exch def /x exch def /xx x x1\n"
    	"sub def /yy y y1 sub def /row e22 xx mul e21 yy mul sub sx mul ee div def /col\n"
    	"e11 yy mul e12 xx mul sub sy mul ee div def /col 0.0 def /row 0.0 def } if /xx\n"
    	"col cvi def /yy row cvi def 2 yy s mul xx add mul yy yy mul sub col xx sub row\n"
    	"yy sub add cvi add end } bind def /lookupcolor { 3 dict begin /c exch def /o\n"
    	"exch def /g exch def c max_collector lt { c min_collector lt { /c min_collector\n"
    	"def } if /c c log min_collector log sub max_collector log min_collector log sub\n"
    	"div def c1_r c2_r c1_r sub c mul add c1_g c2_g c1_g sub c mul add c1_b c2_b c1_b\n"
    	"sub c mul add } { x_oil y_oil x_gas y_gas x_wat y_wat o g 1.0 o sub g sub\n"
    	"trilinear_to_cartesian /g exch def /o exch def x_oil y_oil x_gas y_gas x_wat\n"
    	"y_wat o g tricolor length 3 div sqrt cvi cartesian_to_index /c exch 3 mul def\n"
    	"tricolor c get tricolor c 1 add get tricolor c 2 add get } ifelse setrgbcolor end\n"
    	"} bind def /triarea { 4 index sub 6 1 roll 4 index sub 6 1 roll 2 index sub 6 1\n"
    	"roll 2 index sub 6 1 roll pop pop 4 -1 roll mul 3 1 roll mul sub } bind def\n"
    	"/trifill { 25 dict begin /w3 exch def /o3 exch def /g3 exch def /y3 exch def /x3\n"
    	"exch def /w2 exch def /o2 exch def /g2 exch def /y2 exch def /x2 exch def /w1 exch\n"
    	"def /o1 exch def /g1 exch def /y1 exch def /x1 exch def /sx x2 x1 sub def /sy y2 y1\n"
    	"sub def /tx x3 x1 sub def /ty y3 y1 sub def /ss x1 y1 x2 y2 x3 y3 triarea def /n ss\n"
    	"abs sqrt s_tolerance div ceiling cvi def 0 1 n 1 sub { /i exch def 0 1 n i sub 1\n"
    	"sub { /j exch def /x0 x1 sx i 0.25 add mul tx j 0.25 add mul add n div add def /y0\n"
    	"y1 sy i 0.25 add mul ty j 0.25 add mul add n div add def g1 x0 y0 x2 y2 x3 y3\n"
    	"triarea mul g2 x0 y0 x3 y3 x1 y1 triarea mul add g3 x0 y0 x1 y1 x2 y2 triarea mul\n"
    	"add ss div o1 x0 y0 x2 y2 x3 y3 triarea mul o2 x0 y0 x3 y3 x1 y1 triarea mul add\n"
    	"o3 x0 y0 x1 y1 x2 y2 triarea mul add ss div w1 x0 y0 x2 y2 x3 y3 triarea mul w2 x0\n"
    	"y0 x3 y3 x1 y1 triarea mul add w3 x0 y0 x1 y1 x2 y2 triarea mul add ss div\n"
    	"lookupcolor x1 sx i mul tx j mul add n div add y1 sy i mul ty j mul add n div add\n"
    	"x1 sx i mul tx j 1 add mul add n div add y1 sy i mul ty j 1 add mul add n div add\n"
    	"x1 sx i 1 add mul tx j mul add n div add y1 sy i 1 add mul ty j mul add n div add\n"
    	"newpath 5 index 5 index moveto lineto lineto lineto fill } for } for 1 1 n { /i\n"
    	"exch def 1 1 n i sub { /j exch def /x0 x1 sx i 0.25 sub mul tx j 0.25 sub mul add\n"
    	"n div add def /y0 y1 sy i 0.25 sub mul ty j 0.25 sub mul add n div add def g1 x0\n"
    	"y0 x2 y2 x3 y3 triarea mul g2 x0 y0 x3 y3 x1 y1 triarea mul add g3 x0 y0 x1 y1 x2\n"
    	"y2 triarea mul add ss div o1 x0 y0 x2 y2 x3 y3 triarea mul o2 x0 y0 x3 y3 x1 y1\n"
    	"triarea mul add o3 x0 y0 x1 y1 x2 y2 triarea mul add ss div w1 x0 y0 x2 y2 x3 y3\n"
    	"triarea mul w2 x0 y0 x3 y3 x1 y1 triarea mul add w3 x0 y0 x1 y1 x2 y2 triarea mul\n"
    	"add ss div lookupcolor x1 sx i mul tx j 1 sub mul add n div add y1 sy i mul ty\n"
    	"j 1 sub mul add n div add x1 sx i 1 sub mul tx j mul add n div add y1 sy i 1 sub\n"
    	"mul ty j mul add n div add x1 sx i mul tx j mul add n div add y1 sy i mul ty j\n"
    	"mul add n div add newpath 5 index 5 index moveto lineto lineto lineto fill } for\n"
    	"} for end } bind def /quadfill { 34 dict begin /w4 exch def /o4 exch def /g4\n"
    	"exch def /y4 exch def /x4 exch def /w3 exch def /o3 exch def /g3 exch def /y3\n"
    	"exch def /x3 exch def /w2 exch def /o2 exch def /g2 exch def /y2 exch def /x2\n"
    	"exch def /w1 exch def /o1 exch def /g1 exch def /y1 exch def /x1 exch def /m\n"
    	"x4 x1 sub y4 y1 sub hypot x3 x2 sub y3 y2 sub hypot max s_tolerance div ceiling\n"
    	"cvi def /n x2 x1 sub y2 y1 sub hypot x3 x4 sub y3 y4 sub hypot max s_tolerance\n"
    	"div ceiling cvi def 1 1 m { /j exch def /v j 0.5 sub m div def /p1 x1 x4 x1 sub\n"
    	"j 1 sub mul m div add def /q1 y1 y4 y1 sub j 1 sub mul m div add def /p2 x2 x3 x2\n"
    	"sub j 1 sub mul m div add def /q2 y2 y3 y2 sub j 1 sub mul m div add def /p3 x2 x3\n"
    	"x2 sub j mul m div add def /q3 y2 y3 y2 sub j mul m div add def /p4 x1 x4 x1 sub j\n"
    	"mul m div add def /q4 y1 y4 y1 sub j mul m div add def 1 1 n { /i exch def /u i 0.5\n"
    	"sub n div def newpath p1 p2 p1 sub i 1 sub mul n div add q1 q2 q1 sub i 1 sub mul\n"
    	"n div add moveto p1 p2 p1 sub i mul n div add q1 q2 q1 sub i mul n div add lineto\n"
    	"p4 p3 p4 sub i mul n div add q4 q3 q4 sub i mul n div add lineto p4 p3 p4 sub i 1\n"
    	"sub mul n div add q4 q3 q4 sub i 1 sub mul n div add lineto closepath g1 1 u sub\n"
    	"mul 1 v sub mul g2 u mul 1 v sub mul add g3 u mul v mul add g4 1 u sub mul v mul\n"
    	"add o1 1 u sub mul 1 v sub mul o2 u mul 1 v sub mul add o3 u mul v mul add o4 1 u\n"
    	"sub mul v mul add w1 1 u sub mul 1 v sub mul w2 u mul 1 v sub mul add w3 u mul v\n"
    	"mul add w4 1 u sub mul v mul add lookupcolor fill } for } for end } bind def\n"
    	"/pixelplot { 7 dict begin /ww exch def /oo exch def /gg exch def /yy exch def\n"
    	"/xx exch def /cy exch def /cx exch def 0 cy 1 sub { dup cx 1 sub { dup 1 add\n"
    	"dup cx add dup 1 sub xx 4 index get yy 5 index get gg 6 index get oo 7 index\n"
    	"get ww 8 index get xx 8 index get yy 9 index get gg 10 index get oo 11 index get\n"
    	"ww 12 index get xx 12 index get yy 13 index get gg 14 index get oo 15 index get\n"
    	"ww 16 index get xx 16 index get yy 17 index get gg 18 index get oo 19 index get\n"
    	"ww 20 index get quadfill pop pop pop 1 add } repeat pop cx add } repeat pop end\n"
    	"} bind def\n";


    has_lerpproc = true;
  }
}

inline void psstream_impl::textproc()
{
  if (!has_textproc && !is_procedure)
  {
    std::ofstream app(setup_section, std::ios::app);
    app << "/widen_path 1 def /pi 3.1415926535897932384626433832795 def /tsize\n"
      "{ gsave 0 0 moveto true charpath flattenpath pathbbox grestore /y2 exch\n"
      "widen_path add def /x2 exch widen_path add def /y1 exch widen_path sub def\n"
      "/x1 exch widen_path sub def matrix currentmatrix } bind def /tplot { { x1\n"
      "neg y1 neg moveto show } { pop 0 0 moveto x2 x1 sub 0 rlineto 0 y2 y1 sub\n"
      "rlineto x1 x2 sub 0 rlineto 0 y1 y2 sub rlineto } ifelse setmatrix end }\n"
      "bind def /ltext { 10 dict begin /f exch def /s exch def /a exch def /r exch\n"
      "def /y exch def /x exch def s tsize x y translate a rotate r y2 y1 sub -2\n"
      "div translate s f tplot } bind def /rtext { 10 dict begin /f exch def /s\n"
      "exch def /a exch def /r exch def /y exch def /x exch def s tsize x y\n"
      "translate a rotate x1 x2 sub r sub y2 y1 sub -2 div translate s f tplot\n"
      "} bind def /annot { 9 dict begin /f exch def /s exch def /a exch def /y\n"
      "exch def /x exch def s tsize x y translate a rotate x2 x1 sub -2 div y2\n"
      "y1 sub -2 div translate s f tplot } bind def /label { 17 dict begin /f\n"
      "exch def /s exch def /d exch def /r exch def /y exch def /x exch def s\n"
      "tsize /dy y2 y1 sub def /dx x2 x1 sub def /ss 0.5 pi mul r mul def /dd\n"
      "2 ss mul dx add dy add 2 mul def /d dx dy add pi r mul add d mul def d 0\n"
      "lt { /d d dup neg dd div floor cvi 1 add dd mul add def } if d dd ge { /d\n"
      "d dup dd div floor cvi dd mul sub def } if /d1 0.0 def /d2 0.5 dy mul def\n"
      "d d2 lt { x r add y d add d2 sub } { /d1 d2 def /d2 d2 ss add def d d2 lt\n"
      "{ /a d d1 sub 90 mul ss div def x r a cos mul add y r a sin mul add } {\n"
      "/d1 d2 def /d2 d2 dx add def d d2 lt { x d d1 sub sub y r add } { /d1 d2\n"
      "def /d2 d2 ss add def d d2 lt { /a d d1 sub ss add 90 mul ss div def x dx\n"
      "sub r a cos mul add y r a sin mul add } { /d1 d2 def /d2 d2 dy add def d d2\n"
      "lt { x dx sub r sub y d d1 sub sub } { /d1 d2 def /d2 d2 ss add def d d2 lt\n"
      "{ /a d d1 sub 2 ss mul add 90 mul ss div def x r a cos mul add dx sub y r a\n"
      "sin mul add dy sub } { /d1 d2 def /d2 d2 dx add def d d2 lt { x dx sub d d1\n"
      "sub add y dy sub r sub } { /d1 d2 def /d2 d2 ss add def d d2 lt { /a d d1 sub\n"
      "3 ss mul add 90 mul ss div def x r a cos mul add y r a sin mul add dy sub }\n"
      "{ x r add y d d2 sub add dy sub } ifelse } ifelse } ifelse } ifelse } ifelse\n"
      "} ifelse } ifelse } ifelse translate s f tplot } bind def\n";
    has_textproc = true;
  }
}

inline void psstream_impl::initarray()
{
  if (!has_initarray && !is_procedure)
  {
    std::ofstream app(setup_section, std::ios::app);
    app << "/initarray { 0 { 1 index token not { exit } if 3 index exch 2 index exch\n"
    	"put 1 add dup 3 index length eq { exit } if } loop pop pop } bind def\n";
    has_initarray = true;
  }
}

inline void psstream_impl::label_textproc(double x, double y, double r, 
  double d, const char* s, bool f)
{
  textproc();
  if (!is_procedure)
  {
    out << x << ' ' << y << ' ' << r << ' '  << d << " (" << s << ") " \
      << (f? "true": "false") << " label\n";
  }
  else
  {
    std::ofstream app(setup_section, std::ios::app);
    app << x << ' ' << y << ' ' << r << ' '  << d << " (" << s << ") " \
      << (f? "true": "false") << " label\n";
  }
}

inline void psstream_impl::annot_textproc(double x, double y, double a, 
  const char* s, bool f)
{
  textproc();
  out << x << ' ' << y << ' ' << a << " (" << s << ") " \
    << (f? "true": "false") << " annot\n";
}

inline void psstream_impl::ltext_textproc(double x, double y, double r, \
  double a, const char* s, bool f)
{
  textproc();
  out << x << ' ' << y << ' ' << r << ' ' << a << " (" << s << ") " \
    << (f? "true": "false") << " ltext\n";
}

inline void psstream_impl::rtext_textproc(double x, double y, double r, \
  double a, const char* s, bool f)
{
  textproc();
  out << x << ' ' << y << ' ' << r << ' ' << a << " (" << s << ") " \
    << (f? "true": "false") << " rtext\n";
}

inline void psstream_impl::s_tolerance(double s)
{
  out << "/s_tolerance " << s << " def\n";
}

inline void psstream_impl::min_collector(double p, double r, double g, double b)
{
	out << "/min_collector " << p << " def /c1_r " << r << " def /c1_g " << g << " def /c1_b " << b << " def\n";
}

inline void psstream_impl::max_collector(double p, double r, double g, double b)
{
	out << "/max_collector " << p << " def /c2_r " << r << " def /c2_g " << g << " def /c2_b " << b << " def\n";
}

inline void psstream_impl::tricolor(int n, const color_type* s)
{
	initarray();
	int nn = n * n;
	out << "/tricolor " << nn * 3 << " array currentfile initarray ";
	for (int i = 0; i < nn; ++i)
	{
		out << '\n' << s[i].r << ' ' << s[i].g << ' ' << s[i].b;
	}
	out << " def\n";
}

inline void psstream_impl::trifill(double x1, double y1, double g1, double o1, double w1,
  double x2, double y2, double g2, double o2, double w2, double x3, double y3, double g3, 
  double o3, double w3)
{
  lerpproc();
  out << x1 << ' ' << y1 << ' ' << g1 << ' ' << o1 << ' ' << w1 << ' ' << x2 << ' ' << y2 << ' ' 
    << g2 << ' ' << o2 << ' ' << w2 << ' ' << x3 << ' ' << y3 << ' ' << g3 << ' ' << o3 << ' '
    << w3 << " trifill\n";
  
  updatebb(std::min(std::min(x1, x2), x3), 
    std::max(std::max(x1, x2), x3), 
    std::min(std::min(y1, y2), y3),
    std::max(std::max(y1, y2), y3));
}

inline void psstream_impl::quadfill(double x1, double y1, double g1, double o1, double w1,
  double x2, double y2, double g2, double o2, double w2, double x3, double y3, double g3, 
  double o3, double w3, double x4, double y4, double g4, double o4, double w4)
{
  lerpproc();
  out << x1 << ' ' << y1 << ' ' << g1 << ' ' << o1 << ' ' << w1 << ' ' << x2 << ' ' << y2 << ' ' 
    << g2 << ' ' << o2 << ' ' << w2 << ' ' << x3 << ' ' << y3 << ' ' << g3 << ' ' << o3 << ' '
    << w3 << ' ' << x4 << ' ' << y4 << ' ' << g4 << ' ' << o4 << ' ' << w4 << " quadfill\n";
  
  updatebb(std::min(std::min(x1, x2), std::min(x3, x4)), 
    std::max(std::max(x1, x2), std::max(x3, x4)), 
    std::min(std::min(y1, y2), std::min(y3, y4)),
    std::max(std::max(y1, y2), std::max(y3, y4)));
}

inline void psstream_impl::pixelplot(int cx, int cy, const float* xx, const float* yy,
  const float* gg, const float* oo, const float* ww)
{
	if (cx * cy > 65535)
	{
		int ny = (cy >> 1);
		int d = cx * (ny-1);
		pixelplot(cx, ny, xx, yy, gg, oo, ww);
		pixelplot(cx, cy-ny+1, xx+d, yy+d, gg+d, oo+d, ww+d);
	}
	else
	{
    lerpproc();
    initarray();
    s_tolerance(5.0);
    int i, nn = cx * cy;
    out << cx << ' ' << cy << '\n' << nn << " array currentfile initarray";
    for (i = 0; i < nn; ++i)
    {
      out << (i % 10 != 0? ' ': '\n') << xx[i];
    }
    out << '\n' << nn << " array currentfile initarray";
    for (i = 0; i < nn; ++i)
    {
      out << (i % 10 != 0? ' ': '\n') << yy[i];
    }
    out << '\n' << nn << " array currentfile initarray";
    for (i = 0; i < nn; ++i)
    {
      out << (i % 10 != 0? ' ': '\n') << gg[i];
    }
    out << '\n' << nn << " array currentfile initarray";
    for (i = 0; i < nn; ++i)
    {
      out << (i % 10 != 0? ' ': '\n') << oo[i];
    }
    out << '\n' << nn << " array currentfile initarray";
    for (i = 0; i < nn; ++i)
    {
      out << (i % 10 != 0? ' ': '\n') << ww[i];
    }
    out << "\npixelplot\n";

    updatebb(*std::min_element(xx, xx+nn), *std::max_element(xx, xx+nn),
      *std::min_element(yy, yy+nn), *std::max_element(yy, yy+nn));
	}
}

inline void psstream_impl::horizontal_scale(int start, int step, int final, double orig, 
  double range, double dx, double x0, double y0, double sy)
{
  out << start << ' ' << step << ' ' << final << " { " << orig << " sub\n"
    << range << " div " << dx << " mul " << x0 << " add " << y0 << " moveto 0\n"
    << sy << " rlineto stroke } for\n";

  updatebb(x0, x0+dx, std::min(y0, y0+sy), std::max(y0, y0+sy));
}

inline void psstream_impl::vertical_scale(int start, int step, int final, double orig, \
  double range, double dy, double x0, double y0, double sx)
{
  out << start << ' ' << step << ' ' << final << " { " << orig << " sub\n"
    << range << " div " << dy << " mul " << y0 << " add " << x0 << " exch moveto\n"
    << sx << " 0 rlineto stroke } for\n";
  updatebb(std::min(x0, x0+sx), std::max(x0, x0+sx), y0, y0+dy);
}

inline void psstream_impl::setcolorspace(double line_width, double scale_factor)
{
  pattern_line_width = line_width;
  pattern_scale_factor = scale_factor;
  out << "[/Pattern /DeviceRGB] setcolorspace\n";
}

inline void psstream_impl::setcolor(double r, double g, double b, psstream::Pattern h)
{
  static const char* pattern[psstream::NUM_PATTERNS] =
  {
    "bdiagonal",
    "crosshatch",
    "diaghatch",
    "horizontal",
    "fdiagonal",
    "vertical",
    "sandstone",
    "shale",
    "limestone",
    "dolomite",
    "silicate",
    "salt",
    "anhydrite",
    "siltstone",
    "bitumen",
    "oilzone",
    "watzone",
    "gaszone",
    "gypsum",
    "deepbase",
    "gasoilzone",
    "oilwatzone",
    "gaswatzone",
    "gasoilwatzone",
    "watgaszone",
    "watoilzone",
    "oilgaszone",
    "sand",
    "coal",
    "evaluation"
  };

  if (!has_pattern[h])
  {
    static const char* proc[] = 
    {
      // bdiagonal
      "0.0 2.0 moveto 2.0 0.0 lineto\n"
      "0.0 4.0 moveto 4.0 0.0 lineto\n"
      "2.0 4.0 moveto 6.0 0.0 lineto\n"
      "4.0 4.0 moveto 6.0 2.0 lineto\n"
      "stroke",

      // crosshatch
      "0.0 0.0 moveto 0.0 4.0 lineto\n"
      "2.0 0.0 moveto 2.0 4.0 lineto\n"
      "4.0 0.0 moveto 4.0 4.0 lineto\n"
      "0.0 0.0 moveto 6.0 0.0 lineto\n"
      "0.0 2.0 moveto 6.0 2.0 lineto\n"
      "stroke",

      // diaghatch
      "0.0 2.0 moveto 2.0 0.0 lineto\n"
      "0.0 4.0 moveto 4.0 0.0 lineto\n"
      "2.0 4.0 moveto 6.0 0.0 lineto\n"
      "4.0 4.0 moveto 6.0 2.0 lineto\n"
      "0.0 2.0 moveto 2.0 4.0 lineto\n"
      "0.0 0.0 moveto 4.0 4.0 lineto\n"
      "2.0 0.0 moveto 6.0 4.0 lineto\n"
      "4.0 0.0 moveto 6.0 2.0 lineto\n"
      "stroke",

      // horizontal
      "0.0 0.0 moveto 6.0 0.0 lineto\n"
      "0.0 2.0 moveto 6.0 2.0 lineto\n"
      "stroke",

      // fdiagonal 
      "0.0 2.0 moveto 2.0 4.0 lineto\n"
      "0.0 0.0 moveto 4.0 4.0 lineto\n"
      "2.0 0.0 moveto 6.0 4.0 lineto\n"
      "4.0 0.0 moveto 6.0 2.0 lineto\n"
      "stroke",

      // vertical
      "0.0 0.0 moveto 0.0 4.0 lineto\n"
      "2.0 0.0 moveto 2.0 4.0 lineto\n"
      "4.0 0.0 moveto 4.0 4.0 lineto\n"
      "stroke",

      // sandstone
      "0 0 moveto 6 0 rlineto\n"
      "0 2 moveto 6 0 rlineto\n"
      "1 2 moveto 1 1 1 90 270 arc\n"
      "5 0 moveto 5 1 1 270 90 arc\n"
      "2 2 moveto 2 3 1 270 90 arc\n"
      "4 4 moveto 4 3 1 90 270 arc\n"
      "stroke",

      // shale
      "0 0 moveto 6 0 rlineto\n"
      "0 2 moveto 6 0 rlineto\n"
      "stroke",

      // limestone
      "0 0 moveto 6 0 rlineto\n"
      "0 2 moveto 6 0 rlineto\n"
      "0 0 moveto 0 2 rlineto\n"
      "3 0 moveto 0 2 rlineto\n"
      "1.5 2 moveto 0 2 rlineto\n"
      "4.5 2 moveto 0 2 rlineto\n"
      "stroke",

      // dolomite
      "0 0 moveto 6 0 rlineto\n"
      "0 2 moveto 6 0 rlineto\n"
      "0.0 0 moveto 0 2 rlineto\n"
      "0.5 0 moveto 0 2 rlineto\n"
      "3.0 0 moveto 0 2 rlineto\n"
      "3.5 0 moveto 0 2 rlineto\n"
      "1.5 2 moveto 0 2 rlineto\n"
      "2.0 2 moveto 0 2 rlineto\n"
      "4.5 2 moveto 0 2 rlineto\n"
      "5.0 2 moveto 0 2 rlineto\n"
      "stroke",

      // silicate
      "0.0 0.25 moveto 0 1.5 rlineto\n"
      "0.5 0.25 moveto 0 1.5 rlineto\n"
      "3.0 0.25 moveto 0 1.5 rlineto\n"
      "3.5 0.25 moveto 0 1.5 rlineto\n"
      "1.5 2.25 moveto 0 1.5 rlineto\n"
      "2.0 2.25 moveto 0 1.5 rlineto\n"
      "4.5 2.25 moveto 0 1.5 rlineto\n"
      "5.0 2.25 moveto 0 1.5 rlineto\n"
      "stroke",

      // salt
      "0.0 0.25 moveto 0.0 1.5 rlineto\n"
      "0.0 1.0 moveto 0.75 0.0 rlineto\n"
      "5.25 1.0 moveto 0.75 0.0 rlineto\n"
      "3.0 0.25 moveto 0.0 1.5 rlineto\n"
      "2.25 1.0 moveto 1.5 0.0 rlineto\n"
      "1.5 2.25 moveto 0.0 1.5 rlineto\n"
      "0.75 3.0 moveto 1.5 0.0 rlineto\n"
      "4.5 2.25 moveto 0.0 1.5 rlineto\n"
      "3.75 3.0 moveto 1.5 0.0 rlineto\n"
      "stroke",

      // anhydrite
      "0.0 1.75 moveto 0.5 -1.5 rlineto\n"
      "5.5 0.25 moveto 0.5 1.5 rlineto\n"
      "2.5 0.25 moveto\n"
      "0.5 1.5 rlineto\n"
      "0.5 -1.5 rlineto\n"
      "1.0 2.25 moveto\n"
      "0.5 1.5 rlineto\n"
      "0.5 -1.5 rlineto\n"
      "4.0 2.25 moveto\n"
      "0.5 1.5 rlineto\n"
      "0.5 -1.5 rlineto\n"
      "stroke",

      // siltstone
      "0.75 1 moveto 1.25 1.5 1.75 0.5 2.25 1 curveto\n"
      "3.75 1 moveto 4.25 1.5 4.75 0.5 5.25 1 curveto\n"
      "0.25 1.0 moveto 0.0 1.0 0.25 0 360 arc\n"
      "3.25 1.0 moveto 3.0 1.0 0.25 0 360 arc\n"
      "6.25 1.0 moveto 6.0 1.0 0.25 0 360 arc\n"
      "-0.75 3.0 moveto -0.25 3.5 0.25 2.5 0.75 3.0 curveto\n"
      "2.25 3.0 moveto 2.75 3.5 3.25 2.5 3.75 3.0 curveto\n"
      "5.25 3.0 moveto 5.75 3.5 6.25 2.5 6.75 3.0 curveto\n"
      "1.75 3.0 moveto 1.5 3.0 0.25 0 360 arc\n"
      "4.75 3.0 moveto 4.5 3.0 0.25 0 360 arc\n"
      "stroke",

      // bitumen
      "0.0 0.5 moveto 1.5 0.5 rlineto -1.5 0.5 rlineto\n"
      "3.0 0.5 moveto 1.5 0.5 rlineto -1.5 0.5 rlineto\n"
      "1.5 2.5 moveto 1.5 0.5 rlineto -1.5 0.5 rlineto\n"
      "4.5 2.5 moveto 1.5 0.5 rlineto -1.5 0.5 rlineto\n"
      "fill",

      // oilzone
      "0.5 1.0 moveto 0.0 1.0 0.5 0 360 arc\n"
      "3.5 1.0 moveto 3.0 1.0 0.5 0 360 arc\n"
      "6.5 1.0 moveto 6.0 1.0 0.5 0 360 arc\n"
      "2.0 3.0 moveto 1.5 3.0 0.5 0 360 arc\n"
      "5.0 3.0 moveto 4.5 3.0 0.5 0 360 arc\n"
      "fill",

      // watzone
      "0.5 1.0 moveto 0.0 1.0 0.5 0 360 arc\n"
      "3.5 1.0 moveto 3.0 1.0 0.5 0 360 arc\n"
      "6.5 1.0 moveto 6.0 1.0 0.5 0 360 arc\n"
      "2.0 3.0 moveto 1.5 3.0 0.5 0 360 arc\n"
      "5.0 3.0 moveto 4.5 3.0 0.5 0 360 arc\n"
      "stroke",

      // gaszone
      "0.5 1.0 moveto 0.0 1.0 0.5 0 360 arc\n"
      "3.5 1.0 moveto 3.0 1.0 0.5 0 360 arc\n"
      "6.5 1.0 moveto 6.0 1.0 0.5 0 360 arc\n"
      "2.0 3.0 moveto 1.5 3.0 0.5 0 360 arc\n"
      "5.0 3.0 moveto 4.5 3.0 0.5 0 360 arc\n"
      "0.0 3.0 moveto 0.5 2.5 -0.5 2.0 0.0 1.5 curveto\n"
      "3.0 3.0 moveto 3.5 2.5 2.5 2.0 3.0 1.5 curveto\n"
      "6.0 3.0 moveto 6.5 2.5 5.5 2.0 6.0 1.5 curveto\n"
      "1.5 5.0 moveto 2.0 4.5 1.0 4.0 1.5 3.5 curveto\n"
      "4.5 5.0 moveto 5.0 4.5 4.0 4.0 4.5 3.5 curveto\n"
      "1.5 1.0 moveto 2.0 0.5 1.0 0.0 1.5 -0.5 curveto\n"
      "4.5 1.0 moveto 5.0 0.5 4.0 0.0 4.5 -0.5 curveto\n"
      "stroke",

      // gypsum
      "0.0 0.25 moveto 0.5 1.5 rlineto\n"
      "5.5 1.75 moveto 0.5 -1.5 rlineto\n"
      "2.5 1.75 moveto 0.5 -1.5 rlineto 0.5 1.5 rlineto\n"
      "1.0 3.75 moveto 0.5 -1.5 rlineto 0.5 1.5 rlineto\n"
      "4.0 3.75 moveto 0.5 -1.5 rlineto 0.5 1.5 rlineto\n"
      "stroke",

      // deepbase
      "0.0 -2.0 moveto 2.0 2.0 4.0 -6.0 6.0 -2.0 curveto\n"
      "0.0 0.0 moveto 2.0 4.0 4.0 -4.0 6.0 0.0 curveto\n"
      "0.0 2.0 moveto 2.0 6.0 4.0 -2.0 6.0 2.0 curveto\n"
      "0.0 4.0 moveto 2.0 8.0 4.0 -0.0 6.0 4.0 curveto\n"
      "stroke",

      // gasoilzone
      "3.5 1.0 moveto 3.0 1.0 0.5 0 360 arc fill\n"
      "0.5 1.0 moveto 0.0 1.0 0.5 0 360 arc\n"
      "6.5 1.0 moveto 6.0 1.0 0.5 0 360 arc\n"
      "2.0 3.0 moveto 1.5 3.0 0.5 0 360 arc\n"
      "5.0 3.0 moveto 4.5 3.0 0.5 0 360 arc\n"
      "0.0 3.0 moveto 0.5 2.5 -0.5 2.0 0.0 1.5 curveto\n"
      "6.0 3.0 moveto 6.5 2.5 5.5 2.0 6.0 1.5 curveto\n"
      "1.5 5.0 moveto 2.0 4.5 1.0 4.0 1.5 3.5 curveto\n"
      "4.5 5.0 moveto 5.0 4.5 4.0 4.0 4.5 3.5 curveto\n"
      "1.5 1.0 moveto 2.0 0.5 1.0 0.0 1.5 -0.5 curveto\n"
      "4.5 1.0 moveto 5.0 0.5 4.0 0.0 4.5 -0.5 curveto\n"
      "stroke",

      // oilwatzone
      "3.5 1.0 moveto 3.0 1.0 0.5 0 360 arc stroke\n"
      "0.5 1.0 moveto 0.0 1.0 0.5 0 360 arc\n"
      "6.5 1.0 moveto 6.0 1.0 0.5 0 360 arc\n"
      "2.0 3.0 moveto 1.5 3.0 0.5 0 360 arc\n"
      "5.0 3.0 moveto 4.5 3.0 0.5 0 360 arc\n"
      "fill",

      // gaswatzone
      "0.5 1.0 moveto 0.0 1.0 0.5 0 360 arc\n"
      "6.5 1.0 moveto 6.0 1.0 0.5 0 360 arc\n"
      "2.0 3.0 moveto 1.5 3.0 0.5 0 360 arc\n"
      "5.0 3.0 moveto 4.5 3.0 0.5 0 360 arc\n"
      "0.0 3.0 moveto 0.5 2.5 -0.5 2.0 0.0 1.5 curveto\n"
      "6.0 3.0 moveto 6.5 2.5 5.5 2.0 6.0 1.5 curveto\n"
      "1.5 5.0 moveto 2.0 4.5 1.0 4.0 1.5 3.5 curveto\n"
      "4.5 5.0 moveto 5.0 4.5 4.0 4.0 4.5 3.5 curveto\n"
      "1.5 1.0 moveto 2.0 0.5 1.0 0.0 1.5 -0.5 curveto\n"
      "4.5 1.0 moveto 5.0 0.5 4.0 0.0 4.5 -0.5 curveto\n"
      "3.5 1.0 moveto 3.0 1.0 0.5 0 360 arc\n"
      "stroke",

      // gasoilwatzone
      "2.5 1.0 moveto 2.0 1.0 0.5 0 360 arc\n"
      "5.5 3.0 moveto 5.0 3.0 0.5 0 360 arc fill\n"
      "0.5 1.0 moveto 0.0 1.0 0.5 0 360 arc\n"
      "6.5 1.0 moveto 6.0 1.0 0.5 0 360 arc\n"
      "3.5 3.0 moveto 3.0 3.0 0.5 0 360 arc\n"
      "0.0 3.0 moveto 0.5 2.5 -0.5 2.0 0.0 1.5 curveto\n"
      "6.0 3.0 moveto 6.5 2.5 5.5 2.0 6.0 1.5 curveto\n"
      "3.0 5.0 moveto 3.5 4.5 2.5 4.0 3.0 3.5 curveto\n"
      "3.0 1.0 moveto 3.5 0.5 2.5 0.0 3.0 -0.5 curveto\n"
      "4.5 1.0 moveto 4.0 1.0 0.5 0 360 arc\n"
      "1.5 3.0 moveto 1.0 3.0 0.5 0 360 arc\n"
      "stroke",

      // watgaszone
      "0.5 1.0 moveto 0.0 1.0 0.5 0 360 arc\n"
      "6.5 1.0 moveto 6.0 1.0 0.5 0 360 arc\n"
      "2.0 3.0 moveto 1.5 3.0 0.5 0 360 arc\n"
      "5.0 3.0 moveto 4.5 3.0 0.5 0 360 arc\n"
      "3.5 1.0 moveto 3.0 1.0 0.5 0 360 arc\n"
      "3.0 3.0 moveto 3.5 2.5 2.5 2.0 3.0 1.5 curveto\n"
      "stroke",

      // watoilzone
      "3.5 1.0 moveto 3.0 1.0 0.5 0 360 arc fill\n"
      "0.5 1.0 moveto 0.0 1.0 0.5 0 360 arc\n"
      "6.5 1.0 moveto 6.0 1.0 0.5 0 360 arc\n"
      "2.0 3.0 moveto 1.5 3.0 0.5 0 360 arc\n"
      "5.0 3.0 moveto 4.5 3.0 0.5 0 360 arc\n"
      "stroke",

      // oilgaszone
      "3.5 1.0 moveto 3.0 1.0 0.5 0 360 arc\n"
      "3.0 3.0 moveto 3.5 2.5 2.5 2.0 3.0 1.5 curveto stroke\n"
      "0.5 1.0 moveto 0.0 1.0 0.5 0 360 arc\n"
      "6.5 1.0 moveto 6.0 1.0 0.5 0 360 arc\n"
      "2.0 3.0 moveto 1.5 3.0 0.5 0 360 arc\n"
      "5.0 3.0 moveto 4.5 3.0 0.5 0 360 arc\n"
      "fill",

      // sand
      "0.436882 0.0466292 moveto 0.236882 0.0466292 0.2 0 360 arc\n"
      "0.436882 4.04663 moveto 0.236882 4.04663 0.2 0 360 arc\n"
      "0.225333 1.09774 moveto 0.0253334 1.09774 0.2 0 360 arc\n"
      "6.22533 1.09774 moveto 6.02533 1.09774 0.2 0 360 arc\n"
      "0.257469 1.94022 moveto 0.0574694 1.94022 0.2 0 360 arc\n"
      "6.25747 1.94022 moveto 6.05747 1.94022 0.2 0 360 arc\n"
      "0.0279214 2.99089 moveto -0.172079 2.99089 0.2 0 360 arc\n"
      "6.02792 2.99089 moveto 5.82792 2.99089 0.2 0 360 arc\n"
      "1.42157 0.0775933 moveto 1.22157 0.0775933 0.2 0 360 arc\n"
      "1.42157 4.07759 moveto 1.22157 4.07759 0.2 0 360 arc\n"
      "1.24515 1.09631 moveto 1.04515 1.09631 0.2 0 360 arc\n"
      "1.09948 2.26574 moveto 0.899481 2.26574 0.2 0 360 arc\n"
      "1.20232 2.89869 moveto 1.00232 2.89869 0.2 0 360 arc\n"
      "2.3545 0.262261 moveto 2.1545 0.262261 0.2 0 360 arc\n"
      "2.48394 1.17274 moveto 2.28394 1.17274 0.2 0 360 arc\n"
      "1.97846 2.04385 moveto 1.77846 2.04385 0.2 0 360 arc\n"
      "2.40321 3.14897 moveto 2.20321 3.14897 0.2 0 360 arc\n"
      "3.46913 0.249297 moveto 3.26913 0.249297 0.2 0 360 arc\n"
      "3.348 0.851195 moveto 3.148 0.851195 0.2 0 360 arc\n"
      "3.29292 1.96584 moveto 3.09292 1.96584 0.2 0 360 arc\n"
      "3.09662 3.12403 moveto 2.89662 3.12403 0.2 0 360 arc\n"
      "4.27719 0.127802 moveto 4.07719 0.127802 0.2 0 360 arc\n"
      "4.27719 4.1278 moveto 4.07719 4.1278 0.2 0 360 arc\n"
      "4.09734 0.769527 moveto 3.89734 0.769527 0.2 0 360 arc\n"
      "4.27538 2.09581 moveto 4.07538 2.09581 0.2 0 360 arc\n"
      "4.40519 2.71159 moveto 4.20519 2.71159 0.2 0 360 arc\n"
      "4.94904 -0.114106 moveto 4.74904 -0.114106 0.2 0 360 arc\n"
      "4.94904 3.88589 moveto 4.74904 3.88589 0.2 0 360 arc\n"
      "5.07004 0.806607 moveto 4.87004 0.806607 0.2 0 360 arc\n"
      "5.0104 1.81675 moveto 4.8104 1.81675 0.2 0 360 arc\n"
      "5.06967 3.01971 moveto 4.86967 3.01971 0.2 0 360 arc\n"
      "fill",

      // coal
      "0 1 2 0.5 rectfill\n"
      "3 3 2 0.5 rectfill\n",

      // evaluation
	  	"/Helvetica-Bold findfont 1.0 scalefont setfont\n"
			"0 0 moveto (evaluation) show\n"
			"3 2 moveto (evalua) show -0.46 2 moveto (ation) show\n"
    };

    std::ofstream app(setup_section, std::ios::app);
    app << pattern_line_width << " setlinewidth\n";
    app << '/' << pattern[h] << " << /PatternType 1 /PaintType 2 /TilingType 1\n"
      "/BBox [ 0 0 6 4 ] /XStep 6 /YStep 4 /PaintProc { pop\n"
      << proc[h] << "\n} bind >> " << pattern_scale_factor 
      << ' ' << pattern_scale_factor << " matrix scale\n";

    if (orientation == psstream::LANDSCAPE)
    {
      app << "90 matrix rotate matrix concatmatrix\n"; 
    }
    app << "makepattern def\n";

    has_pattern[h] = true;
  }
  out << r << ' ' << g << ' ' << b << ' ' << pattern[h] << " setcolor\n";
}

/////////////////////////////////////////////////////////////////////////

void psstream::outlineshow(const char* text)
{
  impl->outlineshow(text);
}

void psstream::label_textproc(double x, double y, double r, 
  double d, const char* s, bool f)
{
  impl->label_textproc(x, y, r, d, s, f);
}

void psstream::annot_textproc(double x, double y, double a, 
  const char* s, bool f)
{
  impl->annot_textproc(x, y, a, s, f);
}

void psstream::ltext_textproc(double x, double y, double r, \
  double a, const char* s, bool f)
{
  impl->ltext_textproc(x, y, r, a, s, f);
}

void psstream::rtext_textproc(double x, double y, double r, \
  double a, const char* s, bool f)
{
  impl->rtext_textproc(x, y, r, a, s, f);
}

void psstream::s_tolerance(double s)
{
  impl->s_tolerance(s);
}

void psstream::min_collector(double p, double r, double g, double b)
{
	impl->min_collector(p, r, g, b);
}

void psstream::max_collector(double p, double r, double g, double b)
{
	impl->max_collector(p, r, g, b);
}

void psstream::tricolor(int n, const color_type* s)
{
	impl->tricolor(n, s);
}

void psstream::trifill(double x1, double y1, double g1, double o1, double w1,
  double x2, double y2, double g2, double o2, double w2, double x3, double y3, double g3, 
  double o3, double w3)
{
  impl->trifill(x1, y1, g1, o1, w1, x2, y2, g2, o2, w2, x3, y3, g3, o3, w3);
}

void psstream::quadfill(double x1, double y1, double g1, double o1, double w1,
  double x2, double y2, double g2, double o2, double w2, double x3, double y3, double g3, 
  double o3, double w3, double x4, double y4, double g4, double o4, double w4)
{
  impl->quadfill(x1, y1, g1, o1, w1, x2, y2, g2, o2, w2, x3, y3, g3, o3, w3, x4, y4, g4, o4, w4);
}

void psstream::pixelplot(int cx, int cy, const float* xx, const float* yy,
  const float* gg, const float* oo, const float* ww)
{
  impl->pixelplot(cx, cy, xx, yy, gg, oo, ww);
}

void psstream::horizontal_scale(int start, int step, int final, double orig, 
  double range, double dx, double x0, double y0, double sy)
{
  impl->horizontal_scale(start, step, final, orig, range, dx, x0, y0, sy);
}

void psstream::horizontal(const Scale& scale)
{
	setgray(0.0);
	setlinewidth(scale.line_width);

	double margin = scale.margin;
	if (margin < 0.0)
	{
		margin = 0.0;
	}

	double axis_line;
	if (scale.region[1] < scale.region[3])
	{
		axis_line = scale.region[1] - margin;
		if (scale.flags & Scale::HAS_INNER_TICK)
		{
			axis_line -= scale.tick_length;
		}
	}
	else
	{
		axis_line = scale.region[1] + margin;
		if (scale.flags & Scale::HAS_INNER_TICK)
		{
			axis_line += scale.tick_length;
		}
	}

	if ((scale.flags & Scale::HAS_LABEL_TEXT) != 0 \
		&& scale.font_name && scale.font_name[0] != '\0' \
		&& scale.label != LABEL_NONE)
	{
		setfont(scale.font_name, scale.font_size);

		if (scale.label == LABEL_N && scale.region[1] < scale.region[3])
		{
  		int start = (int) ceil(scale.range[0]);
  		int final = (int) floor(scale.range[1]);
  		double y = axis_line + scale.tick_length;
  		for (int i = start; i <= final; ++i)
  		{
        double bbox[4];
        char text[32];
        sprintf(text, "%d", i);
        double x = scale.region[0] + (scale.region[2] - scale.region[0]) \
        	* (i - scale.range[0]) / (scale.range[1] - scale.range[0]);
        label(*this, x, y, scale.line_width, LABEL_N, true, text, bbox);
  		}
		}
		if (scale.label == LABEL_S && scale.region[1] > scale.region[3])
		{
  		int start = (int) ceil(scale.range[0]);
  		int final = (int) floor(scale.range[1]);
  		double y = axis_line - scale.tick_length;
  		for (int i = start; i <= final; ++i)
  		{
        double bbox[4];
        char text[32];
        sprintf(text, "%d", i);
        double x = scale.region[0] + (scale.region[2] - scale.region[0]) \
        	* (i - scale.range[0]) / (scale.range[1] - scale.range[0]);
        label(*this, x, y, scale.line_width, LABEL_S, true, text, bbox);
  		}
		}
		if ((scale.label == LABEL_E || scale.label == LABEL_NE) \
			&& scale.region[1] < scale.region[3] && scale.region[0] < scale.region[2]
			|| (scale.label == LABEL_W || scale.label == LABEL_NW) \
			&& scale.region[1] < scale.region[3] && scale.region[0] > scale.region[2])
		{
  		int start = (int) ceil(scale.range[0]);
  		int final = (int) floor(scale.range[1]);
  		double y = axis_line + scale.tick_length;
  		for (int i = start; i <= final; ++i)
  		{
        double bbox[8];
        char text[32];
        sprintf(text, "%d", i);
        double x = scale.region[0] + (scale.region[2] - scale.region[0]) \
        	* (i - scale.range[0]) / (scale.range[1] - scale.range[0]);
        annot(x, y, scale.line_width, 270.0, ANNOT_LEFT, true, text, bbox);
  		}
		}
		if ((scale.label == LABEL_W || scale.label == LABEL_SW) \
			&& scale.region[1] > scale.region[3] && scale.region[0] > scale.region[2])
		{
  		int start = (int) ceil(scale.range[0]);
  		int final = (int) floor(scale.range[1]);
  		double y = axis_line + scale.tick_length;
  		for (int i = start; i <= final; ++i)
  		{
        double bbox[8];
        char text[32];
        sprintf(text, "%d", i);
        double x = scale.region[0] + (scale.region[2] - scale.region[0]) \
        	* (i - scale.range[0]) / (scale.range[1] - scale.range[0]);
        annot(x, y, scale.line_width, 90.0, ANNOT_RIGHT, true, text, bbox);
  		}
		}
	}

	if (scale.flags & Scale::HAS_INNER_FRAME)
	{
		rectstroke(scale.region[0], scale.region[1], \
			scale.region[2] - scale.region[0], scale.region[3] - scale.region[1]);
	}

	if ((scale.flags & Scale::HAS_OUTER_FRAME) != 0 \
		&& (margin > 0.0 || (scale.flags & Scale::HAS_INNER_FRAME) == 0))
	{
		double region[4];
		if (scale.region[0] < scale.region[2])
		{
  		region[0] = scale.region[0] - margin;
  		region[2] = scale.region[2] + margin;
		}
		else
		{
			region[0] = scale.region[0] + margin;
			region[2] = scale.region[2] - margin;
		}
		if (scale.region[1] < scale.region[3])
		{
			region[1] = axis_line;
			region[3] = scale.region[3] + margin;
		}
		else
		{
			region[1] = axis_line;
			region[3] = scale.region[3] - margin;
		}
		rectstroke(region[0], region[1], \
			region[2] - region[0], region[3] - region[1]);
	}
	else if (scale.flags & Scale::HAS_AXIS_LINE)
	{
		moveto(scale.region[0], axis_line);
		lineto(scale.region[2], axis_line);
		stroke();
	}

	if (scale.flags & (Scale::HAS_INNER_TICK | Scale::HAS_OUTER_TICK))
	{
		if (scale.type == Scale::ARITHMETIC)
		{
	    double tick_length = scale.tick_length;
	    if (scale.region[1] > scale.region[3])
	    {
	    	tick_length = -tick_length;
	    }
			double y0 = axis_line;
		  if (scale.flags & Scale::HAS_OUTER_TICK)
		  {
		  	y0 -= tick_length;
		  	if (scale.flags & Scale::HAS_INNER_TICK)
		  	{
		  		tick_length *= 2.0;
		  	}
		  }
			int start = (int) ceil(scale.range[0]);
			int final = (int) floor(scale.range[1]);
			impl->horizontal_scale(start, 1, final, scale.range[0], \
				scale.range[1] - scale.range[0], scale.region[2] - scale.region[0], \
				scale.region[0], y0, tick_length);
		}
		else if (scale.type == Scale::LOGARITHMIC)
		{
		}
		else if (scale.type == Scale::PROBABILITY)
		{
		}
	}

	if (scale.flags & Scale::HAS_GRID_LINES)
	{
		int start = (int) ceil(scale.range[0]);
		int final = (int) floor(scale.range[1]);
		impl->horizontal_scale(start, 1, final, scale.range[0], \
			scale.range[1] - scale.range[0], scale.region[2] - scale.region[0], \
			scale.region[0], scale.region[1], scale.region[3] - scale.region[1]);		
	}

}


void psstream::vertical_scale(int start, int step, int final, double orig, 
  double range, double dy, double x0, double y0, double sx)
{
  impl->vertical_scale(start, step, final, orig, range, dy, x0, y0, sx);
}

void psstream::vertical(const Scale& scale)
{
}

void psstream::setcolorspace(double line_width, double scale_factor)
{
  impl->setcolorspace(line_width, scale_factor);
}

void psstream::setcolor(double r, double g, double b, Pattern h)
{
  impl->setcolor(r, g, b, h);
}

void psstream::beginproc(const char* name)
{
	impl->beginproc(name);
}

void psstream::invokeproc(const char* name)
{
	impl->invokeproc(name);
}

void psstream::endproc()
{
	impl->endproc();
}

