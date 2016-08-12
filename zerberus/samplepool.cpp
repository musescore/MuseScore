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
      if (streaming()) {
            fillBuffersThread = new BufferThread(this);
            fillBuffersThread->start();
            }
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
      streamMutex.lock();
      streams.push_back(sampleStream);
      streamMutex.unlock();
      return sampleStream;
      }

void SamplePool::deleteSampleStream(SampleStream *sampleStream)
      {
      streamMutex.lock();
      std::vector<SampleStream*>::iterator toDelete = streams.end();
      for (std::vector<SampleStream*>::iterator i = streams.begin(); i != streams.end(); ++i) {
            if (*i == sampleStream) {
                  toDelete = i;
                  break;
                  }
            }
      if (toDelete != streams.end()) {
            streams.erase(toDelete);
            delete sampleStream;
            }
      else
            qDebug("Could not find samplestream!");
      streamMutex.unlock();
      }

void SamplePool::fillSteamBuffers()
      {
      streamMutex.lock();
      for (SampleStream* sampleStream : streams)
            sampleStream->fillBuffer();
      streamMutex.unlock();
      }

SampleStream::SampleStream(Voice *v, SamplePool *sp)
      {
      voice = v;
      samplePool = sp;
      Sample* s = v->_sample;
      if (!sp->streaming() || !s->needsStreaming()) {
            streaming = false;
            buffer = s->data();
            }
      else {
            streaming = true;
            buffer = new short[STREAM_BUFFER_SIZE * s->channel()];

            // init buffer from sample precache
            memcpy(buffer, s->data(), sizeof(short) * (STREAM_BUFFER_SIZE * s->channel()));
            memset(&info, 0, sizeof(info));
            sf = sf_open(s->filename().toLocal8Bit().constData(), SFM_READ, &info);
            if (!sf) {
                  qDebug("Error opening file %s with error %s", s->filename().toLocal8Bit().constData(), sf_strerror(sf));
                  Q_ASSERT(sf != 0);
                  }
            writePos = STREAM_BUFFER_SIZE * s->channel();
            readPos = 0;
            fileReadPos = STREAM_BUFFER_SIZE;

            loopDuration = v->_loopEnd - v->_loopStart;
            backwardSampleCount = s->channel() * 4; // 4times interpol per channel
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
            if (streaming)
                  voice->eidx += loopDuration;
            else
                  voice->phase.setIndex(voice->_loopStart + (idx - voice->_loopEnd - 1));
            }
      }

short SampleStream::getData(int pos) {
      if (pos < 0 && !voice->_looping)
            return 0;

      if (!streaming) {
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
      else {
            readPosMutex.lock();
            if (pos > readPos && pos < writePos)
                  readPos = pos;
            if (pos >= writePos) {
                  //qDebug("ERROR: streaming buffer empty! pos %d, writePos %d", pos, writePos);
                  // TODO Skip reading if already behind
                  readPosMutex.unlock();
                  return 0;
                  }
            readPosMutex.unlock();
            return buffer[pos % (STREAM_BUFFER_SIZE * voice->_sample->channel())];
            }
      }

void SampleStream::fillBuffer() {
      if (!streaming)
            return;

      readPosMutex.lock();
      unsigned int writePosInBuffer = writePos % (STREAM_BUFFER_SIZE * voice->_sample->channel());
      if (readPos < backwardSampleCount) {
            readPosMutex.unlock();
            return;
            }
      unsigned int readBackInBuffer = (readPos - backwardSampleCount) % (STREAM_BUFFER_SIZE * voice->_sample->channel());

      // buffer is full
      if ((writePos - readPos) >= STREAM_BUFFER_SIZE) {
            readPosMutex.unlock();
            return;
            }

      readPosMutex.unlock();

      sf_count_t toFill;

      if (readBackInBuffer < writePosInBuffer)
            toFill = (STREAM_BUFFER_SIZE * voice->_sample->channel()) - writePosInBuffer;
      else
            toFill = readBackInBuffer - writePosInBuffer;

      toFill /= voice->_sample->channel(); // to fill in frames
      while (toFill > 0) {
            // Just to make sure no nasty things happen -> remove when every seems to work good
            Q_ASSERT(toFill + (writePosInBuffer / (STREAM_BUFFER_SIZE * voice->_sample->channel())) < STREAM_BUFFER_SIZE);
            unsigned int framesThatShouldBeRead = toFill;

            sf_seek(sf, fileReadPos, SEEK_SET);
            sf_count_t frames_read;

            if (voice->_looping && (toFill+fileReadPos > voice->_loopEnd)) {
                  sf_count_t untilLoop =voice->_loopEnd - fileReadPos;
                  frames_read = sf_readf_short(sf, &buffer[writePosInBuffer], untilLoop);
                  framesThatShouldBeRead = untilLoop;
                  }
            else
                  frames_read = sf_readf_short(sf, &buffer[writePosInBuffer], toFill);

            writePos += frames_read * voice->_sample->channel();
            writePosInBuffer = writePos % (STREAM_BUFFER_SIZE * voice->_sample->channel());
            fileReadPos += frames_read;
            toFill -= frames_read;

            if (voice->_looping && fileReadPos > voice->_loopEnd)
                  fileReadPos -= loopDuration;

            if (framesThatShouldBeRead != frames_read)
                  qDebug("ERROR: reading file with error %s", sf_strerror(sf));
            }
      }


void BufferThread::run()
      {
      while (true) {
            samplePool->fillSteamBuffers();
            // TODO use a time based on voices and buffer size!
            usleep(100);
            }
      }

