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
#include "mscore/preferences.h"
#include "voice.h"
#include "zone.h"

SamplePool::SamplePool()
      {
      _streaming = Ms::preferences.zerberusUseStreaming;
      _fillPercentage = (float) Ms::preferences.zerberusRefillPercent / 100.0f;
      _streamBufferSize = Ms::preferences.zerberusStreamingBufferSize;
      qDebug() << "Streaming: " << _streaming << " BufferSize: " << _streamBufferSize << " fill Percentage " << _fillPercentage;

      if (streaming()) {
            fillBuffersThread = new QThread();
            bufferWorker = new BufferWorker(this);
            bufferWorker->moveToThread(fillBuffersThread);
            connect(this, SIGNAL(fillBuffers()), bufferWorker, SLOT(fillBuffers()));
            connect(this, SIGNAL(addStream(SampleStream*)), bufferWorker, SLOT(addStream(SampleStream*)));
            connect(this, SIGNAL(deleteStream(SampleStream*)), bufferWorker, SLOT(deleteStream(SampleStream*)));
            fillBuffersThread->start();
            }
      }

Sample* SamplePool::getSamplePointer(QString filename)
       {
       std::map<QString, Sample*>::iterator sampleIterator = filename2sample.find(filename);
       if (sampleIterator != filename2sample.end())
             return sampleIterator->second;

       Sample* sa;

      try {
            sa  = new Sample(filename, _streaming, _streamBufferSize);
            }
      catch (...) {
            delete sa;
            return nullptr;
            }

      filename2sample.insert(std::pair<QString, Sample*>(filename, sa));

      return sa;
      }

SampleStream* SamplePool::getSampleStream(Voice* v)
      {
      SampleStream* sampleStream;
      try {
            sampleStream = new SampleStream(v, this);
            }
      catch (...) {
            delete sampleStream;
            return nullptr;
            }

      //streams.push_back(sampleStream);
      emit addStream(sampleStream);
      return sampleStream;
      }

void SamplePool::deleteSampleStream(SampleStream *sampleStream)
      {
      emit deleteStream(sampleStream);
      }

void SamplePool::triggerBufferRefill()
      {
      if (refillRuns)
            return;
      emit fillBuffers();
      }

SamplePool:: ~SamplePool()
      {
      fillBuffersThread->quit();
      fillBuffersThread->wait();
      delete fillBuffersThread;
      delete bufferWorker;
      qDeleteAll(streams);

      // delete samples
      for (auto f2s : filename2sample)
            delete f2s.second;
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
            buffer = new short[sp->streamBufferSize() * s->channel()];

            // init buffer from sample precache
            memcpy(buffer, s->data(), sizeof(short) * (sp->streamBufferSize() * s->channel()));
            memset(&info, 0, sizeof(info));
            sf = sf_open(s->filename().toLocal8Bit().constData(), SFM_READ, &info);
            if (!sf) {
                  qDebug("Error opening file %s with error %s", s->filename().toLocal8Bit().constData(), sf_strerror(sf));
                  throw ERROR_OPENING_FILE;
                  }
            writePos = sp->streamBufferSize() * s->channel();
            bufferSize = writePos;
          fillSize = writePos * sp->fillPercentage();
            readPos = 0;
            fileReadPos = sp->streamBufferSize();

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

void SampleStream::updateLoop(int idx)
      {
      bool validLoop = voice->_loopEnd > 0 &&
                  voice->_loopStart >= 0 &&
                  (voice->_loopEnd <= (voice->eidx/voice->audioChan));
      bool shallLoop = voice->loopMode() == LoopMode::CONTINUOUS ||
                  (voice->loopMode() == LoopMode::SUSTAIN &&
                  (voice->_state == VoiceState::PLAYING || voice->_state == VoiceState::SUSTAINED));

      if (voice->_looping && voice->loopMode() == LoopMode::SUSTAIN && (voice->_state != VoiceState::PLAYING || voice->_state != VoiceState::SUSTAINED)) {
            voice->_looping = false;
            qDebug() << "Switch looping to false: Loopmode " << int(voice->loopMode()) << " voice state " << int(voice->_state);
            }

      if (!(validLoop && shallLoop))
            return;

      if (idx > voice->_loopEnd) {
            voice->_looping = true;
            if (streaming)
                  voice->eidx += loopDuration * voice->_sample->channel();
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
            if ((unsigned int) pos > readPos && (unsigned int) pos < writePos)
                  readPos = pos;
            if ((writePos - readPos) <= fillSize)
                  samplePool->triggerBufferRefill();
            if ((unsigned int) pos >= writePos) {
                  qDebug("ERROR: streaming buffer empty! pos %d, writePos %d", pos, writePos);
                  // TODO Skip reading if already behind
                  return 0;
                  }
            return buffer[pos % bufferSize];
            }
      }

void SampleStream::fillBuffer() {
      if (!streaming)
            return;

      unsigned int writePosInBuffer = writePos % bufferSize;
      if (readPos < backwardSampleCount) {
            return;
            }
      unsigned int readBackInBuffer = (readPos - backwardSampleCount) % bufferSize;

      // buffer is more filled than desired fillSize
      if ((writePos - readPos) >= fillSize) {
            return;
            }

      sf_count_t toFill;

      if (readBackInBuffer < writePosInBuffer)
            toFill = bufferSize - writePosInBuffer;
      else
            toFill = readBackInBuffer - writePosInBuffer;

      updateLoop((toFill + fileReadPos) * voice->_sample->channel());
      toFill /= voice->_sample->channel(); // to fill in frames
      if (fileReadPos+toFill > info.frames)
            toFill -= ((fileReadPos+toFill) - info.frames);
      while (toFill > 0) {
            // Just to make sure no nasty things happen -> remove when every seems to work good
            //Q_ASSERT(toFill + (writePosInBuffer / bufferSize) < samplePool->streamBufferSize());
            unsigned int framesThatShouldBeRead = toFill;

            sf_seek(sf, fileReadPos, SEEK_SET);
            sf_count_t frames_read;

            if (voice->_looping && (toFill+fileReadPos > voice->_loopEnd)) {
                  sf_count_t untilLoop =voice->_loopEnd - fileReadPos;
                  frames_read = sf_readf_short(sf, &buffer[writePosInBuffer], untilLoop);
                  framesThatShouldBeRead = untilLoop;
                  }
            else {
                  frames_read = sf_readf_short(sf, &buffer[writePosInBuffer], toFill);
                  }

            writePos += frames_read * voice->_sample->channel();
            writePosInBuffer = writePos % bufferSize;
            fileReadPos += frames_read;
            toFill -= frames_read;

            if (voice->_looping && fileReadPos >= voice->_loopEnd)
                  fileReadPos -= loopDuration;

            if (framesThatShouldBeRead != frames_read) {
                  qDebug("ERROR: reading file %s with error %s",voice->_sample->filename().toLocal8Bit().constData() ,sf_strerror(sf));
                  throw ERROR_READING_FILE;
                  }
            }
      }

void BufferWorker::fillBuffers()
      {
      samplePool->refillRuns = true;
      for (SampleStream* sampleStream : samplePool->streams) {
            try {
                  sampleStream->fillBuffer();
                  }
            catch (...) {
                  qDebug("ERROR filling buffer");
                  }
            }
      samplePool->refillRuns = false;
      }

void BufferWorker::addStream(SampleStream *sampleStream)
      {
      samplePool->streams.push_back(sampleStream);
      }

void BufferWorker::deleteStream(SampleStream *sampleStream)
      {
      std::vector<SampleStream*>::iterator toDelete = samplePool->streams.end();
      for (std::vector<SampleStream*>::iterator i = samplePool->streams.begin(); i != samplePool->streams.end(); ++i) {
            if (*i == sampleStream) {
                  toDelete = i;
                  break;
                  }
            }
      if (toDelete != samplePool->streams.end()) {
            samplePool->streams.erase(toDelete);
            delete sampleStream;
            }
      else
            qDebug("Could not find samplestream!");
      }

