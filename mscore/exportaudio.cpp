//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: exportaudio.cpp 5660 2012-05-22 14:17:39Z wschweer $
//
//  Copyright (C) 2009 Werner Schweer and others
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

#include "config.h"
#include <sndfile.h>
#include "libmscore/score.h"
#include "libmscore/note.h"
#include "libmscore/part.h"
#include "libmscore/mscore.h"
#include "synthesizer/msynthesizer.h"
#include "musescore.h"
#include "preferences.h"

namespace Ms {

#ifdef HAS_AUDIOFILE

//---------------------------------------------------------
//   saveAudio
//---------------------------------------------------------

bool MuseScore::saveAudio(Score* score, const QString& name, const QString& ext)
      {
      int format;
      if (ext == "wav")
            format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
      else if (ext == "ogg")
            format = SF_FORMAT_OGG | SF_FORMAT_VORBIS;
      else if (ext == "flac")
            format = SF_FORMAT_FLAC | SF_FORMAT_PCM_16;
      else {
            qDebug("unknown audio file type <%s>", qPrintable(ext));
            return false;
            }

      EventMap events;
      score->renderMidi(&events);
      if(events.size() == 0)
            return false;

      MasterSynthesizer* synti = synthesizerFactory();
      synti->init();
      int sampleRate = preferences.exportAudioSampleRate;
      synti->setSampleRate(sampleRate);
      synti->setState(score->synthesizerState());

      int oldSampleRate  = MScore::sampleRate;
      MScore::sampleRate = sampleRate;

      SF_INFO info;
      memset(&info, 0, sizeof(info));
      info.channels   = 2;
      info.samplerate = sampleRate;
      info.format     = format;
      SNDFILE* sf     = sf_open(qPrintable(name), SFM_WRITE, &info);
      if (sf == 0) {
            qDebug("open soundfile failed: %s", sf_strerror(sf));
            delete synti;
            MScore::sampleRate = oldSampleRate;
            return false;
            }

      QProgressBar* pBar = showProgressBar();
      pBar->reset();

      float peak  = 0.0;
      double gain = 1.0;
      EventMap::const_iterator endPos = events.cend();
      --endPos;
      const int et = (score->utick2utime(endPos->first) + 1) * MScore::sampleRate;
      pBar->setRange(0, et);

      for (int pass = 0; pass < 2; ++pass) {
            EventMap::const_iterator playPos;
            playPos = events.cbegin();

            //
            // init instruments
            //
            foreach(Part* part, score->parts()) {
                  InstrumentList* il = part->instrList();
                  for(auto i = il->begin(); i!= il->end(); i++) {
                        foreach(const Channel& a, i->second.channel()) {
                              a.updateInitList();
                              foreach(MidiCoreEvent e, a.init) {
                                    if (e.type() == ME_INVALID)
                                          continue;
                                    e.setChannel(a.channel);
                                    int syntiIdx= synti->index(score->midiMapping(a.channel)->articulation->synti);
                                    synti->play(e, syntiIdx);
                                    }
                              }
                        }
                  }

            static const unsigned FRAMES = 512;
            float buffer[FRAMES * 2];
            int playTime = 0;

            for (;;) {
                  unsigned frames = FRAMES;
                  //
                  // collect events for one segment
                  //
                  memset(buffer, 0, sizeof(float) * FRAMES * 2);
                  int endTime = playTime + frames;
                  float* p = buffer;
                  for (; playPos != events.cend(); ++playPos) {
                        int f = score->utick2utime(playPos->first) * MScore::sampleRate;
                        if (f >= endTime)
                              break;
                        int n = f - playTime;
                        if (n) {
                              synti->process(n, p);
                              p += 2 * n;
                              }

                        playTime  += n;
                        frames    -= n;
                        const NPlayEvent& e = playPos->second;
                        if (e.isChannelEvent()) {
                              int channelIdx = e.channel();
                              Channel* c = score->midiMapping(channelIdx)->articulation;
                              if (!c->mute) {
                                    synti->play(e, synti->index(c->synti));
                                    }
                              }
                        }
                  if (frames) {
                        synti->process(frames, p);
                        playTime += frames;
                        }
                  if (pass == 1) {
                        for (unsigned i = 0; i < FRAMES * 2; ++i)
                              buffer[i] *= gain;
                        sf_writef_float(sf, buffer, FRAMES);
                        }
                  else {
                        for (unsigned i = 0; i < FRAMES * 2; ++i)
                              peak = qMax(peak, qAbs(buffer[i]));
                        }
                  playTime = endTime;
                  pBar->setValue((pass * et + playTime) / 2);

                  if (playTime >= et)
                        break;
                  }
            if (pass == 0 && peak == 0.0) {
                  qDebug("song is empty");
                  break;
                  }
            gain = 0.99 / peak;
            }

      hideProgressBar();

      MScore::sampleRate = oldSampleRate;
      delete synti;
      if (sf_close(sf)) {
            qDebug("close soundfile failed");
            return false;
            }

      return true;
      }

#endif // HAS_AUDIOFILE
}

