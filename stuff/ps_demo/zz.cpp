#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string.h>

#if 0

std::ostream& foo(int argc, char** argv, std::ofstream& of)
{
  if (argc > 1 && stricmp(argv[1], "con:") != 0)
  {
    of.open(argv[1]);
    return of;
  }
  return std::cout; 
}

int main(int argc, char** argv)
{
  std::ofstream of;
  std::ostream& out = foo(argc, argv, of);
  for (int i = 0; i < 10000; ++i)
  {
    out << "Hello!\n";
  }
  return 0;
}

#else

int main()
{
  FILE* fp = freopen("con", "w", stdout);
  // FILE* fp = stdout;       
  fprintf(fp, "Hello!\n");
  fclose(fp);
  return 0;
}

#endif
