// #define _UNICODE
// #define UNICODE

/*
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
*/

#define FIELD_X1 1388000
#define FIELD_X2 1429000
#define FIELD_Y1 706000
#define FIELD_Y2 785000

#define STRINGIZE(lex) #lex
#define STRINGIZE2(lex) STRINGIZE(lex)
#define FILEPOS TEXT(__FILE__) TEXT("(") TEXT(STRINGIZE2(__LINE__)) TEXT("): ")

#define WIDECHAR_INLINE(text) L##text
#define WIDECHAR_INLINE2(text) WIDECHAR_INLINE(text)

#define FIELD_LEFT     WIDECHAR_INLINE2(STRINGIZE2(FIELD_X1))
#define FIELD_RIGHT    WIDECHAR_INLINE2(STRINGIZE2(FIELD_X2))
#define FIELD_BOTTOM   WIDECHAR_INLINE2(STRINGIZE2(FIELD_Y1))
#define FIELD_TOP      WIDECHAR_INLINE2(STRINGIZE2(FIELD_Y2))

void main()
{
  // printf(STRINGIZE2(FIELD_X1));
  // printf(STRINGIZE2(FIELD_X2));
  wprintf(FIELD_TOP);
  // wprintf(FILEPOS);
}
