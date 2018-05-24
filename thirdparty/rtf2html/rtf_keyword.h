#ifndef __RTF_KEYWORD_H__
#define __RTF_KEYWORD_H__


#include "config.h"
#include <string>
#include <map>
#include <ctype.h>
#include <stdlib.h>

class rtf_keyword{
 public:
   enum keyword_type {rkw_unknown,
                      rkw_b, rkw_bin, rkw_blue, rkw_brdrnone, rkw_bullet,
                      rkw_cb, rkw_cell, rkw_cellx, rkw_cf, rkw_clbrdrb, rkw_clbrdrl,
                      rkw_clbrdrr, rkw_clbrdrt, rkw_clvertalb, rkw_clvertalc,
                      rkw_clvertalt, rkw_clvmgf, rkw_clvmrg, rkw_colortbl,
                      rkw_emdash, rkw_emspace, rkw_endash, rkw_enspace,
                      rkw_fi, rkw_field, rkw_filetbl,
                      rkw_f, rkw_fprq, rkw_fcharset,
                      rkw_fnil, rkw_froman, rkw_fswiss, rkw_fmodern,
                      rkw_fscript, rkw_fdecor, rkw_ftech, rkw_fbidi,
                      rkw_fldrslt, rkw_fonttbl, rkw_footer, rkw_footerf, rkw_fs,
                      rkw_green,
                      rkw_header, rkw_headerf, rkw_highlight,
                      rkw_i, rkw_info, rkw_intbl,
                      rkw_ldblquote, rkw_li, rkw_line, rkw_lquote,
                      rkw_margl,
                      rkw_object,
                      rkw_paperw, rkw_par, rkw_pard, rkw_pict, rkw_plain,
                      rkw_qc, rkw_qj, rkw_ql, rkw_qmspace, rkw_qr,
                      rkw_rdblquote, rkw_red, rkw_ri, rkw_row, rkw_rquote,
                      rkw_sa, rkw_sb, rkw_sect, rkw_softline, rkw_stylesheet,
                      rkw_sub, rkw_super,
                      rkw_tab, rkw_title, rkw_trleft, rkw_trowd, rkw_trrh,
                      rkw_ul, rkw_ulnone
                     };
 private:
   class keyword_map : public std::map<std::string, keyword_type>
   {
    private:
       typedef std::map<std::string, keyword_type> base_class;
    public:
       keyword_map();
   };
 private:
   static keyword_map keymap;
   std::string s_keyword;
   keyword_type e_keyword;
   int param;
   char ctrl_chr;
   bool is_ctrl_chr;
 public:
   // iter must point after the backslash starting the keyword. We don't check it.
   // after construction, iter points at the char following the keyword
   template <class InputIter> explicit rtf_keyword(InputIter &iter);
   bool is_control_char() const
   {  return is_ctrl_chr; }
   const std::string &keyword_str() const
   {  return s_keyword; }
   keyword_type keyword() const
   {  return e_keyword; }
   int parameter() const
   {  return param; }
   char control_char() const
   {  return ctrl_chr; }
};

template <class InputIter>
rtf_keyword::rtf_keyword(InputIter &iter)
{
   char curchar=*iter;
   is_ctrl_chr=!isalpha(curchar);

   if (is_ctrl_chr)
   {
      ctrl_chr=curchar;
      ++iter;
   }
   else
   {
      do
         s_keyword+=curchar;
      while (isalpha(curchar=*++iter));
      std::string param_str;
      while (isdigit(curchar)||curchar=='-')
      {
         param_str+=curchar;
         curchar=*++iter;
      }
      if (param_str.empty())
         param=-1;
      else
//         param=std::atoi(param_str.c_str());
         param=atoi(param_str.c_str());
      if (curchar==' ')
         ++iter;
      keyword_map::iterator kw_pos=keymap.find(s_keyword);
      if (kw_pos==keymap.end())
         e_keyword=rkw_unknown;
      else
         e_keyword=kw_pos->second;
   }
}

#endif
