#ifndef __RTF_TOOLS_H__
#define __RTF_TOOLS_H__

#include "config.h"
#include "common.h"
#include "rtf_keyword.h"

template <class InputIter>
void skip_group(InputIter &iter);

template <class InputIter>
std::string char_by_code(InputIter &iter);


/****************************************
function assumes that file pointer points AFTER the opening brace
and that the group is really closed. cs is caller's curchar.
Returns the character that comes after the enclosing brace.
*****************************************/

template <class InputIter>
void skip_group(InputIter &iter)
{
   int cnt=1;
   while (cnt)
   {
      switch (*iter++)
      {
      case '{':
         cnt++;
         break;
      case '}':
         cnt--;
         break;
      case '\\':
      {
         rtf_keyword kw(iter);
         if (!kw.is_control_char() && kw.keyword()==rtf_keyword::rkw_bin 
             && kw.parameter()>0)
         {
            std::advance(iter, kw.parameter());
         }
         break;
      }
      }
   }
}

template <class InputIter>
std::string char_by_code(InputIter &iter)
{
   std::string stmp(1, *iter++);
   stmp += *iter++;
   int code = std::strtol(stmp.c_str(), NULL, 16);
   switch (code)
   {
      case 0x3f:
         return std::string();
      case 147:
         return "&ldquo;";
      case 148:
         return "&rdquo;";
      case 167:
         return "&sect;";
      case 188:
         return "&frac14;";
      default:
         return std::string(1, (char)code); //(std::string("&#") + from_int(code) + ";");
   }
}

#endif
