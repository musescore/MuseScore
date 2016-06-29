//=============================================================================
//  Zerberos
//  Zample player
//
//  Copyright (C) 2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <stdio.h>
#include <math.h>
#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <sndfile.h>

#include "libmscore/xml.h"

#include "instrument.h"
#include "zone.h"
#include "sample.h"
#include "zerberus.h"

//---------------------------------------------------------
//   SfzRegion
//---------------------------------------------------------

struct SfzRegion {
      QString path;
      double amp_veltrack;
      // amp envelope all in seconds
      // but the level ones
      // they are in percent
      double ampeg_delay;
      double ampeg_start; // level
      double ampeg_attack;
      double ampeg_hold;
      double ampeg_decay;
      double ampeg_sustain; // level
      double ampeg_release;
      double rt_decay;
      double ampeg_vel2delay;
      double ampeg_vel2attack;
      double ampeg_vel2hold;
      double ampeg_vel2decay;
      double ampeg_vel2sustain;
      double ampeg_vel2release;
      QString sample;
      int lochan, hichan;
      int lokey, hikey, lovel, hivel, pitch_keycenter;
      double hirand, lorand;
      int on_locc[128];
      int on_hicc[128];
      int locc[128];
      int hicc[128];
      bool use_cc;
      int off_by;
      int group;
      int seq_length, seq_position;
      double volume;
      int octave_offset, note_offset;
      int tune, transpose;
      double pitch_keytrack;
      int loopStart, loopEnd;
      Trigger trigger;
      LoopMode loop_mode;
      OffMode off_mode;

      void init(const QString&);
      bool isEmpty() const { return sample.isEmpty(); }
      int readKey(const QString&) const;
      void readOp(const QString& token, const QString& data);
      void setZone(Zone*) const;
      };

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void SfzRegion::init(const QString& _path)
      {
      path            = _path;
      lochan          = 1;
      hichan          = 16;
      amp_veltrack    = 100;
      ampeg_delay     = 0.0;
      ampeg_start     = 0.0; //percent
      ampeg_attack    = 0.001;
      ampeg_hold      = 0.0;
      ampeg_decay     = 0.0;
      ampeg_sustain   = 100.0; // percent
      ampeg_release   = 0.200;  // in sec
      ampeg_vel2delay    = 0.0;
      ampeg_vel2attack   = 0.0;
      ampeg_vel2hold     = 0.0;
      ampeg_vel2decay    = 0.0;
      ampeg_vel2sustain  = 0.0;
      ampeg_vel2release  = 0.0;
      rt_decay        = 0.0;  // dB /sec
      lokey           = 0;
      hikey           = 127;
      lovel           = 0;
      hivel           = 127;
      pitch_keycenter = 60;
      sample          = "";
      lorand          = 0.0;
      hirand          = 1.0;
      group           = 0;
      off_by          = 0;
      volume          = 1.0;
      note_offset     = 0;
      octave_offset   = 0;
      seq_length      = 1;
      seq_position    = 1;
      trigger         = Trigger::ATTACK;
      loop_mode       = LoopMode::CONTINUOUS;
      tune            = 0;
      transpose       = 0;
      pitch_keytrack  = 100.0;
      loopStart       = -1;
      loopEnd         = -1;
      for (int i = 0; i < 128; ++i) {
            on_locc[i] = -1;
            on_hicc[i] = -1;
            locc[i]    = 0;
            hicc[i]    = 127;
            }
      use_cc         = false;
      off_mode = OffMode::FAST;
      }

//---------------------------------------------------------
//   setZone
//    set Zone from sfzRegion
//---------------------------------------------------------

void SfzRegion::setZone(Zone* z) const
      {
      z->keyLo        = lokey;
      z->keyHi        = hikey;
      z->veloLo       = lovel;
      z->veloHi       = hivel;
      z->keyBase      = pitch_keycenter;
      z->offset       = 0;
      z->volume       = pow(10.0, volume / 20.0);
      z->ampVeltrack  = amp_veltrack;
      z->ampegAttack  = ampeg_attack * 1000;
      z->ampegDelay   = ampeg_delay * 1000;
      z->ampegStart   = ampeg_start / 100.0;
      z->ampegHold    = ampeg_hold * 1000;
      z->ampegDecay   = ampeg_decay * 1000;
      z->ampegSustain = ampeg_sustain / 100.0;
      z->ampegRelease = ampeg_release * 1000;
      // all vel2* but vel2sustain are time values in seconds
      z->ampegVel2Delay    = ampeg_vel2delay * 1000;
      z->ampegVel2Attack   = ampeg_vel2attack * 1000;
      z->ampegVel2Hold     = ampeg_vel2hold * 1000;
      z->ampegVel2Decay    = ampeg_vel2decay * 1000;
      z->ampegVel2Sustain  = ampeg_vel2sustain / 100; // level in percent
      z->ampegVel2Release  = ampeg_vel2release * 1000;
      z->seqPos       = seq_position - 1;
      z->seqLen       = seq_length - 1;
      z->seq          = 0;
      z->trigger      = trigger;
      z->loopMode     = loop_mode;
      z->tune         = tune + transpose * 100;
      z->pitchKeytrack = pitch_keytrack / (double) 100.0;
      z->rtDecay      = rt_decay;
      for (int i = 0; i < 128; ++i) {
            z->onLocc[i] = on_locc[i];
            z->onHicc[i] = on_hicc[i];
            z->locc[i]   = locc[i];
            z->hicc[i]   = hicc[i];
            }
      z->useCC        = use_cc;
      z->offMode      = off_mode;
      z->offBy        = off_by;
      z->loRand       = lorand;
      z->hiRand       = hirand;
      z->group        = group;
      z->loopEnd      = loopEnd;
      z->loopStart    = loopStart;
      if (note_offset || octave_offset) {
            qDebug("=========================offsets %d %d", note_offset, octave_offset);
            }
      }

//---------------------------------------------------------
//   readKey
//---------------------------------------------------------

int SfzRegion::readKey(const QString& s) const
      {
      bool ok;
      int i = s.toInt(&ok);
      if (ok)
            return i;
       switch (s[0].toLower().toLatin1()) {
            case 'c': i = 0; break;
            case 'd': i = 2; break;
            case 'e': i = 4; break;
            case 'f': i = 5; break;
            case 'g': i = 7; break;
            case 'a': i = 9; break;
            case 'b': i = 11; break;
            default:
                  qDebug("SfzRegion: Not a note: %s", qPrintable(s));
                  return 0;
            }
      int idx = 1;
      if (s[idx].toLatin1() == '#') {
            i++;
            ++idx;
            }
      else if (s[idx].toLower().toLatin1() == 'b') {
            i--;
            ++idx;
            }
      int octave = s.mid(idx).toInt(&ok);
      if (!ok) {
            qDebug("SfzRegion: Not a note: %s", qPrintable(s));
            return 0;
            }
      i += (octave + 1) * 12;
      return i;
      }

//---------------------------------------------------------
//   addRegion
//---------------------------------------------------------

void ZInstrument::addRegion(SfzRegion& r)
      {
      for (int i = 0; i < 128; ++i) {
            if (r.on_locc[i] != -1 || r.on_hicc[i] != -1) {
                  r.trigger = Trigger::CC;
                  break;
                  }
            }
      Zone* z = new Zone;
      z->sample = readSample(r.sample, 0);
      if (z->sample) {
            qDebug("Sample Loop - start %d, end %d, mode %d", z->sample->loopStart(), z->sample->loopEnd(), z->sample->loopMode());
            // if there is no opcode defining loop ranges, use sample definitions as fallback (according to spec)
            if (r.loopStart == -1)
                  r.loopStart = z->sample->loopStart();
            if (r.loopEnd == -1)
                  r.loopEnd = z->sample->loopEnd();
            }
      r.setZone(z);
      if (z->sample)
            addZone(z);
      }

//---------------------------------------------------------
//   readDouble
//---------------------------------------------------------

static void readDouble(const QString& data, double* val)
      {
      bool ok;
      double d = data.toDouble(&ok);
      if (ok)
            *val = d;
      }

//---------------------------------------------------------
//   readInt
//---------------------------------------------------------

static void readInt(const QString& data, int* val)
      {
      bool ok;
      int i = data.toInt(&ok);
      if (ok)
            *val = i;
      }

//---------------------------------------------------------
//   readOp
//---------------------------------------------------------

void SfzRegion::readOp(const QString& b, const QString& data)
      {
      int i = data.toInt();

      if (b == "amp_veltrack")
            readDouble(data, &amp_veltrack);
      else if (b == "ampeg_delay")
            readDouble(data, &ampeg_delay);
      else if (b == "ampeg_start")
            readDouble(data, &ampeg_start);
      else if (b == "ampeg_attack")
            readDouble(data, &ampeg_attack);
      else if (b == "ampeg_hold")
            readDouble(data, &ampeg_hold);
      else if (b == "ampeg_decay")
            readDouble(data, &ampeg_decay);
      else if (b == "ampeg_sustain")
            readDouble(data, &ampeg_sustain);
      else if (b == "ampeg_release")
            readDouble(data, &ampeg_release);
      else if (b == "ampeg_vel2delay")
            readDouble(data, &ampeg_vel2delay);
      else if (b == "ampeg_vel2attack")
            readDouble(data, &ampeg_vel2attack);
      else if (b == "ampeg_vel2hold")
            readDouble(data, &ampeg_vel2hold);
      else if (b == "ampeg_vel2decay")
            readDouble(data, &ampeg_vel2decay);
      else if (b == "ampeg_vel2sustain")
            readDouble(data, &ampeg_vel2sustain);
      else if (b == "ampeg_vel2release")
            readDouble(data, &ampeg_vel2release);
      else if (b == "sample") {
            sample = path + "/" + data;
            sample.replace("\\", "/");
            }
      else if (b == "key") {
            lokey = readKey(data);
            hikey = lokey;
            pitch_keycenter = lokey;
            }
      else if (b == "pitch_keytrack")
            readDouble(data, &pitch_keytrack);
      else if (b == "trigger") {
            if (data == "attack")
                  trigger = Trigger::ATTACK;
            else if (data == "release")
                  trigger = Trigger::RELEASE;
            else if (data == "first")
                  trigger = Trigger::FIRST;
            else if (data == "legato")
                  trigger = Trigger::LEGATO;
            else
                  qDebug("SfzRegion: bad trigger value: %s", qPrintable(data));
            }
      else if (b == "loop_mode") {
            if (data == "no_loop")
                  loop_mode = LoopMode::NO_LOOP;
            else if (data == "one_shot")
                  loop_mode = LoopMode::ONE_SHOT;
            else if (data == "loop_continuous")
                  loop_mode = LoopMode::CONTINUOUS;
            else if (data == "loop_sustain")
                  loop_mode = LoopMode::SUSTAIN;
            if (loop_mode != LoopMode::ONE_SHOT)
                  qDebug("SfzRegion: loop_mode <%s>", qPrintable(data));
            }
      else if(b == "loop_start")
            readInt(data, &loopStart);
      else if(b == "loop_end")
            readInt(data, &loopEnd);
      else if (b.startsWith("on_locc")) {
            int idx = b.mid(7).toInt();
            if (idx >= 0 && idx < 128)
                  on_locc[idx] = i;
            }
      else if (b.startsWith("on_hicc")) {
            int idx = b.mid(7).toInt();
            if (idx >= 0 && idx < 128)
                  on_hicc[idx] = i;
            }
      else if (b.startsWith("locc")) {
            int idx = b.mid(4).toInt();
            if (!use_cc)
                  use_cc = i != 0;
            if (idx >= 0 && idx < 128)
                  locc[idx] = i;
            }
      else if (b.startsWith("hicc")) {
            int idx = b.mid(4).toInt();
            if (!use_cc)
                  use_cc = i != 127;
            if (idx >= 0 && idx < 128)
                  hicc[idx] = i;
            }
      else if (b == "off_mode") {
            if (data == "fast")
                  off_mode = OffMode::FAST;
            else if (data == "normal")
                  off_mode = OffMode::NORMAL;
            }
      else if (b == "tune")
            tune = i;
      else if (b == "rt_decay")
            readDouble(data, &rt_decay);
      else if (b == "hirand")
            readDouble(data, &hirand);
      else if (b == "lorand")
            readDouble(data, &lorand);
      else if (b == "volume")
            readDouble(data, &volume);
      else if (b == "pitch_keycenter")
            pitch_keycenter = readKey(data);
      else if (b == "lokey")
            lokey = readKey(data);
      else if (b == "hikey")
            hikey = readKey(data);
      else if (b == "lovel")
            lovel = i;
      else if (b == "hivel")
            hivel = i;
      else if (b == "off_by")
            off_by = i;
      else if (b == "group")
            group = i;
      else if (b == "lochan")
            lochan = i;
      else if (b == "hichan")
            hichan = i;
      else if (b == "note_offset")
            note_offset = i;
      else if (b == "octave_offset")
            octave_offset = i;
      else if (b == "seq_length")
            seq_length = i;
      else if (b == "seq_position")
            seq_position = i;
      else if (b == "transpose")
            transpose = i;
      else
            qDebug("SfzRegion: unknown opcode <%s>", qPrintable(b));
      }

//---------------------------------------------------------
//   loadSfz
//---------------------------------------------------------

bool ZInstrument::loadSfz(const QString& s)
      {
      _program = 0;
      QFile f(s);
      if (!f.open(QIODevice::ReadOnly)) {
            qDebug("ZInstrument: cannot load %s", qPrintable(s));
            return false;
            }
      QFileInfo fi(f);
      QString path = fi.absolutePath();
      qint64 total = fi.size();

      QString sample;

      SfzRegion r;
      SfzRegion g;      // group
      r.init(path);
      g.init(path);

      bool groupMode = false;
      zerberus->setLoadProgress(0);

      while (!f.atEnd()) {
            QByteArray ba = f.readLine();
            zerberus->setLoadProgress(((qreal)f.pos() * 100) / total);
            ba = ba.simplified();

            if (ba.isEmpty() || ba.startsWith("//"))
                  continue;
            if (zerberus->loadWasCanceled())
                  return false;
            if (ba.startsWith("<group>")) {
                  if (!groupMode && !r.isEmpty())
                        addRegion(r);
                  g.init(path);
                  r.init(path);
                  groupMode = true;
                  ba = ba.mid(7);
                  }
            else if (ba.startsWith("<region>")) {
                  if (groupMode) {
                        g = r;
                        groupMode = false;
                        }
                  else {
                        if (!r.isEmpty())
                              addRegion(r);
                        r = g;  // initialize next region with group values
                        }
                  ba = ba.mid(8);
                  }
            QRegularExpression re("\\s?(\\w+)=");
            QRegularExpressionMatchIterator i = re.globalMatch(ba);

            while (i.hasNext()) {
                  QRegularExpressionMatch match = i.next();
                  int si = match.capturedEnd();
                  int ei;
                  if (i.hasNext()) {
                        QRegularExpressionMatch nextMatch = i.peekNext();
                        ei = nextMatch.capturedStart();
                        }
                  else
                        ei = ba.size();
                  QString s = ba.mid(si, ei-si);
                  r.readOp(match.captured(1), s);
                  }
            }
      zerberus->setLoadProgress(100);
      if (!groupMode && !r.isEmpty())
            addRegion(r);
      return true;
      }

