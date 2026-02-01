#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <limits>
#include <math.h>
#include <time.h>
#include <getopt.h>
#include "ps_data.h"
#include "demo.h"

void main(int argc, char** argv)
{
  int c, option_index;
  char samples[80] = "d:\\data\\samples.txt";
  char grid[80] = "d:\\data\\elev.grd";
  bool do_usage = false;
  bool do_frame = false;
  bool do_voro = false;
  bool do_hull = false;
  bool do_clip = false;
  bool do_incurve = false;
  bool do_bbox = false;
  bool do_xclip = false;
  bool do_level = false;
  bool do_trilin = false;
  bool do_bitmap = false;
  
  static struct option option_array[] =
  {
    { "help", 0, 0, '?' },
    { "samples", 1, 0, 's' },
    { "grid", 1, 0, 'g' },
    { "frame", 0, 0, 'f' },
    { "voro", 0, 0, 'v' },
    { "hull", 0, 0, 'h'},
    { "clip", 0, 0, 'c'},
    { "incurve", 0, 0, 'i' },
    { "bbox", 0, 0, 'b' },
    { "xclip", 0, 0, 'x' },
    { "level", 0, 0, 'l' },
    { "trilin", 0, 0, 't' },
    { "bitmap", 0, 0, 'm' },
    { 0, 0, 0, 0 }
  };
  const char* option_string = "?s:g:fvhcibxltm";
  while (!do_usage)
  {
    c = getopt_long(argc, argv, option_string, option_array, &option_index);
    if (c == -1)
    {
      break;
    }
    switch (c)
    {
    case '?':
      do_usage = true;
      break;
    case 's':
      if (1 != sscanf(optarg, "%s", samples))
      {
        do_usage = true;
      }
      break;
    case 'g':
      if (1 != sscanf(optarg, "%s", grid))
      {
        do_usage = true;
      }
      break;
    case 'f':
      do_frame = true;
      break;
    case 'v':
      do_voro = true;
      break;
    case 'c':
      do_clip = true;
      break;
    case 'h':
      do_hull = true;
      break;
    case 'i':
      do_incurve = true;
      break;
    case 'b':
      do_bbox = true;
      break;
    case 'x':
      do_xclip = true;
      break;
    case 'l':
      do_level = true;
      break;
    case 't':
    	do_trilin = true;
    	break;
    case 'm':
    	do_bitmap = true;
    	break;
    }
  }

  if (!do_frame && !do_voro && !do_hull && !do_clip && !do_incurve && !do_bbox \
  	&& !do_xclip && !do_level && !do_trilin && !do_bitmap)
  {
    do_usage = true;
  }

  if (do_usage)
  {
    std::clog << "\npostscript library test driver\n\n"
      "usage: " << argv[0] << " [-?] [-g grid] [-s samp] [<options>] [plot]\n\n"
      "\t--help, -?:             this help screen\n"
      "\t--grid, -g:             define name of grid file\n"
      "\t--samples, -s:          define name of xyz file\n"
      "\t--frame, -f:            plot frame\n"
      "\t--voro, -v:             plot voronoi diagram\n"
      "\t--hull, -h:             convex hull check\n"
      "\t--incurve, -i:          check for insideness procedure\n"
      "\t--clip, -c:             check clip procedure\n"
      "\t--bbox, -b:             test for polygon simplisity\n"
      "\t--xclip, -x:            check sutherlan-hodgman clipping\n"
      "\t--level, -l:            auto_conrec test program\n"
      "\t--trilin, -t:           test trilin procedure\n"
      "\t--bitmap, m:            test bitmap output\n";

    return;
  }
  
  psstream out((argc > optind)? argv[optind]: "con:");
  out.selectmedia(psstream::FORMAT_A4, psstream::PORTRAIT);
  if (do_frame)
  {
    check_frame(out);
  }
  if (do_voro)
  {
    check_voro(out, samples);
  }
  if (do_hull)
  {
    check_hull();
  }
  if (do_clip)
  {
    check_clip(out, grid);
  }
  if (do_incurve)
  {
    check_incurve();
  }
  if (do_bbox)
  {
    check_bbox();
  }
  if (do_xclip)
  {
    check_clip_convex();
  }
  if (do_level)
  {
    check_conrec(out, grid);
  }
  if (do_trilin)
  {
  	check_trilin(out);
  }
  if (do_bitmap)
  {
  	check_bitmap(out, grid);
  }
}

