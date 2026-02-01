#include <windows.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <zlib.h>

// #include "ps_data.h"

bool mkfont(std::ostream& res, std::ostream& hdr, const char* base)
{
  static int font_num = 0;
  const int c0 = 32, n = 255+1-c0;
  short bb[5*n];
  std::fill_n(bb, 4*n, (short) 0);
  
  char font[80], buff[512];
  strcat(strcpy(buff, base), ".afm");

  std::ifstream afm(buff);
  std::string rec;
  while (std::getline(afm, rec))
  {
    char name[80];
    int c, w, x1, y1, x2, y2;
    if (sscanf(rec.c_str(), "FontName %s", font) == 1)
    {
      strcat(strcat(strcpy(buff, "..\\font\\"), font), ".pfmz");
    }
    else if (sscanf(rec.c_str(), "C %d ; WX %d ; N %s ; B %d %d %d %d ;",
      &c, &w, name, &x1, &y1, &x2, &y2) == 7 && c >= 32 && c < 256)
    {
      short *b = bb+5*(c-c0);
      b[0] = (short) w;
      b[1] = (short) x1;
      b[2] = (short) y1;
      b[3] = (short) x2;
      b[4] = (short) y2;
    }
  }

  std::ofstream out(buff, std::ios::binary);
  if (!out.write((const char*) bb, sizeof(bb)))
    return false;

  strcat(strcpy(buff, base), ".pfa");
  std::ifstream pfa(buff);
  if (pfa)
  {
    z_stream zz;
    zz.zalloc = Z_NULL;
    zz.zfree = Z_NULL;
    zz.opaque = 0;
    int code = deflateInit(&zz, Z_BEST_COMPRESSION);
    if (code != Z_OK)
      return false;

    zz.next_out = (Bytef*) buff;
    zz.avail_out = sizeof(buff);
    while (std::getline(pfa, rec))
    {
      // trim comments
      std::string::size_type pos = rec.find('%');
      if (pos != std::string::npos)
        rec.erase(pos);
      if (rec.empty())
        continue;
      // setup limit on a string length 
      // to provide DSC conformance
      for (pos = 80; pos < rec.size(); pos += 80)
      {
        rec.insert(pos, 1, '\n');
      }
      rec += '\n';
      zz.next_in = (Bytef*) rec.data();
      zz.avail_in = (uInt) rec.size();
      if (zz.avail_out == 0)
      {
        out.write(buff, sizeof(buff));
        zz.next_out = (Bytef*) buff;
        zz.avail_out = sizeof(buff);
      }
      code = deflate(&zz, Z_NO_FLUSH);

      while (code == Z_OK && zz.avail_in > 0)
      {
        if (zz.avail_out == 0)
        {
          out.write(buff, sizeof(buff));
          zz.next_out = (Bytef*) buff;
          zz.avail_out = sizeof(buff);
        }
        code = deflate(&zz, Z_NO_FLUSH);
      }
    }

    if (zz.avail_out == 0)
    {
      out.write(buff, sizeof(buff));
      zz.next_out = (Bytef*) buff;
      zz.avail_out = sizeof(buff);
    }
    code = deflate(&zz, Z_FINISH);

    while (code == Z_OK)
    {
      if (zz.avail_out == 0)
      {
        out.write(buff, sizeof(buff));
        zz.next_out = (Bytef*) buff;
        zz.avail_out = sizeof(buff);
      }
      code = deflate(&zz, Z_FINISH);
    }
    if (code != Z_STREAM_END)
    {
      code = deflateEnd(&zz);
      return false;
    }
    out.write(buff, sizeof(buff) - zz.avail_out);
    code = deflateEnd(&zz);
    if (code != Z_OK)
      return false;
  }

  res << font << " pfmz \"..\\\\font\\\\" << font << ".pfmz\"\n";
  hdr << "#define FONT_" << std::setw(2) << std::setfill('0') << ++font_num 
    << " \"" << font << "\"\n";

  return true;
}

#define DATA_FOLDER "c:\\gnu\\share\\fonts\\"

int main()
{
  WIN32_FIND_DATA data;
  HANDLE hh = FindFirstFile("..\\font\\*.pfmz", &data);
  if (hh != INVALID_HANDLE_VALUE)
  {
    do
    {
      char file[_MAX_PATH] = "..\\font\\";

      if (lstrcmp(data.cFileName, ".") == 0)
        continue;

      if (lstrcmp(data.cFileName, "..") == 0)
        continue;

      lstrcat(file, data.cFileName);
      DeleteFile(file);
    }
    while (FindNextFile(hh, &data));
    FindClose(hh);
  }
  else
  {
    CreateDirectory("..\\font", NULL);
  }

  std::ofstream res("ps_font.rc"), hdr("ps_font.h");

  mkfont(res, hdr, DATA_FOLDER "core\\Courier");
  mkfont(res, hdr, DATA_FOLDER "core\\Courier-Bold");
  mkfont(res, hdr, DATA_FOLDER "core\\Courier-BoldOblique");
  mkfont(res, hdr, DATA_FOLDER "core\\Courier-Oblique");
  mkfont(res, hdr, DATA_FOLDER "core\\Helvetica");
  mkfont(res, hdr, DATA_FOLDER "core\\Helvetica-Bold");
  mkfont(res, hdr, DATA_FOLDER "core\\Helvetica-BoldOblique");
  mkfont(res, hdr, DATA_FOLDER "core\\Helvetica-Oblique");
  mkfont(res, hdr, DATA_FOLDER "core\\Symbol");
  mkfont(res, hdr, DATA_FOLDER "core\\Times-Bold");
  mkfont(res, hdr, DATA_FOLDER "core\\Times-BoldItalic");
  mkfont(res, hdr, DATA_FOLDER "core\\Times-Italic");
  mkfont(res, hdr, DATA_FOLDER "core\\Times-Roman");
  mkfont(res, hdr, DATA_FOLDER "core\\ZapfDingbats");

  mkfont(res, hdr, DATA_FOLDER "panterra\\smb293__");
  mkfont(res, hdr, DATA_FOLDER "panterra\\smb307__");
  mkfont(res, hdr, DATA_FOLDER "panterra\\symb1-29");
  mkfont(res, hdr, DATA_FOLDER "panterra\\UniWell");
  
  mkfont(res, hdr, DATA_FOLDER "extra\\CC50012M");
  mkfont(res, hdr, DATA_FOLDER "extra\\CC50016M");
  mkfont(res, hdr, DATA_FOLDER "extra\\CC50032M");
  mkfont(res, hdr, DATA_FOLDER "extra\\CC50036M");
  mkfont(res, hdr, DATA_FOLDER "extra\\GC05002T");
  mkfont(res, hdr, DATA_FOLDER "extra\\GC05004T");
  mkfont(res, hdr, DATA_FOLDER "extra\\GC05022T");
  mkfont(res, hdr, DATA_FOLDER "extra\\LC14013T");
  mkfont(res, hdr, DATA_FOLDER "extra\\LC21004L");
  mkfont(res, hdr, DATA_FOLDER "extra\\LC21006L");
  mkfont(res, hdr, DATA_FOLDER "extra\\MC04122T");
  mkfont(res, hdr, DATA_FOLDER "extra\\NC19003L");
  mkfont(res, hdr, DATA_FOLDER "extra\\NC19004L");
  mkfont(res, hdr, DATA_FOLDER "extra\\NC19023L");
  mkfont(res, hdr, DATA_FOLDER "extra\\NC19024L");
  mkfont(res, hdr, DATA_FOLDER "extra\\NC20003D");
  mkfont(res, hdr, DATA_FOLDER "extra\\NC20004D");
  mkfont(res, hdr, DATA_FOLDER "extra\\NC20023D");
  mkfont(res, hdr, DATA_FOLDER "extra\\NC20024D");
  mkfont(res, hdr, DATA_FOLDER "extra\\NC29003T");
  mkfont(res, hdr, DATA_FOLDER "extra\\NC29004T");
  mkfont(res, hdr, DATA_FOLDER "extra\\TC05002T");
  mkfont(res, hdr, DATA_FOLDER "extra\\TC05022T");

  return 0;
}
