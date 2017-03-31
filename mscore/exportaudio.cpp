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
#ifdef HAS_AUDIOFILE
#include <sndfile.h>
#endif
#include "libmscore/score.h"
#include "libmscore/note.h"
#include "libmscore/part.h"
#include "libmscore/mscore.h"
#include "synthesizer/msynthesizer.h"
#include "musescore.h"
#include "preferences.h"

namespace Ms {

///
/// \brief Function to synthesize audio and output it into a generic QIODevice
/// \param The score to output
/// \param The output device
/// \param An optional callback function that will be notified with the progress in range [0, 1]
/// \return True on success, false otherwise.
///
/// If the callback function is non zero an returns false the export will be canceled.
///
bool MuseScore::saveAudio(Score* score, QIODevice *device, std::function<bool(float)> updateProgress)
    {
    if (!device) {
        qDebug() << "Invalid device";
        return false;
    }

    if (!device->open(QIODevice::WriteOnly)) {
        qDebug() << "Could not write to device";
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
    if (MScore::noGui) { // use score settings if possible
          bool r = synti->setState(score->synthesizerState());
          if (!r)
                synti->init();
          }
    else { // use current synth settings
          bool r = synti->setState(mscore->synthesizerState());
          if (!r)
                synti->init();
          }

    int oldSampleRate  = MScore::sampleRate;
    MScore::sampleRate = sampleRate;

    float peak  = 0.0;
    double gain = 1.0;
    EventMap::const_iterator endPos = events.cend();
    --endPos;
    const int et = (score->utick2utime(endPos->first) + 1) * MScore::sampleRate;
    const int maxEndTime = (score->utick2utime(endPos->first) + 3) * MScore::sampleRate;

    bool cancelled = false;
    for (int pass = 0; pass < 2; ++pass) {
          EventMap::const_iterator playPos;
          playPos = events.cbegin();
          synti->allSoundsOff(-1);

          //
          // init instruments
          //
          foreach(Part* part, score->parts()) {
                const InstrumentList* il = part->instruments();
                for(auto i = il->begin(); i!= il->end(); i++) {
                      foreach(const Channel* a, i->second->channel()) {
                            a->updateInitList();
                            foreach(MidiCoreEvent e, a->init) {
                                  if (e.type() == ME_INVALID)
                                        continue;
                                  e.setChannel(a->channel);
                                  int syntiIdx = synti->index(score->masterScore()->midiMapping(a->channel)->articulation->synti);
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
                float max = 0.0;
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
                            Channel* c = score->masterScore()->midiMapping(channelIdx)->articulation;
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
                      for (unsigned i = 0; i < FRAMES * 2; ++i) {
                            max = qMax(max, qAbs(buffer[i]));
                            buffer[i] *= gain;
                            }
                      device->write(reinterpret_cast<const char*>(buffer), 2 * FRAMES * sizeof(float));
                      }
                else {
                      for (unsigned i = 0; i < FRAMES * 2; ++i) {
                            max = qMax(max, qAbs(buffer[i]));
                            peak = qMax(peak, qAbs(buffer[i]));
                            }
                      }
                playTime = endTime;
                if (updateProgress) {
                    // normalize to [0, 1] range
                    if (!updateProgress((pass * et + playTime) / 2.0 / et)) {
                        cancelled = true;
                        break;
                    }
                      }
                if (playTime >= et)
                      synti->allNotesOff(-1);
                // create sound until the sound decays
                if (playTime >= et && max*peak < 0.000001)
                      break;
                // hard limit
                if (playTime > maxEndTime)
                      break;
                }
          if (cancelled)
                break;
          if (pass == 0 && peak == 0.0) {
                qDebug("song is empty");
                break;
                }
          gain = 0.99 / peak;
          }

    MScore::sampleRate = oldSampleRate;
    delete synti;

    device->close();

    return !cancelled;
}

#ifdef HAS_AUDIOFILE


//---------------------------------------------------------
//   saveAudio
//---------------------------------------------------------

bool MuseScore::saveAudio(Score* score, const QString& name)
      {
    // QIODevice - SoundFile wrapper class
    class SoundFileDevice : public QIODevice {
    private:
        SF_INFO info;
        SNDFILE *sf = nullptr;
        const QString filename;
    public:
        SoundFileDevice(int sampleRate, int format, const QString& name)
            : filename(name) {
            memset(&info, 0, sizeof(info));
            info.channels   = 2;
            info.samplerate = sampleRate;
            info.format     = format;
        }
        ~SoundFileDevice() {
            if (sf) {
                sf_close(sf);
                sf = nullptr;
            }
        }

        virtual qint64 readData(char *data, qint64 maxlen) override final {
            Q_UNUSED(data);
            qDebug() << "Error: No write supported!";
            return maxlen;
        }

        virtual qint64 writeData(const char *data, qint64 len) override final {
            int trueFrames = len / sizeof(float) / 2;
            sf_writef_float(sf, reinterpret_cast<const float*>(data), trueFrames);
            return trueFrames * 2 * sizeof(float);
        }

        bool open(QIODevice::OpenMode mode) {
            if ((mode & QIODevice::WriteOnly) == 0) {
                return false;
            }
            sf     = sf_open(qPrintable(filename), SFM_WRITE, &info);
            if (sf == 0) {
                  qDebug("open soundfile failed: %s", sf_strerror(sf));
                  return false;
            }
            return QIODevice::open(mode);
        }
        void close() {
            if (sf && sf_close(sf)) {
                  qDebug("close soundfile failed");
            }
            sf = nullptr;
            QIODevice::close();
        }
    };
      int format;
      if (name.endsWith(".wav"))
            format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
      else if (name.endsWith(".ogg"))
            format = SF_FORMAT_OGG | SF_FORMAT_VORBIS;
      else if (name.endsWith("flac"))
            format = SF_FORMAT_FLAC | SF_FORMAT_PCM_16;
      else {
            qDebug("unknown audio file type <%s>", qPrintable(name));
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
      bool r = synti->setState(score->synthesizerState());
      if (!r)
          synti->init();

      int oldSampleRate  = MScore::sampleRate;
      MScore::sampleRate = sampleRate;


      SoundFileDevice device(sampleRate, format, name);

      // dummy callback function that will be used if there is no gui
      std::function<bool(float)> progressCallback = [](float) {return true;};

      QProgressDialog progress(this);
      progress.setWindowFlags(Qt::WindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowTitleHint));
      progress.setWindowModality(Qt::ApplicationModal);
      //progress.setCancelButton(0);
      progress.setCancelButtonText(tr("Cancel"));
      progress.setLabelText(tr("Exporting..."));
      if (!MScore::noGui) {
          // callback function that will update the progress bar
          // it will return false and thus cancel the export if the user
          // cancels the progress dialog.
          progressCallback = [&progress](float v) -> bool {
              if (progress.wasCanceled())
                    return false;
              progress.setValue(v * 1000);
              qApp->processEvents();
              return true;
          };

            progress.show();
      }

      // The range is set arbitrarily to 1000 as steps.
      // The callback will return float numbers between 0 and 1
      // which will be scaled into integer 0 to 1000 numbers
      // which allows a smooth transition.
      progress.setRange(0, 1000);

      // Save the audio to the SoundFile device
      bool result = saveAudio(score, &device, progressCallback);

      bool wasCanceled = progress.wasCanceled();
      progress.close();

      MScore::sampleRate = oldSampleRate;
      delete synti;

      if (wasCanceled)
            QFile::remove(name);

      return result;
      }

#endif // HAS_AUDIOFILE
}

