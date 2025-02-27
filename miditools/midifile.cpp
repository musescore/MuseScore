//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2007-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <QFile>
#include "midifile.h"

//---------------------------------------------------------
//   MidiFile
//---------------------------------------------------------

MidiFile::MidiFile()
      {
      fp               = 0;
      _format          = 1;
      _division        = 480;
      }

//---------------------------------------------------------
//   readMidi
//    return false on error
//---------------------------------------------------------

bool MidiFile::read(const QString& path)
      {
      QFile f(path);
      if (!f.open(QIODevice::ReadOnly))
            return false;
      return read(&f);
      }

bool MidiFile::read(QIODevice* f)
      {
      fp = f;
      _tracks.clear();
      curPos    = 0;

      char tmp[4];

      read(tmp, 4);
      int len = readLong();
      if (memcmp(tmp, "MThd", 4) || len < 6)
            throw(QString("bad midifile: MThd expected"));

      _format     = readShort();
      int ntracks = readShort();
      _division   = readShort();

      if (_division < 0)
            _division = (-(_division/256)) * (_division & 0xff);
      if (len > 6)
            skip(len-6); // skip the excess

      switch (_format) {
            case 0:
                  if (readTrack())
                        return false;
                  break;
            case 1:
                  for (int i = 0; i < ntracks; i++) {
                        if (readTrack())
                              return false;
                        }
                  break;
            default:
                  throw(QString("midi file format %1 not implemented").arg(_format));
            }
      return true;
      }

//---------------------------------------------------------
//   readTrack
//    return true on error
//---------------------------------------------------------

bool MidiFile::readTrack()
      {
      char tmp[4];
      read(tmp, 4);
      if (memcmp(tmp, "MTrk", 4))
            throw(QString("bad midifile: MTrk expected"));
      int len       = readLong();       // len
      qint64 endPos = curPos + len;
      status        = -1;
      sstatus       = -1;  // running status, will not be reset on meta or sysex
      click         =  0;
      MidiTrack* track  = new MidiTrack(this);
      _tracks.push_back(track);

      for (;;) {
            MidiEvent event;
            int rv = readEvent(&event);
            if (rv == 0)
                  track->events().insert(std::pair<int, MidiEvent>(click, event));
            else if (rv == 2)
                  break;
            }
      if (curPos != endPos) {
            qDebug("bad track len: %lld != %lld, %lld bytes too much\n", endPos, curPos, endPos - curPos);
            if (curPos < endPos) {
                  qDebug("  skip %lld\n", endPos-curPos);
                  skip(endPos - curPos);
                  }
            }
      return false;
      }

/*---------------------------------------------------------
 *    read
 *    return false on error
 *---------------------------------------------------------*/

void MidiFile::read(void* p, qint64 len)
      {
      curPos += len;
      qint64 rv = fp->read((char*)p, len);
      if (rv != len)
            throw(QString("bad midifile: unexpected EOF"));
      }

//---------------------------------------------------------
//   readShort
//---------------------------------------------------------

int MidiFile::readShort()
      {
      char c;
      int val = 0;
      for (int i = 0; i < 2; ++i) {
            fp->getChar(&c);
            val <<= 8;
            val += (c & 0xff);
            }
      return val;
      }

//---------------------------------------------------------
//   readLong
//   writeLong
//---------------------------------------------------------

int MidiFile::readLong()
      {
      char c;
      int val = 0;
      for (int i = 0; i < 4; ++i) {
            fp->getChar(&c);
            val <<= 8;
            val += (c & 0xff);
            }
      return val;
      }

/*---------------------------------------------------------
 *    skip
 *    This is meant for skipping a few bytes in a
 *    file or fifo.
 *---------------------------------------------------------*/

void MidiFile::skip(qint64 len)
      {
      if (len <= 0)
            return;
#if (!defined (_MSCVER) && !defined (_MSC_VER))
      char tmp[len];
#else
      // MSVC does not support VLA. Replace with std::vector. If profiling determines that the
      //    heap allocation is slow, an optimization might be used.
      std::vector<char> buffer(len);
      char *tmp = buffer.data();
#endif
      read(tmp, len);
      }

/*---------------------------------------------------------
 *    getvl
 *    Read variable-length number (7 bits per byte, MSB first)
 *---------------------------------------------------------*/

int MidiFile::getvl()
      {
      int l = 0;
      for (int i = 0; i < 16; i++) {
            uchar c;
            read(&c, 1);
            l += (c & 0x7f);
            if (!(c & 0x80)) {
                  return l;
                  }
            l <<= 7;
            }
      return -1;
      }

//---------------------------------------------------------
//   MidiTrack
//---------------------------------------------------------

MidiTrack::MidiTrack(MidiFile* f)
      {
      mf          = f;
//      _outChannel = -1;
//      _outPort    = -1;
//      _drumTrack  = false;
//      _hasKey     = false;
//      _staffIdx   = -1;
      }

//---------------------------------------------------------
//   readEvent
//    return 0 - valid event
//           1 - skip event
//           2 - EOT
//---------------------------------------------------------

int MidiFile::readEvent(MidiEvent* event)
      {
      uchar me, a, b;

      int nclick = getvl();
      if (nclick == -1)
            throw(QString("readEvent: error 1(getvl)"));
      click += nclick;
      for (;;) {
            read(&me, 1);
            if (me >= 0xf1 && me <= 0xfe && me != 0xf7)
                  qDebug("Midi: Unknown Message 0x%02x", me & 0xff);
            else
                  break;
            }

      int dataLen;

      if (me == 0xf0 || me == 0xf7) {
            status  = -1;                  // no running status
            int len = getvl();
            if (len == -1)
                  throw(QString("readEvent: error 3"));
            dataLen = len;
            std::vector<unsigned char> data(len + 1);
            read(data.data(), len);
            if (data[len - 1] != 0xf7) {
                  qDebug("SYSEX does not end with 0xf7!");
                  // more to come?
                  }
            else
                  dataLen--;      // don't count 0xf7
#if 0
            event->setType(MidiEventType::SYSEX);
            event->setData(data);
            event->setLen(dataLen);
            event->setOntime(click);
#endif
            return 1;
            }
      else if (me == 0xff) { // MidiEventType::META) {
            status = -1;                  // no running status
            uchar type;
            read(&type, 1);
            dataLen = getvl();                // read len
            if (dataLen == -1)
                  throw(QString("readEvent: error 6"));
            std::vector<unsigned char> data(dataLen + 1);
            if (dataLen)
                  read(data.data(), dataLen);

            if (type == META_TEMPO) {
                  unsigned tempo = data[2] + (data[1] << 8) + (data[0] << 16);
                  double t = 1000000.0 / double(tempo);
                  _tempoMap.insert(std::pair<const int, qreal>(click, t));
                  }
//else
//printf("META %02x\n", type);
            if (type == META_EOT)
                  return 2;
            return 1;
            }

      if (me & 0x80) {                     // status byte
            status   = me;
            sstatus  = status;
            read(&a, 1);
            }
      else {
            if (status == -1) {
                  qDebug("readEvent: no running status, read 0x%02x", me);
                  qDebug("sstatus ist 0x%02x", sstatus);
                  if (sstatus == -1)
                        return 0;
                  status = sstatus;
                  }
            a = me;
            }
      int channel = status & 0x0f;
      b           = 0;
      MidiEventType t = MidiEventType(status & 0xf0);
      switch (t) {
            case MidiEventType::NOTEOFF:
            case MidiEventType::NOTEON:
            case MidiEventType::POLYAFTER:
            case MidiEventType::CONTROLLER:        // controller
            case MidiEventType::PITCHBEND:        // pitch bend
                  read(&b, 1);
            default:
                  break;
            }
      switch (t) {
            case MidiEventType::NOTEOFF:
            case MidiEventType::NOTEON:
            case MidiEventType::CONTROLLER:
            case MidiEventType::PITCHBEND:
            case MidiEventType::POLYAFTER:
                  event->set(t, channel, a, b);
                  break;
            case MidiEventType::PROGRAM:
            case MidiEventType::AFTERTOUCH:
                  event->set(t, channel, a, 0);
                  break;
            default:          // f1 f2 f3 f4 f5 f6 f7 f8 f9
                  throw(QString("BAD STATUS")); // 0x%02x, me 0x%02x", status, me);
            }

      if ((a & 0x80) || (b & 0x80)) {
            qDebug("8't bit in data set(%02x %02x): tick %d read 0x%02x  status:0x%02x",
              a & 0xff, b & 0xff, click, me, status);
            qDebug("readEvent: error 16");
            if (b & 0x80) {
                  // Try to fix: interpret as channel byte
                  status   = b;
                  sstatus  = status;
                  return 0;
                  }
            throw(QString("readEvent: error 16"));
            }
      return 0;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

bool MidiFile::write(const QString& path)
      {
      QFile f(path);
      if (!f.open(QIODevice::WriteOnly))
            return false;
      return write(&f);
      }

bool MidiFile::write(QIODevice* f)
      {
      fp = f;
      write("MThd", 4);
      writeLong(6);                 // header len
      writeShort(_format);          // format
      writeShort(_tracks.size());
      writeShort(_division);
      foreach (const MidiTrack* t, _tracks) {
            if (writeTrack(t))
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void MidiFile::writeEvent(const MidiEvent& event)
      {
      switch (event.type()) {
            case MidiEventType::NOTEON:
                  writeStatus(event.type(), event.channel());
                  put(event.dataA() & 0x7f);
                  put(event.dataB() & 0x7f);
                  break;

            case MidiEventType::NOTEOFF:
                  writeStatus(event.type(), event.channel());
                  put(event.dataA() & 0x7f);
                  put(event.dataB() & 0x7f);
                  break;

            case MidiEventType::CONTROLLER:
                  writeStatus(event.type(), event.channel());
                  put(event.dataA() & 0x7f);
                  put(event.dataB() & 0x7f);
                  break;

            case MidiEventType::PROGRAM:
                  writeStatus(event.type(), event.channel());
                  put(event.dataA() & 0x7f);
                  break;
#if 0
            case MidieEventType::META:
                  put(MidiEventType::META);
                  put(event.metaType());
                  // Don't null terminate text meta events
                  if (event.metaType() >= 0x1 && event.metaType() <= 0x14) {
                        putvl(event.len() - 1);
                        write(event.edata(), event.len() - 1);
                        }
                  else {
                        putvl(event.len());
                        write(event.edata(), event.len());
                        }
                  resetRunningStatus();     // really ?!
                  break;

            case MidiEventType::SYSEX:
                  put(MidiEventType::SYSEX);
                  putvl(event.len() + 1);  // including 0xf7
                  write(event.edata(), event.len());
                  put(MidiEventTppe::ENDSYSEX);
                  resetRunningStatus();
                  break;
#endif
            default:
//fprintf(stderr, "unsupported\n");
                  break;
            }
      }

//---------------------------------------------------------
//   writeTrack
//---------------------------------------------------------

bool MidiFile::writeTrack(const MidiTrack* t)
      {
      write("MTrk", 4);
      qint64 lenpos = fp->pos();
      writeLong(0);                 // dummy len

      status   = -1;
      int tick = 0;
      for (auto i : t->events()) {
            int ntick = i.first;
            putvl(ntick - tick);    // write tick delta
            writeEvent(i.second);
            tick = ntick;
            }

      //---------------------------------------------------
      //    write "End Of Track" Meta
      //    write Track Len
      //

      putvl(1);
      put(0xff);        // Meta
      put(0x2f);        // EOT
      putvl(0);         // len 0
      qint64 endpos = fp->pos();
      fp->seek(lenpos);
      writeLong(endpos-lenpos-4);   // tracklen
      fp->seek(endpos);
      return false;
      }

//---------------------------------------------------------
//   writeStatus
//---------------------------------------------------------

void MidiFile::writeStatus(MidiEventType st, int c)
      {
      uchar nstat = uchar(st) | (c & 0xf);
      //
      //  running status; except for Sysex- and Meta Events
      //
      if ((((nstat & 0xf0) != 0xf0) && (nstat != status))) {
            status = nstat;
            put(nstat);
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

bool MidiFile::write(const void* p, qint64 len)
      {
      qint64 rv = fp->write((char*)p, len);
      if (rv == len)
            return false;
      qDebug("write midifile failed: %s", fp->errorString().toLatin1().data());
      return true;
      }

//---------------------------------------------------------
//   writeShort
//---------------------------------------------------------

void MidiFile::writeShort(int i)
      {
      fp->putChar(i >> 8);
      fp->putChar(i);
      }

//---------------------------------------------------------
//   writeLong
//---------------------------------------------------------

void MidiFile::writeLong(int i)
      {
      fp->putChar(i >> 24);
      fp->putChar(i >> 16);
      fp->putChar(i >> 8);
      fp->putChar(i);
      }

/*---------------------------------------------------------
 *    putvl
 *    Write variable-length number (7 bits per byte, MSB first)
 *---------------------------------------------------------*/

void MidiFile::putvl(unsigned val)
      {
      unsigned long buf = val & 0x7f;
      while ((val >>= 7) > 0) {
            buf <<= 8;
            buf |= 0x80;
            buf += (val & 0x7f);
            }
      for (;;) {
            put(buf);
            if (buf & 0x80)
                  buf >>= 8;
            else
                  break;
            }
      }


