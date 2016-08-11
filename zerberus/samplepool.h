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

#define STREAM_BUFFER_SIZE 1024

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
      unsigned int readPos = 0;
      unsigned int writePos = 0;
      sf_count_t fileReadPos;
      Voice* voice;
      SF_INFO info;
      SNDFILE *sf;

public:
      SampleStream(Voice *v, SamplePool* sp);
      ~SampleStream();
      void updateLoop();
      short getData(int pos);
      };

class SamplePool
      {
      std::map<QString, Sample*> filename2sample;
      std::vector<SampleStream *> streams;
      bool _streaming = false;

public:
      SamplePool();
      void fillSteamBuffers();
      bool streaming() { return _streaming; }
      Sample* getSamplePointer(QString filename);
      SampleStream* getSampleStream(Voice* v);
      void deleteSampleStream(SampleStream *sampleStream);
      };

#endif // SAMPLEPOOL_H
