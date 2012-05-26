#ifndef __FMT_OPTS_H__
#define __FMT_OPTS_H__

#include "config.h"
#include "common.h"
#include <stack>
#include <vector>
#include <deque>
#include <map>

struct color {
   int r, g, b;
   color() : r(-1), g(-1), b(-1) {}
   bool operator==(const color &clr)
   {
      return r==clr.r && g==clr.g && b==clr.b;
   }
   bool operator!=(const color &clr)
   {
      return !(*this==clr);
   }
   color &operator=(const color &clr)
   {
      r=clr.r; g=clr.g; b=clr.b;
      return *this;
   }
};

typedef std::vector<color> colorvect;

struct font {
   enum font_family {ff_none, ff_serif, ff_sans_serif, ff_cursive, 
                     ff_fantasy, ff_monospace};
   font_family family;
   std::string name;
   int pitch;
   int charset;
   font() : family(ff_none), name(), pitch(0), charset(0) {}
   bool operator==(const font &f)
   {
      return family==f.family && name==f.name;
   }
   bool operator!=(const font &f)
   {
      return !(*this==f);
   }
   font &operator=(const font &f)
   {
      family=f.family; name=f.name; pitch=f.pitch; charset=f.charset;
      return *this;
   }
};

typedef std::map<int, font> fontmap;

struct formatting_options
{
   enum halign {align_left, align_right, align_center, align_justify, align_error};
   enum valign {va_normal, va_sub, va_sup};
   bool chpBold, chpItalic, chpUnderline;
   valign chpVAlign;
   int chpFontSize, chpHighlight;
   color chpFColor, chpBColor;
   font chpFont;
   int papLeft, papRight, papFirst;
   int papBefore, papAfter;
   halign papAlign;
   bool papInTbl;
   formatting_options()
   {
      chpBold=chpItalic=chpUnderline=false;
      chpVAlign=va_normal;
      chpFontSize=chpHighlight=0;
      papLeft=papRight=papFirst=papBefore=papAfter=0;
      papAlign=align_left;
      papInTbl=false;
   }
   bool operator==(const formatting_options &opt) // tests only for character options
   {
      return chpBold==opt.chpBold && chpItalic==opt.chpItalic 
             && chpUnderline==opt.chpUnderline && chpVAlign==opt.chpVAlign
             && chpFontSize==opt.chpFontSize
             && chpFColor==opt.chpFColor && chpBColor==opt.chpBColor
             && chpHighlight==opt.chpHighlight && chpFont==opt.chpFont;
   }
   bool operator!=(const formatting_options &opt) // tests only for character options
   {
      return !(*this==opt);
   }
   formatting_options &operator=(const formatting_options &opt)
   {
      chpBold=opt.chpBold; chpItalic=opt.chpItalic;
      chpUnderline=opt.chpUnderline; chpVAlign=opt.chpVAlign;
      chpFontSize=opt.chpFontSize; 
      chpFColor=opt.chpFColor; chpBColor=opt.chpBColor;
      chpHighlight=opt.chpHighlight; chpFont=opt.chpFont;
      papLeft=opt.papLeft; papRight=opt.papRight;
      papFirst=opt.papFirst; papBefore=opt.papBefore; papAfter=opt.papAfter;
      papAlign=opt.papAlign; papInTbl=opt.papInTbl;
      return *this;
   }
   std::string get_par_str() const;
};

typedef std::stack<formatting_options> fo_stack;

typedef std::deque<formatting_options> fo_deque;

class formatter {
 private:
   fo_deque opt_stack;
 public:
   std::string format(const formatting_options &opt);
   std::string close();
   void clear() { opt_stack.clear(); }
};

class html_text {
 private:
   const formatting_options &opt;
   formatter fmt;
   std::string text;
 public:
   html_text(const formatting_options &_opt) : opt(_opt) {}
   const std::string &str() { return text; }
   template <class T> void write(T s)
   {
      text+=fmt.format(opt)+s;
   }
   std::string close() { return fmt.close(); }
//   void write(char c) { write(std::string()+c); }
   void clear() { text.clear(); fmt.clear(); }
};

#endif

