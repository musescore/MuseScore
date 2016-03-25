#ifndef __RTF_TOOLS_H__
#define __RTF_TOOLS_H__

#include "config.h"
#include "common.h"
#include "rtf_keyword.h"

template <class InputIter>
void skip_group(InputIter &iter);


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

#endif
