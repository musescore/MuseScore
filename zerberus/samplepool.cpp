//=============================================================================
//  Zerberus
//  Zample player
//
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "samplepool.h"
#include "audiofile/audiofile.h"
#include "voice.h"
#include "zone.h"

SamplePool::SamplePool()
      {

      }

Sample* SamplePool::getSamplePointer(QString filename)
       {
       std::map<QString, Sample*>::iterator sampleIterator = filename2sample.find(filename);
       if (sampleIterator != filename2sample.end())
             return sampleIterator->second;

      Sample* sa  = new Sample(filename, _streaming);

      filename2sample.insert(std::pair<QString, Sample*>(filename, sa));

      return sa;
      }

SampleStream* SamplePool::getSampleStream(Voice* v)
      {
      SampleStream* sampleStream = new SampleStream(v, this);
      streams.push_back(sampleStream);
      return sampleStream;
      }

SampleStream::SampleStream(Voice *v, SamplePool *sp)
      {
      voice = v;
      samplePool = sp;
      Sample* s = v->_sample;
      if (!sp->streaming()) {
            streaming = false;
            buffer = s->data();
            }
      else {
            streaming = true;
            buffer = new short[STREAM_BUFFER_SIZE * s->channel()];
            memcpy(buffer, s->data(), STREAM_BUFFER_SIZE * s->channel());
            sf = sf_open(s->filename().toLocal8Bit().constData(), SFM_READ, &info);
            writePos = STREAM_BUFFER_SIZE - 1;
            fileReadPos = STREAM_BUFFER_SIZE - 1;
            sf_seek(sf, fileReadPos, SEEK_SET);
            }
      }

SampleStream::~SampleStream() {
      if (streaming) {
            delete[] buffer;
            sf_close(sf);
            }
      }

//---------------------------------------------------------
//   updateLoop
//---------------------------------------------------------

void SampleStream::updateLoop()
      {
      int idx = voice->phase.index();
      bool validLoop = voice->_loopEnd > 0 &&
                  voice->_loopStart >= 0 &&
                  (voice->_loopEnd <= (voice->eidx/voice->audioChan));
      bool shallLoop = voice->loopMode() == LoopMode::CONTINUOUS ||
                  (voice->loopMode() == LoopMode::SUSTAIN &&
                  (voice->_state == VoiceState::PLAYING || voice->_state == VoiceState::SUSTAINED));

      if (voice->_looping && voice->loopMode() == LoopMode::SUSTAIN && (voice->_state != VoiceState::PLAYING || voice->_state != VoiceState::SUSTAINED))
            voice->_looping = false;

      if (!(validLoop && shallLoop))
            return;

      if (idx > voice->_loopEnd) {
            voice->_looping = true;
            voice->phase.setIndex(voice->_loopStart + (idx - voice->_loopEnd - 1));
            }
      }

short SampleStream::getData(int pos) {
      if (pos < 0 && !voice->_looping)
            return 0;
      //TODO implement streaming

      if (!voice->_looping)
            return buffer[pos];

      int loopEnd = voice->_loopEnd * voice->audioChan;
      int loopStart = voice->_loopStart * voice->audioChan;

      if (pos < loopStart)
            return buffer[loopEnd + (pos - loopStart) + voice->audioChan];
      else if (pos > (loopEnd + voice->audioChan - 1))
            return buffer[loopStart + (pos - loopEnd) - voice->audioChan];
      else
            return buffer[pos];
      }

