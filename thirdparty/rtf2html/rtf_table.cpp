#include "rtf_table.h"
#include <set>
#include <ostream>
#include <iostream>
#include <stdexcept>
#include <functional>
#include <algorithm>

typedef std::set<int> intset;

template <class T, class C>
std::basic_ostream<C>& operator<<(std::basic_ostream<C> &dest, std::set<T> &s)
{
   for (typename std::set<T>::iterator i=s.begin(); i!=s.end(); ++i)
      dest<<*i<<" ";
   return dest;
}

std::string table::make()
{
   std::string result;
   intset pts;
   iterator row, span_row, row2;
   table_cell_defs::iterator cell_def, prev_cell_def, cell_def_2;
   table_cells::iterator cell;
   intset::iterator pt, ptp;
   int left, right, colspan;
   bool btop, bbottom, bleft, bright;
   std::string style;
   for (row=begin(); row!=end();)
   {
      if ((*row)->Cells.empty())
      {
         delete *row;
         row=erase(row);
      }
      else
      {
         pts.insert((*row)->Left);
         for (cell_def=(*row)->CellDefs->begin(); cell_def!=(*row)->CellDefs->end(); ++cell_def)
         {
            pts.insert((*cell_def)->Right);
         }
         ++row;
      }
   }
   if (pts.empty())
   {
      throw std::logic_error("No CellDefs!");
   }
   pt=pts.begin();
   ptp=pts.end();
   ptp--;
   result="<table border=0 width=";
   result+=from_int((int)rint((*ptp-*pt)/15));
   result+=" style=\"margin-left:";
   result+=from_int((int)rint(*pt/15));
   result+=";border-collapse: collapse;\">";
   result+="<tr height=0>";
   for (ptp=pt++=pts.begin(); pt!=pts.end(); ptp=pt++)
   {
      result+="<td width=";
      result+=from_int((int)rint((*pt-*ptp)/15));
      result+="></td>";
                            //coefficient may be different
   }
   result+="</tr>\n";

   // first, we'll determine all the rowspans and leftsides
   for (row=begin(); row!=end(); ++row)
   {
      if ((*row)->CellDefs->size()!=(*row)->Cells.size())
         throw std::logic_error("Number of Cells and number of CellDefs are unequal!");
      for (cell_def=(*row)->CellDefs->begin(), cell=(*row)->Cells.begin();
           cell!=(*row)->Cells.end();
           ++cell, prev_cell_def=cell_def++
          )
      {
         if (cell_def==(*row)->CellDefs->begin())
            (*cell_def)->Left=(*row)->Left;
         else
            (*cell_def)->Left=(*prev_cell_def)->Right;
         if ((*cell_def)->FirstMerged)
         {
            for (span_row=row, ++span_row; span_row!=end();
                 ++span_row)
            {
               cell_def_2=
                   std::find_if((*span_row)->CellDefs->begin(),
                                (*span_row)->CellDefs->end(),
                                std::bind2nd(
                                   std::mem_fun(&table_cell_def::right_equals),
                                                (*cell_def)->Right));
               if (cell_def_2==(*span_row)->CellDefs->end())
                  break;
               if (!(*cell_def_2)->Merged)
                  break;
            }
            (*cell)->Rowspan=span_row-row;
         }
      }
   }

   for (row=begin(); row!=end(); ++row)
   {
      result+="<tr>";
      pt=pts.find((*row)->Left);
      if (pt==pts.end())
         throw std::logic_error("No row.left point!");
      if (pt!=pts.begin())
      {
         result+="<td colspan=";
         result+=from_int(std::distance(pts.begin(), pt));
         result+="></td>";
      }
      for (cell_def=(*row)->CellDefs->begin(), cell=(*row)->Cells.begin();
           cell!=(*row)->Cells.end(); ++cell, ++cell_def)
      {
         ptp=pts.find((*cell_def)->Right);
         if (ptp==pts.end())
            throw std::logic_error("No celldef.right point!");
         colspan=std::distance(pt, ptp);
         pt=ptp;
         if (!(*cell_def)->Merged)
         {
            result+="<td";
            // analyzing borders
            left=(*cell_def)->Left;
            right=(*cell_def)->Right;
            bbottom=(*cell_def)->BorderBottom;
            btop=(*cell_def)->BorderTop;
            bleft=(*cell_def)->BorderLeft;
            bright=(*cell_def)->BorderRight;
            span_row=row;
            if ((*cell_def)->FirstMerged)
               std::advance(span_row, (*cell)->Rowspan-1);
            for (row2=row; row2!=span_row; ++row2)
            {
               cell_def_2=
                   std::find_if((*row2)->CellDefs->begin(),
                                (*row2)->CellDefs->end(),
                                std::bind2nd(
                                   std::mem_fun(&table_cell_def::right_equals),
                                                left));
               if (cell_def_2!=(*row2)->CellDefs->end())
               {
                  bleft=bleft && (*cell_def_2)->BorderRight;
               }
               cell_def_2=
                   std::find_if((*row2)->CellDefs->begin(),
                                (*row2)->CellDefs->end(),
                                std::bind2nd(
                                   std::mem_fun(&table_cell_def::left_equals),
                                                right));
               if (cell_def_2!=(*row2)->CellDefs->end())
               {
                  bright=bright && (*cell_def_2)->BorderLeft;
               }
            }

            if (bbottom && btop && bleft && bright)
            {
               style="border:1px solid black;";
            }
            else
            {
               style="";
               if (bbottom)
                  style+="border-bottom:1px solid black;";
               if (btop)
                  style+="border-top:1px solid black;";
               if (bleft)
                  style+="border-left:1px solid black;";
               if (bright)
                  style+="border-right:1px solid black;";
            }
            if (!style.empty())
            {
               result+=" style=\"";
               result+=style;
               result+="\"";
            }
            if (colspan>1)
            {
               result+=" colspan=";
               result+=from_int(colspan);
            }
            if ((*cell_def)->FirstMerged)
            {
               result+=" rowspan=";
               result+=from_int((*cell)->Rowspan);
            }

            switch ((*cell_def)->VAlign)
            {
            case table_cell_def::valign_top:
               result+=" valign=top";
               break;
            case table_cell_def::valign_bottom:
               result+=" valign=bottom";
               break;
            case table_cell_def::valign_center:
                break;
            }

            result+=">";
            if ((*cell)->Text[0]>0)
               result+=(*cell)->Text;
            else
               result+="&nbsp;";
            result+="</td>";
         }
      }
      result+="</tr>";
   }
   result+="</table>";
   return result;
}
