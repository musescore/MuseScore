//=============================================================================
//  MusE Reader
//  Music Score Reader
//
//  Copyright (C) 2010 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "utils.h"
#include "omr.h"

namespace Ms {

char Omr::bitsSetTable[256];

//---------------------------------------------------------
//   initUtils
//---------------------------------------------------------

void Omr::initUtils()
      {
      static bool initialized = false;
      if (initialized)
            return;
      initialized = true;
      //
      // populate the bitsSetTable
      bitsSetTable[0] = 0;
      for (int i = 1; i < 256; i++)
            bitsSetTable[i] = (i & 1) + bitsSetTable[i/2];
      }

//---------------------------------------------------------
//   mean
//    Compute the arithmetic mean of a dataset using the
//    recurrence relation
//    mean_(n) = mean(n-1) + (data[n] - mean(n-1))/(n+1)
//---------------------------------------------------------

double mean(const double data[], int size)
      {
      long double mean = 0;

      for (int i = 0; i < size; i++)
            mean += (data[i] - mean) / (i + 1);
      return mean;
      }

//---------------------------------------------------------
//   covariance
//---------------------------------------------------------

double covariance(const double data1[],
   const double data2[], int n, double mean1, double mean2)
      {
      long double covariance = 0.0;

      /* find the sum of the squares */
      for (size_t i = 0; i < (size_t)n; i++) {
            const long double delta1 = (data1[i] - mean1);
            const long double delta2 = (data2[i] - mean2);
            covariance += (delta1 * delta2 - covariance) / (i + 1);
            }
      return covariance;
      }

double covariance(const double data1[], const double data2[], int n)
      {
      double mean1 = mean(data1, n);
      double mean2 = mean(data2, n);

      return covariance(data1, data2, n, mean1, mean2);
      }
}

