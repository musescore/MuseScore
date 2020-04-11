#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "sfont.h"
#include "audiofile/audiofile.h"

namespace FluidS {

//---------------------------------------------------------
//   decompressOggVorbis
//---------------------------------------------------------

bool Sample::decompressOggVorbis(char* src, int size)
      {
      AudioFile af;
      QByteArray ba(src, size);

      start = 0;
      end   = 0;
      if (!af.open(ba)) {
            qDebug("Sample::decompressOggVorbis: open failed: %s", af.error());
            return false;
            }
      int frames = af.frames();
      data = new short[frames * af.channels()];
      if (frames != af.readData(data, frames)) {
            qDebug("Sample read failed: %s", af.error());
            delete[] data;
            data = 0;
            }
      end = frames - 1;

      if (loopend > end ||loopstart >= loopend || loopstart <= start) {
            /* can pad loop by 8 samples and ensure at least 4 for loop (2*8+4) */
            if ((end - start) >= 20) {
                  loopstart = start + 8;
                  loopend = end - 8;
                  }
            else { // loop is fowled, sample is tiny (can't pad 8 samples)
                  loopstart = start + 1;
                  loopend = end - 1;
                  }
            }
      if ((end - start) < 8) {
            qDebug("invalid sample");
            setValid(false);
            }

      return true;
      }
} // namespace
