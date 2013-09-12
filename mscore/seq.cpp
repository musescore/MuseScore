//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: seq.cpp 5660 2012-05-22 14:17:39Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "config.h"
#include "seq.h"
#include "musescore.h"

#include "synthesizer/msynthesizer.h"
#include "libmscore/slur.h"
#include "libmscore/tie.h"
#include "libmscore/score.h"
#include "libmscore/segment.h"
#include "libmscore/note.h"
#include "libmscore/chord.h"
#include "libmscore/tempo.h"
#include "scoreview.h"
#include "playpanel.h"
#include "libmscore/staff.h"
#include "libmscore/measure.h"
#include "preferences.h"
#include "libmscore/part.h"
#include "libmscore/ottava.h"
#include "libmscore/utils.h"
#include "libmscore/repeatlist.h"
#include "libmscore/audio.h"
#include "synthcontrol.h"
#include "pianoroll.h"

#include "click.h"

#include <vorbis/vorbisfile.h>

namespace Ms {

Seq* seq;

static const int guiRefresh   = 10;       // Hz
static const int peakHoldTime = 1400;     // msec
static const int peakHold     = (peakHoldTime * guiRefresh) / 1000;
static OggVorbis_File vf;

static const int AUDIO_BUFFER_SIZE = 1024 * 512;  // 2 MB

//---------------------------------------------------------
//   VorbisData
//---------------------------------------------------------

struct VorbisData {
      int pos;          // current position in audio->data()
      QByteArray data;
      };

static VorbisData vorbisData;

static size_t ovRead(void* ptr, size_t size, size_t nmemb, void* datasource);
static int ovSeek(void* datasource, ogg_int64_t offset, int whence);
static long ovTell(void* datasource);

static ov_callbacks ovCallbacks = {
      ovRead, ovSeek, 0, ovTell
      };

//---------------------------------------------------------
//   ovRead
//---------------------------------------------------------

static size_t ovRead(void* ptr, size_t size, size_t nmemb, void* datasource)
      {
      VorbisData* vd = (VorbisData*)datasource;
      size_t n = size * nmemb;
      if (vd->data.size() < int(vd->pos + n))
            n = vd->data.size() - vd->pos;
      if (n) {
            const char* src = vd->data.data() + vd->pos;
            memcpy(ptr, src, n);
            vd->pos += n;
            }
      return n;
      }

//---------------------------------------------------------
//   ovSeek
//---------------------------------------------------------

static int ovSeek(void* datasource, ogg_int64_t offset, int whence)
      {
      VorbisData* vd = (VorbisData*)datasource;
      switch(whence) {
            case SEEK_SET:
                  vd->pos = offset;
                  break;
            case SEEK_CUR:
                  vd->pos += offset;
                  break;
            case SEEK_END:
                  vd->pos = vd->data.size() - offset;
                  break;
            }
      return 0;
      }

//---------------------------------------------------------
//   ovTell
//---------------------------------------------------------

static long ovTell(void* datasource)
      {
      VorbisData* vd = (VorbisData*)datasource;
      return vd->pos;
      }

//---------------------------------------------------------
//   Seq
//---------------------------------------------------------

Seq::Seq()
      {
      running         = false;
      playlistChanged = false;
      cs              = 0;
      cv              = 0;
      tackRest        = 0;
      tickRest        = 0;

      endTick  = 0;
      state    = TRANSPORT_STOP;
      oggInit  = false;
      _driver  = 0;
      playPos  = events.cbegin();

      playTime  = 0;
      metronomeVolume = 0.3;

      meterValue[0]     = 0.0;
      meterValue[1]     = 0.0;
      meterPeakValue[0] = 0.0;
      meterPeakValue[1] = 0.0;
      peakTimer[0]       = 0;
      peakTimer[1]       = 0;

      heartBeatTimer = new QTimer(this);
      connect(heartBeatTimer, SIGNAL(timeout()), this, SLOT(heartBeatTimeout()));

      noteTimer = new QTimer(this);
      noteTimer->setSingleShot(true);
      connect(noteTimer, SIGNAL(timeout()), this, SLOT(stopNotes()));
      noteTimer->stop();

      connect(this, SIGNAL(toGui(int)), this, SLOT(seqMessage(int)), Qt::QueuedConnection);
      }

//---------------------------------------------------------
//   Seq
//---------------------------------------------------------

Seq::~Seq()
      {
      delete _driver;
      }

//---------------------------------------------------------
//   setScoreView
//---------------------------------------------------------

void Seq::setScoreView(ScoreView* v)
      {
      if (oggInit) {
            ov_clear(&vf);
            oggInit = false;
            }
      if (cv !=v && cs) {
            unmarkNotes();
            stopWait();
            }
      cv = v;
      cs = cv ? cv->score() : 0;

      if (!heartBeatTimer->isActive())
            heartBeatTimer->start(20);    // msec

      playlistChanged = true;
      _synti->reset();
      if (cs)
            initInstruments();
      }

//---------------------------------------------------------
//   selectionChanged
//---------------------------------------------------------

void Seq::selectionChanged(int mode)
      {
      if (cs == 0 || _driver == 0)
            return;

      int tick = cs->pos();
      if (tick == -1)
            return;

      if ((mode != SEL_LIST) || (state == TRANSPORT_STOP))
            cs->setPlayPos(tick);
      else
            seek(tick);
      }

//---------------------------------------------------------
//   init
//    return false on error
//---------------------------------------------------------

bool Seq::init()
      {
      if (!_driver || !_driver->start()) {
            qDebug("Cannot start I/O");
            return false;
            }
      running = true;
      return true;
      }

//---------------------------------------------------------
//   exit
//---------------------------------------------------------

void Seq::exit()
      {
      if (_driver) {
            if (MScore::debugMode)
                  qDebug("Stop I/O\n");
            stopWait();
            delete _driver;
            _driver = 0;
            }
      }

//---------------------------------------------------------
//   inputPorts
//---------------------------------------------------------

QList<QString> Seq::inputPorts()
      {
      if (_driver)
            return _driver->inputPorts();
      QList<QString> a;
      return a;
      }

//---------------------------------------------------------
//   rewindStart
//---------------------------------------------------------

void Seq::rewindStart()
      {
      seek(0);
      }

//---------------------------------------------------------
//   loopStart
//---------------------------------------------------------

void Seq::loopStart()
      {
      start();
//      qDebug("LoopStart. playPos = %d\n", playPos);
      }

//---------------------------------------------------------
//   canStart
//    return true if sequencer can be started
//---------------------------------------------------------

bool Seq::canStart()
      {
      if (!_driver)
            return false;
      if (events.empty() || cs->playlistDirty() || playlistChanged)
            collectEvents();
      return (!events.empty() && endTick != 0);
      }

//---------------------------------------------------------
//   start
//    called from gui thread
//---------------------------------------------------------

void Seq::start()
      {
      if (events.empty() || cs->playlistDirty() || playlistChanged)
            collectEvents();
      if (cs->playMode() == PLAYMODE_AUDIO) {
            if (!oggInit) {
                  vorbisData.pos  = 0;
                  vorbisData.data = cs->audio()->data();
                  int n = ov_open_callbacks(&vorbisData, &vf, 0, 0, ovCallbacks);
                  if (n < 0) {
                        printf("ogg open failed: %d\n", n);
                        }
                  oggInit = true;
                  }
            }
      if ((mscore->loop())) {
            if(cs->selection().state() == SEL_RANGE)
                  setLoopSelection();
            seek(cs->repeatList()->tick2utick(cs->loopInTick()));
            }
      else
            seek(cs->playPos());
      _driver->startTransport();
      }

//---------------------------------------------------------
//   stop
//    called from gui thread
//---------------------------------------------------------

void Seq::stop()
      {
      if (state == TRANSPORT_STOP)
            return;
      if (oggInit) {
            ov_clear(&vf);
            oggInit = false;
            }
      if (!_driver)
            return;
      _driver->stopTransport();
      if (cv)
            cv->setCursorOn(false);
      if (cs) {
            cs->setLayoutAll(false);
            cs->setUpdateAll();
            cs->end();
            }
      }

//---------------------------------------------------------
//   stopWait
//---------------------------------------------------------

void Seq::stopWait()
      {
      stop();
      QWaitCondition sleep;
      int idx = 0;
      while (state != TRANSPORT_STOP) {
            printf("state %d\n", state);
            mutex.lock();
            sleep.wait(&mutex, 100);
            mutex.unlock();
            ++idx;
            if (idx > 10)
                  abort();
            }
      }

//---------------------------------------------------------
//   seqStarted
//---------------------------------------------------------

void MuseScore::seqStarted()
      {
      if (cv)
            cv->setCursorOn(true);
      if (cs)
            cs->end();
      }

//---------------------------------------------------------
//   seqStopped
//    JACK has stopped
//    executed in gui environment
//---------------------------------------------------------

void MuseScore::seqStopped()
      {
      cv->setCursorOn(false);
      }

//---------------------------------------------------------
//   unmarkNotes
//---------------------------------------------------------

void Seq::unmarkNotes()
      {
      foreach(const Note* n, markedNotes) {
            n->setMark(false);
            cs->addRefresh(n->canvasBoundingRect());
            }
      markedNotes.clear();
      }

//---------------------------------------------------------
//   guiStop
//---------------------------------------------------------

void Seq::guiStop()
      {
      QAction* a = getAction("play");
      a->setChecked(false);

      unmarkNotes();
      if (!cs)
            return;

      cs->setPlayPos(cs->utime2utick(qreal(playTime) / qreal(MScore::sampleRate)));
      cs->end();
      emit stopped();
      }

//---------------------------------------------------------
//   seqSignal
//    sequencer message to GUI
//    execution environment: gui thread
//---------------------------------------------------------

void Seq::seqMessage(int msg)
      {
      switch(msg) {
            case '4':   // Restart the playback at the end of the score
                  loopStart();
                  break;
            case '3':   // Loop restart while playing
                  seek(cs->repeatList()->tick2utick(cs->loopInTick()));
                  break;
            case '2':
                  guiStop();
//                  heartBeatTimer->stop();
                  if (_driver && mscore->getSynthControl()) {
                        meterValue[0]     = .0f;
                        meterValue[1]     = .0f;
                        meterPeakValue[0] = .0f;
                        meterPeakValue[1] = .0f;
                        peakTimer[0]       = 0;
                        peakTimer[1]       = 0;
                        mscore->getSynthControl()->setMeter(0.0, 0.0, 0.0, 0.0);
                        }
                  seek(0);
                  break;
            case '0':         // STOP
                  guiStop();
//                  heartBeatTimer->stop();
                  if (_driver && mscore->getSynthControl()) {
                        meterValue[0]     = .0f;
                        meterValue[1]     = .0f;
                        meterPeakValue[0] = .0f;
                        meterPeakValue[1] = .0f;
                        peakTimer[0]       = 0;
                        peakTimer[1]       = 0;
                        mscore->getSynthControl()->setMeter(0.0, 0.0, 0.0, 0.0);
                        }
                  break;

            case '1':         // PLAY
                  emit started();
//                  heartBeatTimer->start(1000/guiRefresh);
                  break;

            default:
                  qDebug("MScore::Seq:: unknown seq msg %d\n", msg);
                  break;
            }
      }

//---------------------------------------------------------
//   playEvent
//    send one event to the synthesizer
//---------------------------------------------------------

void Seq::playEvent(const NPlayEvent& event)
      {
      int type = event.type();
      if (type == ME_NOTEON) {
            bool mute;
            const Note* note = event.note();

            if (note) {
                  Instrument* instr = note->staff()->part()->instr();
                  const Channel& a = instr->channel(note->subchannel());
                  mute = a.mute || a.soloMute;
                  }
            else
                  mute = false;

            if (!mute)
                  putEvent(event);
            }
      else if (type == ME_CONTROLLER)
            putEvent(event);
      }

//---------------------------------------------------------
//   processMessages
//---------------------------------------------------------

void Seq::processMessages()
      {
      for (;;) {
            if (toSeq.isEmpty())
                  break;
            SeqMsg msg = toSeq.dequeue();
            switch(msg.id) {
                  case SEQ_TEMPO_CHANGE:
                        {
                        if (playTime != 0) {
                              int tick = cs->utime2utick(qreal(playTime) / qreal(MScore::sampleRate));
                              cs->tempomap()->setRelTempo(msg.realVal);
                              cs->repeatList()->update();
                              playTime = cs->utick2utime(tick) * MScore::sampleRate;
                              }
                        else
                              cs->tempomap()->setRelTempo(msg.realVal);
                        }
                        break;
                  case SEQ_PLAY:
                        putEvent(msg.event);
                        break;
                  case SEQ_SEEK:
                        setPos(msg.intVal);
                        break;
                  }
            }
      }

//---------------------------------------------------------
//   metronome
//---------------------------------------------------------

void Seq::metronome(unsigned n, float* p)
      {
      if (!mscore->metronome()) {
            tickRest = 0;
            tackRest = 0;
            return;
            }
      if (tickRest) {
            tackRest = 0;
            int idx = tickLength - tickRest;
            int nn = n < tickRest ? n : tickRest;
            for (int i = 0; i < nn; ++i) {
                  qreal v = tick[idx] * metronomeVolume;
                  *p++ += v;
                  *p++ += v;
                  ++idx;
                  }
            tickRest -= nn;
            }
      if (tackRest) {
            int idx = tackLength - tackRest;
            int nn = n < tackRest ? n : tackRest;
            for (int i = 0; i < nn; ++i) {
                  qreal v = tack[idx] * metronomeVolume;
                  *p++ += v;
                  *p++ += v;
                  ++idx;
                  }
            tackRest -= nn;
            }
      }

//---------------------------------------------------------
//   process
//---------------------------------------------------------

void Seq::process(unsigned n, float* buffer)
      {
      unsigned frames = n;
      int driverState = _driver->getState();

      if (driverState != state) {
            if (state == TRANSPORT_STOP && driverState == TRANSPORT_PLAY) {
                  state = TRANSPORT_PLAY;
                  emit toGui('1');
                  }
            else if (state == TRANSPORT_PLAY && driverState == TRANSPORT_STOP) {
                  state = TRANSPORT_STOP;
                  stopNotes();
                  // send sustain off
                  // TODO: channel?
                  putEvent(NPlayEvent(ME_CONTROLLER, 0, CTRL_SUSTAIN, 0));
                  if (playPos == events.cend()) {
                        if (mscore->loop()) {
                              qDebug("Seq.cpp - Process - Loop whole score. playPos = %d     cs->pos() = %d", playPos->first,cs->pos());
                              emit toGui('4');
                              loopStart();
                              }
                        else {
                              emit toGui('2');
                              }
                        }
                  else {
                     emit toGui('0');
                     }
                  }
            else if (state != driverState)
                  qDebug("Seq: state transition %d -> %d ?\n",
                     state, driverState);
            }

      memset(buffer, 0, sizeof(float) * n * 2);
      float* p = buffer;
      processMessages();

      if (state == TRANSPORT_PLAY) {
            if(!cs)
                  return;
            //
            // play events for one segment
            //
            unsigned framePos = 0;
            int endTime = playTime + frames;
            for (; playPos != events.cend();) {
                  int f = cs->utick2utime(playPos->first) * MScore::sampleRate;
                  if (f >= endTime)
                        break;
                  int n = f - playTime;
                  if (n < 0) {
                        qDebug("%d:  %d - %d\n", playPos->first, f, playTime);
						n = 0;
                        }
                  if (mscore->loop()) {
                        int tickLoop = cs->repeatList()->tick2utick(cs->loopOutTick());
                        if (tickLoop < cs->lastMeasure()->endTick()-1)
                              if (playPos->first >= tickLoop) {
                                    qDebug ("Process playPos = %d  in/out tick = %d/%d  getCurTick() = %d   tickLoop = %d   playTime = %d", playPos->first, cs->loopInTick(), cs->loopOutTick(), getCurTick(), tickLoop, playTime);
                                    emit toGui('3');   // Exit this function to avoid segmentation fault in Scoreview
                                    return;
                                    }
                        }
                  if (n) {
                        if (cs->playMode() == PLAYMODE_SYNTHESIZER) {
                              metronome(n, p);
                              _synti->process(n, p);
                              p += n * 2;
                              playTime  += n;
                              frames    -= n;
                              framePos  += n;
                              }
                        else {
                              while (n > 0) {
                                    int section;
                                    float** pcm;
                                    long rn = ov_read_float(&vf, &pcm, n, &section);
                                    if (rn == 0)
                                          break;
                                    for (int i = 0; i < rn; ++i) {
                                          *p++ = pcm[0][i];
                                          *p++ = pcm[1][i];
                                          }
                                    playTime += rn;
                                    frames   -= rn;
                                    framePos += rn;
                                    n        -= rn;
                                    }
                              }
                        }
                  const NPlayEvent& event = playPos->second;
                  playEvent(event);
                  if (event.type() == ME_TICK1)
                        tickRest = tickLength;
                  else if (event.type() == ME_TICK2)
                        tackRest = tackLength;
                  mutex.lock();
                  ++playPos;
                  mutex.unlock();
                  }
            if (frames) {
                  if (cs->playMode() == PLAYMODE_SYNTHESIZER) {
                        metronome(frames, p);
                        _synti->process(frames, p);
                        playTime += frames;
                        }
                  else {
                        int n = frames;
                        while (n > 0) {
                              int section;
                              float** pcm;
                              long rn = ov_read_float(&vf, &pcm, n, &section);
                              if (rn == 0)
                                    break;
                              for (int i = 0; i < rn; ++i) {
                                    *p++ = pcm[0][i];
                                    *p++ = pcm[1][i];
                                    }
                              playTime += rn;
                              frames   -= rn;
                              framePos += rn;
                              n        -= rn;
                              }
                        }
                  }
            if (playPos == events.cend())
                  _driver->stopTransport();
            }
      else {
            _synti->process(frames, p);
            }
      //
      // metering / master gain
      //
      float lv = 0.0f;
      float rv = 0.0f;
      p = buffer;
      for (unsigned i = 0; i < n; ++i) {
            qreal val = *p;
            lv = qMax(lv, fabsf(val));
            *p++ = val;

            val = *p;
            rv = qMax(lv, fabsf(val));
            *p++ = val;
            }
      meterValue[0] = lv;
      meterValue[1] = rv;
      if (meterPeakValue[0] < lv) {
            meterPeakValue[0] = lv;
            peakTimer[0] = 0;
            }
      if (meterPeakValue[1] < rv) {
            meterPeakValue[1] = rv;
            peakTimer[1] = 0;
            }
      }

//---------------------------------------------------------
//   initInstruments
//---------------------------------------------------------

void Seq::initInstruments()
      {
      foreach(const MidiMapping& mm, *cs->midiMapping()) {
            Channel* channel = mm.articulation;
            foreach(const MidiCoreEvent& e, channel->init) {
                  if (e.type() == ME_INVALID)
                        continue;
                  NPlayEvent event(e.type(), channel->channel, e.dataA(), e.dataB());
                  sendEvent(event);
                  }
            }
      }

//---------------------------------------------------------
//   collectEvents
//---------------------------------------------------------

void Seq::collectEvents()
      {
      //do not collect even while playing
      if (state ==  TRANSPORT_PLAY)
            return;
      events.clear();

      mutex.lock();
      cs->renderMidi(&events);
      endTick = 0;

      if (!events.empty()) {
            auto e = events.cend();
            --e;
            endTick = e->first;
            }
      playPos  = events.cbegin();
      mutex.unlock();

      playlistChanged = false;
      cs->setPlaylistDirty(false);
      }

//---------------------------------------------------------
//   getCurTick
//---------------------------------------------------------

int Seq::getCurTick()
      {
      return cs->utime2utick(qreal(playTime) / qreal(MScore::sampleRate));
      }

//---------------------------------------------------------
//   setRelTempo
//    relTempo = 1.0 = normal tempo
//---------------------------------------------------------

void Seq::setRelTempo(double relTempo)
      {
      guiToSeq(SeqMsg(SEQ_TEMPO_CHANGE, relTempo));
      }

//---------------------------------------------------------
//   setPos
//    seek
//    realtime environment
//---------------------------------------------------------

void Seq::setPos(int utick)
      {
      if (cs == 0)
            return;
      stopNotes();

      int ucur;
      if (playPos != events.end())
            ucur = cs->repeatList()->utick2tick(playPos->first);
      else
            ucur = utick - 1;
      if (utick != ucur)
            updateSynthesizerState(ucur, utick);

      playTime  = cs->utick2utime(utick) * MScore::sampleRate;
      mutex.lock();
      playPos   = events.lower_bound(utick);
      mutex.unlock();
      }

//---------------------------------------------------------
//   seek
//    send seek message to sequencer
//---------------------------------------------------------

void Seq::seek(int utick)
      {
      if (cs == 0)
            return;

      qDebug ("seek : utick=%d",utick);
      if (events.empty() || cs->playlistDirty() || playlistChanged)
            collectEvents();
      int tick     = cs->repeatList()->utick2tick(utick);
      Segment* seg = cs->tick2segment(tick);
      seek(utick, seg);
      }

//---------------------------------------------------------
//   seek
//    send seek message to sequencer
//---------------------------------------------------------

void Seq::seek(int utick, Segment* seg)
      {
      if (seg)
            mscore->currentScoreView()->moveCursor(seg->tick());

      cs->setPlayPos(utick);
      cs->setLayoutAll(false);
      cs->end();

      if (cs->playMode() == PLAYMODE_AUDIO) {
            ogg_int64_t sp = cs->utick2utime(utick) * MScore::sampleRate;
            ov_pcm_seek(&vf, sp);
            }

      guiToSeq(SeqMsg(SEQ_SEEK, utick));
      guiPos = events.lower_bound(utick);
      mscore->setPos(utick);
      unmarkNotes();
      cs->update();
      }

//---------------------------------------------------------
//   startNote
//---------------------------------------------------------

void Seq::startNote(int channel, int pitch, int velo, double nt)
      {
      if (state != TRANSPORT_STOP)
            return;
      NPlayEvent ev(ME_NOTEON, channel, pitch, velo);
      ev.setTuning(nt);
      sendEvent(ev);
      }

void Seq::startNote(int channel, int pitch, int velo, int duration, double nt)
      {
      stopNotes();
      startNote(channel, pitch, velo, nt);
      startNoteTimer(duration);
      }

//---------------------------------------------------------
//   startNoteTimer
//---------------------------------------------------------

void Seq::startNoteTimer(int duration)
      {
      if (duration) {
            noteTimer->setInterval(duration);
            noteTimer->start();
            }
      }
//---------------------------------------------------------
//   stopNoteTimer
//---------------------------------------------------------

void Seq::stopNoteTimer()
      {
      if (noteTimer->isActive()) {
            noteTimer->stop();
            stopNotes();
            }
      }

//---------------------------------------------------------
//   stopNotes
//---------------------------------------------------------

void Seq::stopNotes(int channel)
      {
      _synti->allNotesOff(channel);
      }

//---------------------------------------------------------
//   setController
//---------------------------------------------------------

void Seq::setController(int channel, int ctrl, int data)
      {
      NPlayEvent event(ME_CONTROLLER, channel, ctrl, data);
      sendEvent(event);
      }

//---------------------------------------------------------
//   sendEvent
//    called from GUI context to send a midi event to
//    midi out or synthesizer
//---------------------------------------------------------

void Seq::sendEvent(const NPlayEvent& ev)
      {
      guiToSeq(SeqMsg(SEQ_PLAY, ev));
      }

//---------------------------------------------------------
//   nextMeasure
//---------------------------------------------------------

void Seq::nextMeasure()
      {
      Measure* m = cs->tick2measure(guiPos->first);
      if (m) {
            if (m->nextMeasure())
                  m = m->nextMeasure();
            seek(m->tick());
            }
      }

//---------------------------------------------------------
//   nextChord
//---------------------------------------------------------

void Seq::nextChord()
      {
      int tick = guiPos->first;
      for (auto i = guiPos; i != events.cend(); ++i) {
            if (i->second.type() == ME_NOTEON && i->first > tick && i->second.velo()) {
                  seek(i->first);
                  break;
                  }
            }
      }

//---------------------------------------------------------
//   prevMeasure
//---------------------------------------------------------

void Seq::prevMeasure()
      {
      auto i = guiPos;
      if (i == events.begin())
            return;
      --i;
      Measure* m = cs->tick2measure(i->first);
      if (m) {
            if ((i->first == m->tick()) && m->prevMeasure())
                  m = m->prevMeasure();
            seek(m->tick());
            }
      }

//---------------------------------------------------------
//   prevChord
//---------------------------------------------------------

void Seq::prevChord()
      {
      int tick  = playPos->first;
      //find the chord just before playpos
      EventMap::const_iterator i = events.upper_bound(cs->playPos());
      for (;;) {
            if (i->second.type() == ME_NOTEON) {
                  const NPlayEvent& n = i->second;
                  if (i->first < tick && n.velo()) {
                        tick = i->first;
                        break;
                        }
                  }
            if (i == events.cbegin())
                  break;
            --i;
            }
      //go the previous chord
      if (i != events.cbegin()) {
            i = playPos;
            for (;;) {
                  if (i->second.type() == ME_NOTEON) {
                        const NPlayEvent& n = i->second;
                        if (i->first < tick && n.velo()) {
                              seek(i->first);
                              break;
                              }
                        }
                  if (i == events.cbegin())
                        break;
                  --i;
                  }
            }
      }

//---------------------------------------------------------
//   seekEnd
//---------------------------------------------------------

void Seq::seekEnd()
      {
      qDebug("seek to end\n");
      }

//---------------------------------------------------------
//   guiToSeq
//---------------------------------------------------------

void Seq::guiToSeq(const SeqMsg& msg)
      {
      if (!_driver || !running)
            return;
      toSeq.enqueue(msg);
      }

//---------------------------------------------------------
//   eventToGui
//---------------------------------------------------------

void Seq::eventToGui(NPlayEvent e)
      {
      fromSeq.enqueue(SeqMsg(SEQ_MIDI_INPUT_EVENT, e));
      }

//---------------------------------------------------------
//   midiInputReady
//---------------------------------------------------------

void Seq::midiInputReady()
      {
      if (_driver)
            _driver->midiRead();
      }

//---------------------------------------------------------
//   SeqMsgFifo
//---------------------------------------------------------

SeqMsgFifo::SeqMsgFifo()
      {
      maxCount = SEQ_MSG_FIFO_SIZE;
      clear();
      }

//---------------------------------------------------------
//   enqueue
//---------------------------------------------------------

void SeqMsgFifo::enqueue(const SeqMsg& msg)
      {
      int i = 0;
      int n = 50;

      QMutex mutex;
      QWaitCondition qwc;
      mutex.lock();
      for (; i < n; ++i) {
            if (!isFull())
                  break;
            qwc.wait(&mutex,100);
            }
      mutex.unlock();
      if (i == n) {
            qDebug("===SeqMsgFifo: overflow\n");
            return;
            }
      messages[widx] = msg;
      push();
      }

//---------------------------------------------------------
//   dequeue
//---------------------------------------------------------

SeqMsg SeqMsgFifo::dequeue()
      {
      SeqMsg msg = messages[ridx];
      pop();
      return msg;
      }

//---------------------------------------------------------
//   putEvent
//---------------------------------------------------------

void Seq::putEvent(const NPlayEvent& event)
      {
      if (!cs)
            return;
      int channel = event.channel();
      if (channel >= cs->midiMapping()->size()) {
            qDebug("bad channel value");
            return;
            }
      int syntiIdx= _synti->index(cs->midiMapping(channel)->articulation->synti);
      _synti->play(event, syntiIdx);
      }

//---------------------------------------------------------
//   heartBeat
//    update GUI
//---------------------------------------------------------

void Seq::heartBeatTimeout()
      {
      SynthControl* sc = mscore->getSynthControl();
      if (sc && _driver) {
            if (++peakTimer[0] >= peakHold)
                  meterPeakValue[0] *= .7f;
            if (++peakTimer[1] >= peakHold)
                  meterPeakValue[1] *= .7f;
            sc->setMeter(meterValue[0], meterValue[1], meterPeakValue[0], meterPeakValue[1]);
            }

      while (!fromSeq.isEmpty()) {
            SeqMsg msg = fromSeq.dequeue();
            if (msg.id == SEQ_MIDI_INPUT_EVENT) {
                  int type = msg.event.type();
                  if (type == ME_NOTEON)
                        mscore->midiNoteReceived(msg.event.channel(), msg.event.pitch(), msg.event.velo());
                  else if (type == ME_NOTEOFF)
                        mscore->midiNoteReceived(msg.event.channel(), msg.event.pitch(), 0);
                  else if (type == ME_CONTROLLER)
                        mscore->midiCtrlReceived(msg.event.controller(), msg.event.value());
                  }
            }

      if (state != TRANSPORT_PLAY)
            return;
      int endTime = playTime;

      mutex.lock();
      auto ppos = playPos;
      if (ppos != events.cbegin())
            --ppos;
      mutex.unlock();

      QRectF r;
      for (;guiPos != events.cend(); ++guiPos) {
            if (guiPos->first > ppos->first)
                  break;
            if (mscore->loop())
                  if (guiPos->first >= cs->loopOutTick())
                        break;
            const NPlayEvent& n = guiPos->second;
            if (n.type() == ME_NOTEON) {
                  const Note* note1 = n.note();
                  if (n.velo()) {
                        while (note1) {
                              note1->setMark(true);
                              markedNotes.append(note1);
                              r |= note1->canvasBoundingRect();
                              note1 = note1->tieFor() ? note1->tieFor()->endNote() : 0;
                              }

                        }
                  else {
                        while (note1) {
                              note1->setMark(false);
                              r |= note1->canvasBoundingRect();
                              markedNotes.removeOne(note1);
                              note1 = note1->tieFor() ? note1->tieFor()->endNote() : 0;
                              }
                        }
                  }
            }
      int utick = ppos->first;
      int tick = cs->repeatList()->utick2tick(utick);
      mscore->currentScoreView()->moveCursor(tick);
      mscore->setPos(tick);

      emit(heartBeat(tick, utick, endTime));

      PianorollEditor* pre = mscore->getPianorollEditor();
      if (pre && pre->isVisible())
            pre->heartBeat(this);
      cv->update(cv->toPhysical(r));
      }

//---------------------------------------------------------
//   updateSynthesizerState
//    collect all controller events between tick1 and tick2
//    and send them to the synthesizer
//---------------------------------------------------------

void Seq::updateSynthesizerState(int tick1, int tick2)
      {
      if (tick1 > tick2)
            tick1 = 0;
      EventMap::const_iterator i1 = events.lower_bound(tick1);
      EventMap::const_iterator i2 = events.upper_bound(tick2);

      for (; i1 != i2; ++i1) {
            if (i1->second.type() == ME_CONTROLLER)
                  playEvent(i1->second);
            }
      }

//---------------------------------------------------------
//   curTempo
//---------------------------------------------------------

double Seq::curTempo() const
      {
      return cs->tempomap()->tempo(playPos->first);
      }

//---------------------------------------------------------
//   set Loop in position
//---------------------------------------------------------

void Seq::setLoopIn()
      {
      int tick;
      if (state == TRANSPORT_PLAY) {      // If in playback mode, set the In position where note is being played
            auto ppos = playPos;
            if (ppos != events.cbegin())
                  --ppos;                 // We have to go back one pos to get the correct note that has just been played
            tick = cs->repeatList()->utick2tick(ppos->first);
            }
      else {
            tick = cs->pos();             // Otherwise, use the selected note.
            }
      if (tick >= cs->loopOutTick())   // If In pos >= Out pos, reset Out pos to end of score
            cs->setLoopOutTick(-1);
      cs->setLoopInTick(tick);
      qDebug ("seq::setLoopIn() : tick = %d\n",tick);
      }

//---------------------------------------------------------
//   set Loop Out position
//---------------------------------------------------------

void Seq::setLoopOut()
      {
      int tick;
      if (state == TRANSPORT_PLAY) {    // If in playback mode, set the Out position where note is being played
            tick = cs->repeatList()->utick2tick(playPos->first);
            }
      else {
            tick = cs->pos()+cs->inputState().ticks();   // Otherwise, use the selected note.
            }
      if (tick <= cs->loopInTick())   // If Out pos <= In pos, reset In pos to beginning of score
            cs->setLoopInTick(-1);
      cs->setLoopOutTick(tick);
      qDebug ("seq::setLoopOut() : loopOutPos = %d  ;  cs->pos() = %d  + cs->inputState().ticks() = %d\n", tick, cs->pos(), cs->inputState().ticks());
      if (state == TRANSPORT_PLAY)
            guiToSeq(SeqMsg(SEQ_SEEK, tick));
      }

//---------------------------------------------------------
//   set Loop In/Out position based on the selection
//---------------------------------------------------------

void Seq::setLoopSelection()
      {
      cs->setLoopInTick(cs->selection().tickStart());
      cs->setLoopOutTick(cs->selection().tickEnd());
      cs->updateLoopCursors();
      qDebug ("setLoopSelection : loopInTick = %d  loopOutTick = %d\n",cs->selection().tickStart(), cs->selection().tickEnd());
      }
}
