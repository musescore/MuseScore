#ifndef __COMMON_H__
#define __COMMON_H__

#include "config.h"
#include <string>
#include <sstream>
#include <iomanip>

#ifndef HAVE_RINT

#include <cmath>
   inline int rint(double f)
   { 
      return(f-std::floor(f)<0.5?std::floor(f):std::ceil(f));
   }

#endif

inline std::string from_int(int value)
{
   std::ostringstream buf;
   buf<<value;
   return buf.str();
}

inline std::string hex(unsigned int value)
{
   std::ostringstream buf;
   buf<<std::setw(2)<<std::setfill('0')<<std::hex<<value;
   return buf.str();
}

#endif
