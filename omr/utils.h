//=============================================================================
//  MusE Reader
//  Linux Music Score Reader
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

#ifndef __UTILS_H__
#define __UTILS_H__

namespace Ms {

extern double curTime();

//---------------------------------------------------------
//   Benchmark
//---------------------------------------------------------

class Benchmark {
      double startTime;
      const char* msg;

   public:
      Benchmark(const char* p) {
            msg = p;
            startTime = curTime();
            printf("===%s start\n", msg);
            }
      ~Benchmark() {
            double elapsed = curTime() - startTime;
            printf("===%s elapsed %f\n", msg, elapsed);
            }
      };

extern double covariance(const double data1[], const double data2[], int n);

}

#endif

