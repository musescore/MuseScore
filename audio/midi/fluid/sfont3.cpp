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

      // try to fixup start<loopstart<loopend<end similar to recent FluidSynth;
      // some of these are technically invalid but exist in the field; see the
      // SoundFont 2.04 spec-related diagnostic about this in sfont.cpp
      if (loopstart == loopend) {
            // normalise to ensure they are within the sample, even non-looped
            loopstart = loopend = start;
            }
      else if (loopstart > loopend) {
            unsigned int looptmp;

            qWarning("SoundFont(%s) Sample(%s) swapping reversed loopstart<=>loopend",
                     qPrintable(sf->get_name()), name);
            looptmp = loopstart;
            loopstart = loopend;
            loopend = looptmp;
            }
      // ensure they are at least within the sample
      if (loopstart < start || loopstart > end) {
            qWarning("SoundFont(%s) Sample(%s) loopstart(%u) out of bounds [start=%u end=%u], setting to start",
                     qPrintable(sf->get_name()), name, loopstart, start, end);
            loopstart = start;
            }
      if (loopend < start || loopend > end) {
            qWarning("SoundFont(%s) Sample(%s) loopend(%u) out of bounds [start=%u end=%u], setting to end",
                     qPrintable(sf->get_name()), name, loopend, start, end);
            loopstart = end;
            }
      // ensure it can even play
      if ((end - start) < 8) {
            qWarning("SoundFont(%s) Sample(%s) too small, disabling", qPrintable(sf->get_name()), name);
            return false;
            }

      return true;
      }
} // namespace
