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

#include "midifile.h"

namespace Ms {

static const uchar gmOnMsg[] = {
      0x7e,       // Non-Real Time header
      0x7f,       // ID of target device (7f = all devices)
      0x09,
      0x01
      };
static const uchar gsOnMsg[] = {
      0x41,       // roland id
      0x10,       // Id of target device (default = 10h for roland)
      0x42,       // model id (42h = gs devices)
      0x12,       // command id (12h = data set)
      0x40,       // address & value
      0x00,
      0x7f,
      0x00,
      0x41        // checksum?
      };
static const uchar xgOnMsg[] = {
      0x43,       // yamaha id
      0x10,       // device number (0)
      0x4c,       // model id
      0x00,       // address (high, mid, low)
      0x00,
      0x7e,
      0x00        // data
      };

const int gmOnMsgLen = sizeof(gmOnMsg);
const int gsOnMsgLen = sizeof(gsOnMsg);
const int xgOnMsgLen = sizeof(xgOnMsg);

//---------------------------------------------------------
//   MidiFile
//---------------------------------------------------------

MidiFile::MidiFile()
      {
      fp               = 0;
      _division        = 0;
      _isDivisionInTps = false;
      _format          = 1;
      _midiType        = MidiType::UNKNOWN;
      _noRunningStatus = false;

      status           = 0;
      sstatus          = 0;
      click            = 0;
      curPos           = 0;
      }

//---------------------------------------------------------
//   write
//    returns true on error
//---------------------------------------------------------

bool MidiFile::write(QIODevice* out)
      {
      fp = out;
      write("MThd", 4);
      writeLong(6);                 // header len
      writeShort(_format);          // format
      writeShort(_tracks.size());
      writeShort(_division);
      for (const auto &t: qAsConst(_tracks)) {
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
      switch(event.type()) {
            case ME_NOTEON:
                  writeStatus(ME_NOTEON, event.channel());
                  put(event.pitch());
                  put(event.velo());
                  break;

            case ME_NOTEOFF:
                  writeStatus(ME_NOTEOFF, event.channel());
                  put(event.pitch());
                  put(event.velo());
                  break;

            case ME_PITCHBEND:
                  writeStatus(ME_PITCHBEND, event.channel());
                  put(event.dataA());
                  put(event.dataB());
                  break;

            case ME_CONTROLLER:
                  switch(event.controller()) {
                        case CTRL_PROGRAM:
                              writeStatus(ME_PROGRAM, event.channel());
                              put(event.value() & 0x7f);
                              break;
                        case CTRL_PRESS:
                              writeStatus(ME_AFTERTOUCH, event.channel());
                              put(event.value() & 0x7f);
                              break;
                        default:
                              writeStatus(ME_CONTROLLER, event.channel());
                              put(event.controller());
                              put(event.value() & 0x7f);
                              break;
                        }
                  break;

            case ME_META:
                  put(ME_META);
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

            case ME_SYSEX:
                  put(ME_SYSEX);
                  putvl(event.len() + 1);  // including 0xf7
                  write(event.edata(), event.len());
                  put(ME_ENDSYSEX);
                  resetRunningStatus();
                  break;
            }
      }

//---------------------------------------------------------
//   writeTrack
//---------------------------------------------------------

bool MidiFile::writeTrack(const MidiTrack &t)
      {
      write("MTrk", 4);
      qint64 lenpos = fp->pos();
      writeLong(0);                 // dummy len

      status   = -1;
      int tick = 0;
      for (auto& i : t.events()) {
            int ntick = i.first;
            putvl(ntick - tick);    // write tick delta
            //
            // if track channel != -1, then use this
            //    channel for all events in this track
            //
            if (t.outChannel() != -1)
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

void MidiFile::writeStatus(int nstat, int c)
      {
      nstat |= (c & 0xf);
      //
      //  running status; except for Sysex- and Meta Events
      //
      if (_noRunningStatus || (((nstat & 0xf0) != 0xf0) && (nstat != status))) {
            status = nstat;
            put(nstat);
            }
      }

//---------------------------------------------------------
//   readMidi
//    return false on error
//---------------------------------------------------------

bool MidiFile::read(QIODevice* in)
     {
      fp = in;
      _tracks.clear();
      curPos    = 0;

      // === Read header_chunk = "MThd" + <header_length> + <format> + <n> + <division>
      //
      // "MThd" 4 bytes
      //    the literal string MThd, or in hexadecimal notation: 0x4d546864.
      //    These four characters at the start of the MIDI file
      //    indicate that this is a MIDI file.
      // <header_length> 4 bytes
      //    length of the header chunk (always =6 bytes long - the size of the next
      //    three fields which are considered the header chunk).
      //    Although the header chunk currently always contains 6 bytes of data,
      //    this should not be assumed, this value should always be read and acted upon,
      //    to allow for possible future extension to the standard.
      // <format> 2 bytes
      //    0 = single track file format
      //    1 = multiple track file format
      //    2 = multiple song file format (i.e., a series of type 0 files)
      // <n> 2 bytes
      //    number of track chunks that follow the header chunk
      // <division> 2 bytes
      //    unit of time for delta timing. If the value is positive, then it represents
      //    the units per beat. For example, +96 would mean 96 ticks per beat.
      //    If the value is negative, delta times are in SMPTE compatible units.

      char tmp[4];

      read(tmp, 4);
      int len = readLong();
      if (memcmp(tmp, "MThd", 4) || len < 6)
            throw(QString("bad midifile: MThd expected"));

      if (len > 6)
            throw(QString("unsupported MIDI header data size: %1 instead of 6").arg(len));

      _format     = readShort();
      int ntracks = readShort();

      // ================ Read MIDI division =================
      //
      //                        2 bytes
      //  +-------+---+-------------------+-----------------+
      //  |  bit  |15 | 14              8 | 7             0 |
      //  +-------+---+-------------------------------------+
      //  |       | 0 |       ticks per quarter note        |
      //  | value +---+-------------------------------------+
      //  |       | 1 |  -frames/second   |   ticks/frame   |
      //  +-------+---+-------------------+-----------------+

      char firstByte;
      fp->getChar(&firstByte);
      char secondByte;
      fp->getChar(&secondByte);
      const char topBit = (firstByte & 0x80) >> 7;

      if (topBit == 0) {            // ticks per beat
            _isDivisionInTps = false;
            _division = (firstByte << 8) | (secondByte & 0xff);
            }
      else {                        // ticks per second = fps * ticks per frame
            _isDivisionInTps = true;
            const int framesPerSecond = -((signed char) firstByte);
            const int ticksPerFrame = secondByte;
            if (framesPerSecond == 29)
                  _division = qRound(29.97 * ticksPerFrame);
            else
                  _division = framesPerSecond * ticksPerFrame;
            }

      // =====================================================

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

                  // Prevent "unreachable code" warning 
                  // return false;
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
      _tracks.push_back(MidiTrack());

      int port = 0;
      _tracks.back().setOutPort(port);
      _tracks.back().setOutChannel(-1);

      for (;;) {
            MidiEvent event;
            if (!readEvent(&event))
                  return true;

            // check for end of track:
            if ((event.type() == ME_META) && (event.metaType() == META_EOT))
                  break;
            _tracks.back().insert(click, event);
            }
      if (curPos != endPos) {
            qWarning("bad track len: %lld != %lld, %lld bytes too much\n", endPos, curPos, endPos - curPos);
            if (curPos < endPos) {
                  qWarning("  skip %lld\n", endPos-curPos);
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
//   writeShort
//---------------------------------------------------------

void MidiFile::writeShort(short i)
      {
      fp->putChar(i >> 8);
      fp->putChar(i);
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

//---------------------------------------------------------
//   writeLong
//---------------------------------------------------------

void MidiFile::writeLong(long i)
      {
      fp->putChar(i >> 24);
      fp->putChar(i >> 16);
      fp->putChar(i >> 8);
      fp->putChar(i);
      }

/*---------------------------------------------------------
 *    skip
 *    This is meant for skipping a few bytes in a
 *    file or fifo.
 *---------------------------------------------------------*/

void MidiFile::skip(qint64 len)
      {
      // Note: if MS is updated to use Qt 5.10, this can be implemented with QIODevice::skip(), which should be more efficient
      //       as bytes do not need to be moved around.
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

//---------------------------------------------------------
//   MidiTrack
//---------------------------------------------------------

MidiTrack::MidiTrack()
      {
      _outChannel = -1;
      _outPort    = -1;
      _drumTrack  = false;
      }

MidiTrack::~MidiTrack()
      {
      }

//---------------------------------------------------------
//   insert
//---------------------------------------------------------

void MidiTrack::insert(int tick, const MidiEvent& event)
      {
      _events.insert({tick, event});
      }

//---------------------------------------------------------
//   readEvent
//    return true on success
//---------------------------------------------------------

bool MidiFile::readEvent(MidiEvent* event)
      {
      uchar me, a, b;

      int nclick = getvl();
      if (nclick == -1) {
            qDebug("readEvent: error 1(getvl)");
            return false;
            }
      click += nclick;
      for (;;) {
            read(&me, 1);
            if (me >= 0xf1 && me <= 0xfe && me != 0xf7) {
                  qDebug("Midi: Unknown Message 0x%02x", me & 0xff);
                  }
            else
                  break;
            }

      int dataLen;

      if (me == 0xf0 || me == 0xf7) {
            status  = -1;                  // no running status
            int len = getvl();
            if (len == -1) {
                  qDebug("readEvent: error 3");
                  return false;
                  }
            dataLen = len;
            std::vector<unsigned char> data(len + 1);
            read(data.data(), len);
            if (data[len - 1] != 0xf7) {
                  qDebug("SYSEX does not end with 0xf7!");
                  // more to come?
                  }
            else
                  dataLen--;      // don't count 0xf7
            event->setType(ME_SYSEX);
            event->setEData(std::move(data));
            event->setLen(dataLen);
            return true;
            }
      else if (me == ME_META) {
            status = -1;                  // no running status
            uchar type;
            read(&type, 1);
            dataLen = getvl();                // read len
            if (dataLen == -1) {
                  qDebug("readEvent: error 6");
                  return false;
                  }
            std::vector<unsigned char> data(dataLen + 1);
            if (dataLen)
                  read(data.data(), dataLen);

            event->setType(ME_META);
            event->setMetaType(type);
            event->setLen(dataLen);
            event->setEData(std::move(data));
            return true;
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
                  if (sstatus == -1) {
                        return 0;
                        }
                  status = sstatus;
                  }
            a = me;
            }
      int channel = status & 0x0f;
      b           = 0;
      switch (status & 0xf0) {
            case ME_NOTEOFF:
            case ME_NOTEON:
            case ME_POLYAFTER:
            case ME_CONTROLLER:        // controller
            case ME_PITCHBEND:        // pitch bend
                  read(&b, 1);
                  break;
            }
      event->setType(status & 0xf0);
      event->setChannel(channel);
      switch (status & 0xf0) {
            case ME_NOTEOFF:
                  event->setDataA(a & 0x7f);
                  event->setDataB(b & 0x7f);
                  break;
            case ME_NOTEON:
                  event->setDataA(a & 0x7f);
                  event->setDataB(b & 0x7f);
                  break;
            case ME_POLYAFTER:
                  event->setType(ME_CONTROLLER);
                  event->setController(CTRL_POLYAFTER);
                  event->setValue(((a & 0x7f) << 8) + (b & 0x7f));
                  break;
            case ME_CONTROLLER:        // controller
                  event->setController(a & 0x7f);
                  event->setValue(b & 0x7f);
                  break;
            case ME_PITCHBEND:        // pitch bend
                  event->setDataA(a & 0x7f);
                  event->setDataB(b & 0x7f);
                  break;
            case ME_PROGRAM:
                  event->setValue(a & 0x7f);
                  break;
            case ME_AFTERTOUCH:
                  event->setType(ME_CONTROLLER);
                  event->setController(CTRL_PRESS);
                  event->setValue(a & 0x7f);
                  break;
            default:          // f1 f2 f3 f4 f5 f6 f7 f8 f9
                  qDebug("BAD STATUS 0x%02x, me 0x%02x", status, me);
                  return false;
            }

      if ((a & 0x80) || (b & 0x80)) {
            qDebug("8't bit in data set(%02x %02x): tick %d read 0x%02x  status:0x%02x",
              a & 0xff, b & 0xff, click, me, status);
            qDebug("readEvent: error 16");
            if (b & 0x80) {
                  // Try to fix: interpret as channel byte
                  status   = b;
                  sstatus  = status;
                  return true;
                  }
            return false;
            }
      return true;
      }

//---------------------------------------------------------
//   setOutChannel
//---------------------------------------------------------

void MidiTrack::setOutChannel(int n)
      {
      _outChannel = n;
      if (_outChannel == 9)
            _drumTrack = true;
      }

//---------------------------------------------------------
//   mergeNoteOnOffAndFindMidiType
//    - find matching note on / note off events and merge
//      into a note event with tick duration
//    - find MIDI type
//---------------------------------------------------------

void MidiTrack::mergeNoteOnOffAndFindMidiType(MidiType *mt)
      {
      std::multimap<int, MidiEvent> el;

      int hbank = 0xff;
      int lbank = 0xff;
      int rpnh  = -1;
      int rpnl  = -1;
      int datah = 0;
      int datal = 0;
      int dataType = 0; // 0 : disabled, 0x20000 : rpn, 0x30000 : nrpn;

      for (auto i = _events.begin(); i != _events.end(); ++i) {
            MidiEvent& ev = i->second;
            if (ev.type() == ME_INVALID)
                  continue;
            if ((ev.type() != ME_NOTEON) && (ev.type() != ME_NOTEOFF)) {
                  if (ev.type() == ME_CONTROLLER) {
                        int val  = ev.value();
                        int cn   = ev.controller();
                        if (cn == CTRL_HBANK) {
                              hbank = val;
                              ev.setType(ME_INVALID);
                              continue;
                              }
                        else if (cn == CTRL_LBANK) {
                              lbank = val;
                              ev.setType(ME_INVALID);
                              continue;
                              }
                        else if (cn == CTRL_HDATA) {
                              datah = val;

                              // check if a CTRL_LDATA follows
                              // e.g. wie have a 14 bit controller:
                              auto ii = i;
                              ++ii;
                              bool found = false;
                              for (; ii != _events.end(); ++ii) {
                                    MidiEvent& ev1 = ii->second;
                                    if (ev1.type() == ME_CONTROLLER) {
                                          if (ev1.controller() == CTRL_LDATA) {
                                                // handle later
                                                found = true;
                                                }
                                          break;
                                          }
                                    }
                              if (!found) {
                                    if (rpnh == -1 || rpnl == -1) {
                                          qDebug("parameter number not defined, data 0x%x", datah);
                                          ev.setType(ME_INVALID);
                                          continue;
                                          }
                                    else {
                                          ev.setController(dataType | (rpnh << 8) | rpnl);
                                          ev.setValue(datah);
                                          }
                                    }
                              else {
                                    ev.setType(ME_INVALID);
                                    continue;
                                    }
                              }
                        else if (cn == CTRL_LDATA) {
                              datal = val;

                              if (rpnh == -1 || rpnl == -1) {
                                    qDebug("parameter number not defined, data 0x%x 0x%x, tick %d, channel %d",
                                       datah, datal, i->first, ev.channel());
                                    continue;
                                    }
                              // assume that the sequence is always
                              //    CTRL_HDATA - CTRL_LDATA
                              // eg. that LDATA is always send last

                              // 14 Bit RPN/NRPN
                              ev.setController((dataType+0x30000) | (rpnh << 8) | rpnl);
                              ev.setValue((datah << 7) | datal);
                              }
                        else if (cn == CTRL_HRPN) {
                              rpnh = val;
                              dataType = 0x20000;
                              ev.setType(ME_INVALID);
                              continue;
                              }
                        else if (cn == CTRL_LRPN) {
                              rpnl = val;
                              dataType = 0x20000;
                              ev.setType(ME_INVALID);
                              continue;
                              }
                        else if (cn == CTRL_HNRPN) {
                              rpnh = val;
                              dataType = 0x30000;
                              ev.setType(ME_INVALID);
                              continue;
                              }
                        else if (cn == CTRL_LNRPN) {
                              rpnl = val;
                              dataType = 0x30000;
                              ev.setType(ME_INVALID);
                              continue;
                              }
                        else if (cn == CTRL_PROGRAM) {
                              ev.setValue((hbank << 16) | (lbank << 8) | ev.value());
                              // TODO el.insert(ev);
                              ev.setType(ME_INVALID);
                              continue;
                              }
                        }
                  else if (ev.type() == ME_SYSEX) {
                        int len = ev.len();
                        const uchar* buffer = ev.edata();
                        if ((len == gmOnMsgLen) && memcmp(buffer, gmOnMsg, gmOnMsgLen) == 0) {
                              *mt = MidiType::GM;
                              ev.setType(ME_INVALID);
                              continue;
                              }
                        if ((len == gsOnMsgLen) && memcmp(buffer, gsOnMsg, gsOnMsgLen) == 0) {
                              *mt = MidiType::GS;
                              ev.setType(ME_INVALID);
                              continue;
                              }
                        if ((len == xgOnMsgLen) && memcmp(buffer, xgOnMsg, xgOnMsgLen) == 0) {
                              *mt = MidiType::XG;
                              ev.setType(ME_INVALID);
                              continue;
                              }
                        if (buffer[0] == 0x43) {    // Yamaha
                              *mt = MidiType::XG;
                              int type   = buffer[1] & 0xf0;
                              if (type == 0x10) {
//TODO                                    if (buffer[1] != 0x10) {
//                                          buffer[1] = 0x10;    // fix to Device 1
//                                          }
                                    if ((len == xgOnMsgLen) && memcmp(buffer, xgOnMsg, xgOnMsgLen) == 0) {
                                          *mt = MidiType::XG;
                                          ev.setType(ME_INVALID);
                                          continue;
                                          }
                                    if (len == 7 && buffer[2] == 0x4c && buffer[3] == 0x08 && buffer[5] == 7) {
                                          // part mode
                                          // 0 - normal
                                          // 1 - DRUM
                                          // 2 - DRUM 1
                                          // 3 - DRUM 2
                                          // 4 - DRUM 3
                                          // 5 - DRUM 4
                                          if (buffer[6] != 0 && buffer[4] == ev.channel()) {
                                                _drumTrack = true;
                                                }
                                          ev.setType(ME_INVALID);
                                          continue;
                                          }
                                    }
                              }
                        }
                  el.insert(std::pair<int,MidiEvent>(i->first, ev));
                  ev.setType(ME_INVALID);
                  continue;
                  }
            int tick = i->first;
            if (ev.type() == ME_NOTEOFF || ev.velo() == 0) {
                  qDebug("-extra note off at %d", tick);
                  ev.setType(ME_INVALID);
                  continue;
                  }
            MidiEvent note(ME_NOTE, ev.channel(), ev.dataA(), ev.dataB());

            auto k = i;
            ++k;
            for (; k != _events.end(); ++k) {
                  MidiEvent& e = k->second;
                  if (e.type() != ME_NOTEON && e.type() != ME_NOTEOFF)
                        continue;
                  if ((e.type() == ME_NOTEOFF || (e.type() == ME_NOTEON && e.velo() == 0))
                     && (e.pitch() == note.pitch())) {
                        int t = k->first - tick;
                        if (t <= 0)
                              t = 1;
                        note.setLen(t);
                        e.setType(ME_INVALID);
                        break;
                        }
                  }
            if (k == _events.end()) {
                  qDebug("-no note-off for note at %d", tick);
                  note.setLen(1);
                  }
            el.insert(std::pair<int,MidiEvent>(tick, note));
            ev.setType(ME_INVALID);
            }
      _events = el;
      }

//---------------------------------------------------------
//   separateChannel
//    if a track contains events for different midi channels,
//    then split events into separate tracks
//---------------------------------------------------------

void MidiFile::separateChannel()
      {
      for (int i = 0; i < _tracks.size(); ++i) {
                        // create a list of channels used in current track
            QList<int> channel;
            MidiTrack &mt = _tracks[i];      // current track
            for (const auto& ie : mt.events()) {
                  const MidiEvent& e = ie.second;
                  if (e.isChannelEvent() && !channel.contains(e.channel()))
                        channel.append(e.channel());
                  }
            mt.setOutChannel(channel.empty() ? 0 : channel[0]);
            int nn = channel.size();
            if (nn <= 1)
                  continue;
            std::sort(channel.begin(), channel.end());
                        // -- split --
                        // insert additional tracks, assign to them found channels
            for (int ii = 1; ii < nn; ++ii) {
                  MidiTrack t;
                  t.setOutChannel(channel[ii]);
                  _tracks.insert(i + ii, t);
                  }
                        // extract all different channel events from current track to inserted tracks
            for (auto ie = mt.events().begin(); ie != mt.events().end(); ) {
                  const MidiEvent& e = ie->second;
                  if (e.isChannelEvent()) {
                        int ch  = e.channel();
                        int idx = channel.indexOf(ch);
                        MidiTrack &t = _tracks[i + idx];
                        if (&t != &mt) {
                              t.insert(ie->first, e);
                              ie = mt.events().erase(ie);
                              continue;
                              }
                        }
                  ++ie;
                  }
            i += nn - 1;
            }
      }

} // namespace Ms
