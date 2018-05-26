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
//   SfzControl
//---------------------------------------------------------

struct SfzControl {
      QString defaultPath;
      int octave_offset;
      int note_offset;
      int set_cc[128];
      std::map<QString, QString> defines;
      void init();
      };

void SfzControl::init()
      {
      defaultPath.clear();
      octave_offset = 0;
      note_offset = 0;
      }

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
      long long loopStart, loopEnd; // [0, 4Gb) or [0, 4294967295]
      Trigger trigger;
      LoopMode loop_mode;
      OffMode off_mode;
      std::map<int, double> gain_oncc;
      float delay; //seconds
      int pan;    // [-100, 100]
      long long offset; // [0, 4Gb) or [0, 4294967295]
      float group_volume; // [-144 to 6] (dB)

      void init(const QString&);
      bool isEmpty() const { return sample.isEmpty(); }
      int readKey(const QString&, SfzControl c) const;
      void readOp(const QString& token, const QString& data, SfzControl& c);
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
      gain_oncc.clear();
      delay = 0.0f;
      pan = 0;
      offset = 0;
      group_volume = 0.0f;
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
      z->offset       = offset;
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
      z->gainOnCC     = gain_oncc;
      z->delay        = delay * 1000; //convert seconds from sfz to ms for computations
      z->pan          = pan;
      z->group_volume = pow(10.0, group_volume / 20.0); //dB -> volume multiplier
      }

//---------------------------------------------------------
//   readKey
//---------------------------------------------------------

int SfzRegion::readKey(const QString& s, SfzControl c) const
      {
      bool ok;
      int i = s.toInt(&ok);
      if (ok) {
            i += c.octave_offset * 12;
            i += c.note_offset;
            return i;
            }
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
      i += c.octave_offset * 12;
      i += c.note_offset;
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
            //qDebug("Sample Loop - start %ll, end %ll, mode %d", z->sample->loopStart(), z->sample->loopEnd(), z->sample->loopMode());
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
//   readLongLong
//---------------------------------------------------------

static void readLongLong(const QString& data, long long& val)
      {
      bool ok;
      float d = data.toLongLong(&ok);
      if (ok)
            val = d;
      }

//---------------------------------------------------------
//   readFloat
//---------------------------------------------------------

static void readFloat(const QString& data, float& val)
      {
      bool ok;
      float d = data.toFloat(&ok);
      if (ok)
            val = d;
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
//   readOp
//---------------------------------------------------------

void SfzRegion::readOp(const QString& b, const QString& data, SfzControl &c)
      {
      QString opcode           = QString(b);
      QString opcode_data_full = QString(data);

      for(auto define : c.defines) {
            opcode.replace(define.first, define.second);
            opcode_data_full.replace(define.first, define.second);
            }

      QStringList splitData = opcode_data_full.split(" "); // no spaces in opcode values except for sample definition
      QString opcode_data = splitData[0];

      int i = opcode_data.toInt();

      if (opcode == "amp_veltrack")
            readDouble(opcode_data, &amp_veltrack);
      else if (opcode == "ampeg_delay")
            readDouble(opcode_data, &ampeg_delay);
      else if (opcode == "ampeg_start")
            readDouble(opcode_data, &ampeg_start);
      else if (opcode == "ampeg_attack")
            readDouble(opcode_data, &ampeg_attack);
      else if (opcode == "ampeg_hold")
            readDouble(opcode_data, &ampeg_hold);
      else if (opcode == "ampeg_decay")
            readDouble(opcode_data, &ampeg_decay);
      else if (opcode == "ampeg_sustain")
            readDouble(opcode_data, &ampeg_sustain);
      else if (opcode == "ampeg_release")
            readDouble(opcode_data, &ampeg_release);
      else if (opcode == "ampeg_vel2delay")
            readDouble(opcode_data, &ampeg_vel2delay);
      else if (opcode == "ampeg_vel2attack")
            readDouble(opcode_data, &ampeg_vel2attack);
      else if (opcode == "ampeg_vel2hold")
            readDouble(opcode_data, &ampeg_vel2hold);
      else if (opcode == "ampeg_vel2decay")
            readDouble(opcode_data, &ampeg_vel2decay);
      else if (opcode == "ampeg_vel2sustain")
            readDouble(opcode_data, &ampeg_vel2sustain);
      else if (opcode == "ampeg_vel2release")
            readDouble(opcode_data, &ampeg_vel2release);
      else if (opcode == "sample") {
            sample = path + "/" + c.defaultPath + "/" + opcode_data_full; // spaces are allowed
            sample.replace("\\", "/");
            }
      else if (opcode == "default_path") {
            c.defaultPath = opcode_data_full;
            }
      else if (opcode == "key") {
            lokey = readKey(opcode_data, c);
            hikey = lokey;
            pitch_keycenter = lokey;
            }
      else if (opcode == "pitch_keytrack")
            readDouble(opcode_data, &pitch_keytrack);
      else if (opcode == "trigger") {
            if (opcode_data == "attack")
                  trigger = Trigger::ATTACK;
            else if (opcode_data == "release")
                  trigger = Trigger::RELEASE;
            else if (opcode_data == "first")
                  trigger = Trigger::FIRST;
            else if (opcode_data == "legato")
                  trigger = Trigger::LEGATO;
            else
                  qDebug("SfzRegion: bad trigger value: %s", qPrintable(opcode_data));
            }
      else if (opcode == "loop_mode") {
            if (opcode_data == "no_loop")
                  loop_mode = LoopMode::NO_LOOP;
            else if (opcode_data == "one_shot")
                  loop_mode = LoopMode::ONE_SHOT;
            else if (opcode_data == "loop_continuous")
                  loop_mode = LoopMode::CONTINUOUS;
            else if (opcode_data == "loop_sustain")
                  loop_mode = LoopMode::SUSTAIN;
            if (loop_mode != LoopMode::ONE_SHOT)
                  qDebug("SfzRegion: loop_mode <%s>", qPrintable(opcode_data));
            }
      else if(opcode == "loop_start")
            readLongLong(opcode_data, loopStart);
      else if(opcode == "loop_end")
            readLongLong(opcode_data, loopEnd);
      else if (opcode.startsWith("on_locc")) {
            int idx = b.mid(7).toInt();
            if (idx >= 0 && idx < 128)
                  on_locc[idx] = i;
            }
      else if (opcode.startsWith("on_hicc")) {
            int idx = b.mid(7).toInt();
            if (idx >= 0 && idx < 128)
                  on_hicc[idx] = i;
            }
      else if (opcode.startsWith("locc")) {
            int idx = b.mid(4).toInt();
            if (!use_cc)
                  use_cc = i != 0;
            if (idx >= 0 && idx < 128)
                  locc[idx] = i;
            }
      else if (opcode.startsWith("hicc")) {
            int idx = b.mid(4).toInt();
            if (!use_cc)
                  use_cc = i != 127;
            if (idx >= 0 && idx < 128)
                  hicc[idx] = i;
            }
      else if (opcode.startsWith("set_cc")) {
            int idx = b.mid(6).toInt();
            if (idx >= 0 && idx < 128)
                  c.set_cc[idx] = i;
            }
      else if (opcode == "off_mode") {
            if (opcode_data == "fast")
                  off_mode = OffMode::FAST;
            else if (opcode_data == "normal")
                  off_mode = OffMode::NORMAL;
            }
      else if (opcode.startsWith("gain_cc")) {
            int idx = b.mid(7).toInt();
            double v;
            if (idx >= 0 && idx < 128) {
                  readDouble(opcode_data, &v);
                  gain_oncc.insert(std::pair<int, double>(idx, v));
                  }
            }
      else if (opcode.startsWith("gain_oncc")) {
            int idx = b.mid(9).toInt();
            double v;
            if (idx >= 0 && idx < 128) {
                  readDouble(opcode_data, &v);
                  gain_oncc.insert(std::pair<int, double>(idx, v));
                  }
            }
      else if (opcode == "tune")
            tune = i;
      else if (opcode == "rt_decay")
            readDouble(opcode_data, &rt_decay);
      else if (opcode == "hirand")
            readDouble(opcode_data, &hirand);
      else if (opcode == "lorand")
            readDouble(opcode_data, &lorand);
      else if (opcode == "volume")
            readDouble(opcode_data, &volume);
      else if (opcode == "pitch_keycenter")
            pitch_keycenter = readKey(opcode_data ,c);
      else if (opcode == "lokey")
            lokey = readKey(opcode_data, c);
      else if (opcode == "hikey")
            hikey = readKey(opcode_data, c);
      else if (opcode == "lovel")
            lovel = i;
      else if (opcode == "hivel")
            hivel = i;
      else if (opcode == "off_by")
            off_by = i;
      else if (opcode == "group")
            group = i;
      else if (opcode == "lochan")
            lochan = i;
      else if (opcode == "hichan")
            hichan = i;
      else if (opcode == "note_offset")
            c.note_offset = i;
      else if (opcode == "octave_offset")
            c.octave_offset = i;
      else if (opcode == "seq_length")
            seq_length = i;
      else if (opcode == "seq_position")
            seq_position = i;
      else if (opcode == "transpose")
            transpose = i;
      else if (opcode == "delay")
            readFloat(opcode_data, delay);
      else if (opcode == "pan")
            pan = i;
      else if (opcode == "offset")
            readLongLong(opcode_data, offset);
      else if (opcode == "group_volume")
            readFloat(opcode_data, group_volume);
      else
            qDebug("SfzRegion: unknown opcode <%s>", qPrintable(opcode));
      }

QStringList readFile(QString const fn)
      {
            QFile f(fn);
            QStringList list;
            if (!f.open(QIODevice::ReadOnly)) {
                  qDebug("ZInstrument: cannot load %s", qPrintable(fn));
                  return list;
                  }
            while (!f.atEnd())
                  list.append(QString(f.readLine().simplified()));
            return list;
      }

//---------------------------------------------------------
//   loadSfz
//---------------------------------------------------------

bool ZInstrument::loadSfz(const QString& s)
      {
      _program = 0;
      QFileInfo fi(s);
      QString path = fi.absolutePath();

      QStringList fileContents = readFile(s);

      if (fileContents.empty()) {
            return false;
            }

      SfzControl c;
      c.init();
      c.defines.clear();
      for (int i = 0;i < 128; i++)
            c.set_cc[i] = -1;

      int idx = 0;
      bool inBlockComment = false;
      // preprocessor
      while(idx < fileContents.size()) {
            QRegularExpression findWithSpaces("\"(.+)\"");
            QRegularExpression comment("//.*$");
            QRegularExpression trailingSpacesOrTab("^[\\s\\t]*");
            QRegularExpressionMatch foundWithSpaces;
            QString curLine = fileContents[idx];
            QString curLineCopy = curLine;
            bool nextIsImportant = false;
            int idxBlockComment = 0;
            int from = 0;

            for (QChar chr : curLineCopy) {
                  bool terminated = false;

                  if (nextIsImportant) {
                        nextIsImportant = false;
                        if (inBlockComment && chr == '/') { // found block end
                              inBlockComment = false;
                              terminated = true;
                              curLine.remove(from, idxBlockComment - from + 1);
                              idxBlockComment = from - 1;
                              }
                        else if (!inBlockComment && chr == '*') { // found block start
                              inBlockComment = true;
                              terminated = true;
                              from = idxBlockComment - 1;
                              }
                        }

                  if (!terminated && inBlockComment && chr == '*')
                        nextIsImportant = true;
                  else if (!terminated && !inBlockComment && chr == '/')
                        nextIsImportant = true;

                  idxBlockComment++;
                  }

            if (inBlockComment)
                  curLine.remove(from, curLine.size() - from);

            curLine = curLine.remove(comment);
            curLine.remove(trailingSpacesOrTab);
            fileContents[idx] = curLine;

            if (curLine.startsWith("#define")) {
                  QStringList define = curLine.split(" ");
                  foundWithSpaces = findWithSpaces.match(curLine);
                  if (define.size() == 3)
                        c.defines.insert(std::pair<QString, QString>(define[1], define[2]));
                  else if(foundWithSpaces.hasMatch())
                        c.defines.insert(std::pair<QString, QString>(define[1], foundWithSpaces.captured(1)));
                  fileContents.removeAt(idx);
                  }
            else if (curLine.startsWith("#include")) {
                  foundWithSpaces = findWithSpaces.match(curLine);
                  if (foundWithSpaces.hasMatch()) {
                        QString newFilename = foundWithSpaces.captured(1);

                        for(auto define : c.defines) {
                              newFilename.replace(define.first, define.second);
                              }

                        QStringList newFileContents = readFile(path + "/" + newFilename);
                        if (newFileContents.empty())
                              return false;

                        int offset = 1;
                        for (QString newFileLine : newFileContents) {
                              fileContents.insert(idx+offset, newFileLine);
                              offset++;
                              }

                        fileContents.removeAt(idx);
                        }
                  }
            else if (curLine.isEmpty())
                  fileContents.removeAt(idx);
            else
                  idx++;
            }

      int total = fileContents.size();
      SfzRegion r;
      SfzRegion g;      // group
      SfzRegion glob;
      r.init(path);
      g.init(path);
      glob.init(path);

      bool groupMode = false;
      bool globMode = false;
      zerberus->setLoadProgress(0);

      for (int idx = 0; idx < fileContents.size(); idx++) {
            QString curLine = fileContents[idx];
            zerberus->setLoadProgress(((qreal) idx * 100) /  (qreal) total);

            if (zerberus->loadWasCanceled())
                  return false;
            if (curLine.startsWith("<global>")) {
                  if (!globMode && !groupMode && !r.isEmpty())
                        addRegion(r);
                  glob.init(path);
                  g.init(path); // global also resets group
                  r.init(path);
                  globMode = true;
                  }
            if (curLine.startsWith("<group>")) {
                  if (!groupMode && !globMode && !r.isEmpty())
                        addRegion(r);
                  g.init(path);
                  if (globMode) {
                        glob = r;
                        globMode = false;
                        }
                  else {
                        r = glob; // initialize group with global values
                        }
                  groupMode = true;
                  curLine = curLine.mid(7);
                  }
            else if (curLine.startsWith("<region>")) {
                  if (groupMode) {
                        g = r;
                        groupMode = false;
                        }
                  else if (globMode) {
                        glob = r;
                        g = glob;
                        globMode = false;
                        }
                  else {
                        if (!r.isEmpty())
                              addRegion(r);
                        r = g;  // initialize next region with group values
                        }
                  curLine = curLine.mid(8);
                  }
            else if (curLine.startsWith("<control>"))
                  c.init();

            QRegularExpression re("\\s?([\\w\\$]+)="); // defines often use the $-sign
            QRegularExpressionMatchIterator i = re.globalMatch(curLine);

            while (i.hasNext()) {
                  QRegularExpressionMatch match = i.next();
                  int si = match.capturedEnd();
                  int ei;
                  if (i.hasNext()) {
                        QRegularExpressionMatch nextMatch = i.peekNext();
                        ei = nextMatch.capturedStart();
                        }
                  else
                        ei = curLine.size();
                  QString s = curLine.mid(si, ei-si);
                  r.readOp(match.captured(1), s, c);
                  }
            }

      for (int i = 0; i < 128; i++)
            _setcc[i] = c.set_cc[i];

      zerberus->setLoadProgress(100);
      if (!groupMode && !globMode && !r.isEmpty())
            addRegion(r);
      return true;
      }

