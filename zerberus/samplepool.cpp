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

QByteArray SamplePool::buf;

SamplePool::SamplePool()
      {

      }

 Sample* SamplePool::getSamplePointer(QString filename)
       {
       std::map<QString, Sample*>::iterator sampleIterator = filename2sample.find(filename);
       if (sampleIterator != filename2sample.end())
             return sampleIterator->second;

       QFile f(filename);
       if (!f.open(QIODevice::ReadOnly)) {
             printf("Sample::read: open <%s> failed\n", qPrintable(filename));
             return 0;
             }
       buf = f.readAll();

      AudioFile a;
      if (!a.open(buf)) {
            printf("open <%s> failed: %s\n", qPrintable(filename), a.error());
            return 0;
            }

      int channel = a.channels();
      int frames  = a.frames();
      int sr      = a.samplerate();

      short* data = new short[frames * channel];
      Sample* sa  = new Sample(channel, data, frames, sr);
      sa->setLoopStart(a.loopStart());
      sa->setLoopEnd(a.loopEnd());
      sa->setLoopMode(a.loopMode());

      if (frames != a.read(data, frames)) {
            qDebug("Sample read failed: %s\n", a.error());
            delete sa;
            sa = 0;
            }

      filename2sample.insert(std::pair<QString, Sample*>(filename, sa));

      return sa;
      }
