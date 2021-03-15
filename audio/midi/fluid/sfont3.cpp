#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "sfont.h"
#include "audiofile/audiofile.h"

namespace FluidS {

//---------------------------------------------------------
//   decompressOggVorbis
//---------------------------------------------------------

bool Sample::decompressOggVorbis(char* src, unsigned int size)
      {
      AudioFile af;
      QByteArray ba(src, size);

      start = 0;
      end   = 0;
      if (!af.open(ba)) {
            qDebug("SoundFont(%s) Sample(%s) decompressOggVorbis: open failed: %s", qPrintable(sf->get_name()), name, af.error());
            return false;
            }
      unsigned int frames = af.frames();
      data = new short[frames * af.channels()];
      if (frames != (unsigned int)af.readData(data, frames)) {
            qDebug("SoundFont(%s) Sample(%s) read failed: %s", qPrintable(sf->get_name()), name, af.error());
            delete[] data;
            data = 0;
            return false;
            }
      // cf. https://musescore.org/en/node/89216#comment-1068379 and following
      end = frames;

      // loop is fowled?? (cluck cluck :)
      if (loopend > end || loopstart >= loopend || loopstart <= start) {
            qWarning("SoundFont(%s) Sample(%s) start(%u) startloop(%u) endloop(%u) end(%u) fowled (broken soundfont?), fixing up",
                   qPrintable(sf->get_name()), name, start, loopstart, loopend, end);
            /* can pad loop by 8 samples and ensure at least 4 for loop (2*8+4) */
            if ((end - start) >= 20) {
                  loopstart = start + 8;
                  loopend   = end - 8;
                  }
            else {      // loop is fowled, sample is tiny (can't pad 8 samples)
                  loopstart = start + 1;
                  loopend   = end - 1;
                  }
            }
      if ((end - start) < 8) {
            qWarning("SoundFont(%s) Sample(%s) too small, disabling", qPrintable(sf->get_name()), name);
            return false;
            }

      return true;
      }
} // namespace
