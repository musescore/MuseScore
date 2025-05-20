#ifndef __RTF_H__
#define __RTF_H__

#include "config.h"
#if defined(_STLP_DEBUG) && defined (__BORLANDC__)
   #include <stdio.h> // Just to make debug version of STLPort work under BC
#endif
#include "common.h"
#include <vector>
#include <cmath>
#include <list>
#include <cstdlib>

struct table_cell
{
   int Rowspan;
   std::string Text;
   table_cell() : Rowspan(0) {}
};

struct table_cell_def
{
   enum valign {valign_top, valign_bottom, valign_center};
   bool BorderTop, BorderBottom, BorderLeft, BorderRight;
   bool *ActiveBorder;
   int Right, Left;
   bool Merged, FirstMerged;
   valign VAlign;
   table_cell_def()
   {
      BorderTop=BorderBottom=BorderLeft=BorderRight=Merged=FirstMerged=false;
      ActiveBorder=NULL;
      Right=Left=0;
      VAlign=valign_top;
   }
   bool right_equals(int x) { return x==Right; }
   bool left_equals(int x) { return x==Left; }
};

template <class T> 
class killing_ptr_vector : public std::vector<T*>
{
 public:
   ~killing_ptr_vector()
   {
      for (typename killing_ptr_vector<T>::iterator i=this->begin(); i!=this->end(); ++i)
         delete *i;
   }
};

typedef killing_ptr_vector<table_cell> table_cells;
typedef killing_ptr_vector<table_cell_def> table_cell_defs;

typedef std::list<table_cell_defs> table_cell_defs_list;

struct table_row
{
   table_cells Cells;
   table_cell_defs_list::iterator CellDefs;
   int Height;
   int Left;
   table_row() : Height(-1000),  Left(-1000) {}
};

class table : public killing_ptr_vector<table_row>
{
 private:
   typedef killing_ptr_vector<table_row> base_class;
 public:
   table() : base_class() {}
   std::string make();
};

#endif
