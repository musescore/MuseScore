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

struct SampleLoop
      {
      Sample* sample;
      unsigned int loopStart = -1;
      unsigned int loopEnd = -1;
      LoopMode loopmode;
      };

struct SampleStream
      {
      SampleLoop* sampleLoop;
      float buffer[STREAM_BUFFER_SIZE];
      unsigned int readPos = 0;
      unsigned int writePos = 0;
      unsigned int fileReadPos = 0;
      bool looping = false;
      };

class SamplePool
      {
      std::map<QString, Sample*> filename2sample;
      std::vector<SampleStream *> streams;
      bool streaming;

      static QByteArray buf;  // used during read of Sample
public:
      SamplePool();
      void fillSteamBuffers();
      Sample* getSamplePointer(QString filename);
      SampleStream* getSampleStream(QString filename);
      };

#endif // SAMPLEPOOL_H
