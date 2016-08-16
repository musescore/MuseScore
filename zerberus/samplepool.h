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

#ifndef SAMPLEPOOL_H
#define SAMPLEPOOL_H

#include "sample.h"
#include <map>

enum class LoopMode : char;
class Voice;
class SamplePool;

struct SampleLoop
      {
      Sample* sample;
      unsigned int loopStart = -1;
      unsigned int loopEnd = -1;
      LoopMode loopmode;
      };

class SampleStream
      {
      SamplePool* samplePool;
      short *buffer;
      bool streaming;

      unsigned int loopDuration;
      unsigned int readPos = 0;
      unsigned int writePos = 0;

      unsigned int backwardSampleCount;

      sf_count_t fileReadPos;
      Voice* voice;
      SF_INFO info;
      SNDFILE *sf;

public:
      SampleStream(Voice *v, SamplePool* sp);
      ~SampleStream();
      void updateLoop(int idx);
      short getData(int pos);
      void fillBuffer();
      };

class BufferWorker : public QObject
      {
      Q_OBJECT

      SamplePool* samplePool;
public slots:
      void fillBuffers();
public:
      BufferWorker(SamplePool* sp) : QObject(), samplePool(sp) {}
      };

class SamplePool : public QObject
      {
      Q_OBJECT

      std::map<QString, Sample*> filename2sample;
      std::vector<SampleStream *> streams;
      bool _streaming = true;
      float _fillPercentage = 0.3; // if the buffer is filled lower than this percentage it triggers a refill
      unsigned int _streamBufferSize = 4096; // size of the ringbuffer (in frames) that gets filled and read during streaming
      QThread* fillBuffersThread; // this thread will run the worker below
      BufferWorker* bufferWorker; // this worker will run the buffer refill in a seperate thread (once triggered)
      QMutex streamMutex; // mutex for stream access (by audio thread and buffer refill thread)
      bool refillRuns = false;

signals:
      void fillBuffers();

public:
      SamplePool();
      ~SamplePool();
      void fillSteamBuffers();
      bool streaming() { return _streaming; }
      Sample* getSamplePointer(QString filename);
      SampleStream* getSampleStream(Voice* v);
      void triggerBufferRefill();
      void deleteSampleStream(SampleStream *sampleStream);
      float fillPercentage() { return _fillPercentage; }
      unsigned int streamBufferSize() { return _streamBufferSize; }
      };

#endif // SAMPLEPOOL_H
