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

#ifdef USE_ALSA
#include "alsa.h"
#endif
#ifdef USE_PORTAUDIO
#include "pa.h"
#endif

#include "msynth/synti.h"
#include "libmscore/slur.h"
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

#ifdef USE_JACK
#include "jackaudio.h"
#endif

#ifdef AEOLUS
#include "aeolus/aeolus/aeolus.h"
#endif

#include "fluid/fluid.h"
#include "click.h"

#include <vorbis/vorbisfile.h>

#ifdef USE_PULSEAUDIO
extern Driver* getPulseAudioDriver(Seq*);
#endif

Seq* seq;
MasterSynth* synti;

static const int guiRefresh   = 10;       // Hz
static const int peakHoldTime = 1400;     // msec
static const int peakHold     = (peakHoldTime * guiRefresh) / 1000;
static OggVorbis_File vf;

static const int AUDIO_BUFFER_SIZE = 1024 * 512;  // 2 MB

//---------------------------------------------------------
//   RenderAudio
//    the prerendering audio thread
//---------------------------------------------------------

class RenderAudio : public QThread
      {
      float fifo[AUDIO_BUFFER_SIZE];
      int ridx;               // read index
      int widx;               // write index
      volatile int counter;   // objects in fifo

      void clear() {
	      ridx    = 0;
            widx    = 0;
            counter = 0;
            }

      void push() {
//            widx = (widx + 1) % maxCount;
//            ++counter;
            }

      void pop() {
//            ridx = (ridx + 1) % maxCount;
//            --counter;
            }

   public:
      RenderAudio() : QThread() {
            }
      void run() {
            }
      };

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

      endTick  = 0;
      state    = TRANSPORT_STOP;
      oggInit  = false;
      driver   = 0;
      playPos  = events.constBegin();

      playTime  = 0;
      metronomeVolume = 0.3;

      meterValue[0]     = 0.0;
      meterValue[1]     = 0.0;
      meterPeakValue[0] = 0.0;
      meterPeakValue[1] = 0.0;
      peakTimer[0]       = 0;
      peakTimer[1]       = 0;

      heartBeatTimer = new QTimer(this);
      connect(heartBeatTimer, SIGNAL(timeout()), this, SLOT(heartBeat()));

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
      delete synti;
      delete driver;
      }

//---------------------------------------------------------
//   stopWait
//---------------------------------------------------------

void Seq::stopWait()
      {
      stop();
      QMutex mutex;
      QWaitCondition sleep;
      while (state != TRANSPORT_STOP) {
            mutex.lock();
            sleep.wait(&mutex, 100);
            mutex.unlock();
            }
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
            cs->setSyntiState(synti->state());
            markedNotes.clear();
            stopWait();
            }
      cv = v;
      cs = cv ? cv->score() : 0;

      if (!heartBeatTimer->isActive())
            heartBeatTimer->start(20);    // msec

      playlistChanged = true;
      synti->reset();
      if (cs) {
            synti->setState(cs->syntiState());
            initInstruments();
            seek(cs->playPos());
            }
      tackRest = 0;
      tickRest = 0;
      }

//---------------------------------------------------------
//   selectionChanged
//---------------------------------------------------------

void Seq::selectionChanged(int mode)
      {
      if (cs == 0 || driver == 0)
            return;

      int tick = cs->pos();
      if (tick == -1)
            return;

      if ((mode != SEL_LIST) || (state == TRANSPORT_STOP))
            cs->setPlayPos(tick);
      else {
            seek(tick);
            }
      }

//---------------------------------------------------------
//   init
//    return false on error
//---------------------------------------------------------

bool Seq::init()
      {
      driver = 0;

      bool useJackFlag      = preferences.useJackAudio || preferences.useJackMidi;
      bool useAlsaFlag      = preferences.useAlsaAudio;
#ifdef USE_PORTAUDIO
      bool usePortaudioFlag = preferences.usePortaudioAudio;
#endif
#ifdef USE_PULSEAUDIO
      bool usePulseAudioFlag = preferences.usePulseAudio;
#endif

#ifdef USE_PULSEAUDIO
      if (MScore::debugMode)
            qDebug("usePulseAudioFlag %d\n", usePulseAudioFlag);
      if (usePulseAudioFlag) {
            driver = getPulseAudioDriver(this);
            if (!driver->init()) {
                  qDebug("init PulseAudio failed");
                  delete driver;
                  driver = 0;
                  }
            else
                  usePortaudio = true;
            }
#endif
#ifdef USE_PORTAUDIO
      if (MScore::debugMode)
            qDebug("usePortaudioFlag %d\n", usePortaudioFlag);
      if (usePortaudioFlag) {
            driver = new Portaudio(this);
            if (!driver->init()) {
                  qDebug("init PortAudio failed");
                  delete driver;
                  driver = 0;
                  }
            else
                  usePortaudio = true;
            }
#endif
#ifdef USE_ALSA
      if (MScore::debugMode)
            qDebug("useAlsaFlag %d\n", useAlsaFlag);
      if (driver == 0 && useAlsaFlag) {
            driver = new AlsaAudio(this);
            if (!driver->init()) {
                  qDebug("init ALSA driver failed\n");
                  delete driver;
                  driver = 0;
                  }
            else {
                  useALSA = true;
                  }
            }
#endif

#ifdef USE_JACK
      if (MScore::debugMode)
            qDebug("useJackFlag %d\n", useJackFlag);
      if (useJackFlag) {
            useAlsaFlag      = false;
#ifdef USE_PORTAUDIO
            usePortaudioFlag = false;
#endif
            driver = new JackAudio(this);
            if (!driver->init()) {
                  qDebug("no JACK server found\n");
                  delete driver;
                  driver = 0;
                  }
            else
                  useJACK = true;
            }
#endif

      if (driver == 0) {
#if 0
            QString s = tr("Init audio driver failed.\n"
                                "Sequencer will be disabled.");
            QMessageBox::critical(0, "MuseScore: Init Audio Driver", s);
#endif
            qDebug("init audio driver failed");
            return false;
            }
      MScore::sampleRate = driver->sampleRate();
      synti->init(MScore::sampleRate);

      if (!driver->start()) {
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
      if (driver) {
            if (MScore::debugMode)
                  qDebug("Stop I/O\n");
            stopWait();
            delete driver;
            driver = 0;
            }
      }

//---------------------------------------------------------
//   inputPorts
//---------------------------------------------------------

QList<QString> Seq::inputPorts()
      {
      if (driver)
            return driver->inputPorts();
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
//   canStart
//    return true if sequencer can be started
//---------------------------------------------------------

bool Seq::canStart()
      {
      if (!driver)
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
      seek(cs->playPos());
      driver->startTransport();
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
      if (!driver)
            return;
      driver->stopTransport();
      if (cv)
            cv->setCursorOn(false);
      if (cs) {
            cs->setPlayPos(playPos.key());
            cs->setLayoutAll(false);
            cs->setUpdateAll();
            cs->end();
            }
      }

//---------------------------------------------------------
//   seqStarted
//---------------------------------------------------------

void MuseScore::seqStarted()
      {
      if(cv)
            cv->setCursorOn(true);
      if(cs)
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
//   guiStop
//---------------------------------------------------------

void Seq::guiStop()
      {
      QAction* a = getAction("play");
      a->setChecked(false);

      if (!cs) {
            markedNotes.clear();
            return;
            }

      //
      // deselect all selected notes
      //
      foreach(const Note* n, markedNotes) {
            ((Note*)n)->setSelected(false);     //HACK
            cs->addRefresh(n->canvasBoundingRect());
            }
      markedNotes.clear();
      cs->setPlayPos(cs->utime2utick(playTime) * MScore::sampleRate);
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
            case '0':         // STOP
                  guiStop();
//                  heartBeatTimer->stop();
                  if (driver && mscore->getSynthControl()) {
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
//   stopTransport
//    JACK has stopped
//    executed in realtime environment
//---------------------------------------------------------

void Seq::stopTransport()
      {
      state = TRANSPORT_STOP;
      if (cs == 0)
            return;
      stopNotes();
      // send sustain off
      Event e;
      e.setType(ME_CONTROLLER);
      e.setController(CTRL_SUSTAIN);
      e.setValue(0);
      putEvent(e);
      emit toGui('0');
      }

//---------------------------------------------------------
//   startTransport
//    JACK has started
//    executed in realtime environment
//---------------------------------------------------------

void Seq::startTransport()
      {
      emit toGui('1');
      state = TRANSPORT_PLAY;
      }

//---------------------------------------------------------
//   playEvent
//    send one event to the synthesizer
//---------------------------------------------------------

void Seq::playEvent(const Event& event)
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
                              cs->tempomap()->setRelTempo(msg.data.realVal);
                              cs->repeatList()->update();
                              playTime = cs->utick2utime(tick) * MScore::sampleRate;
                              }
                        else
                              cs->tempomap()->setRelTempo(msg.data.realVal);
                        }
                        break;
                  case SEQ_PLAY:
                        putEvent(msg.event);
                        break;
                  case SEQ_SEEK:
                        setPos(msg.data.intVal);
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
      int driverState = driver->getState();

      if (driverState != state) {
            if (state == TRANSPORT_STOP && driverState == TRANSPORT_PLAY)
                  startTransport();
            else if (state == TRANSPORT_PLAY && driverState == TRANSPORT_STOP)
                  stopTransport();
            else if (state != driverState)
                  qDebug("Seq: state transition %d -> %d ?\n",
                     state, driverState);
            }

      float* p = buffer;
      memset(p, 0, sizeof(float) * n * 2);
      processMessages();

      if (state == TRANSPORT_PLAY) {
            //
            // play events for one segment
            //
            unsigned framePos = 0;
            int endTime = playTime + frames;
            for (; playPos != events.constEnd(); ++playPos) {
                  int f = cs->utick2utime(playPos.key()) * MScore::sampleRate;
                  if (f >= endTime)
                        break;
                  int n = f - playTime;
                  if (n < 0) {
                        qDebug("%d:  %d - %d\n", playPos.key(), f, playTime);
      			n = 0;
                        }
                  if (n) {
                        if (cs->playMode() == PLAYMODE_SYNTHESIZER) {
                              metronome(n, p);
                              synti->process(n, p);
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
                  const Event& event = playPos.value();
                  playEvent(event);
                  if (event.type() == ME_TICK1)
                        tickRest = tickLength;
                  else if (event.type() == ME_TICK2)
                        tackRest = tackLength;
                  }
            if (frames) {
                  if (cs->playMode() == PLAYMODE_SYNTHESIZER) {
                        metronome(frames, p);
                        synti->process(frames, p);
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
            if (playPos == events.constEnd()) {
                  driver->stopTransport();
                  rewindStart();
                  }
            }
      else {
            synti->process(frames, p);
            }
      //
      // metering
      //
      float lv = 0.0f;
      float rv = 0.0f;
      for (unsigned i = 0; i < n; ++i) {
            lv = qMax(lv, fabsf(buffer[i * 2]));
            rv = qMax(rv, fabsf(buffer[i * 2 + 1]));
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
            foreach(Event e, channel->init) {
                  if (e.type() == ME_INVALID)
                        continue;
                  e.setChannel(channel->channel);
                  sendEvent(e);
                  }
            }
      }

//---------------------------------------------------------
//   collectEvents
//---------------------------------------------------------

void Seq::collectEvents()
      {
      events.clear();

      cs->toEList(&events);
      endTick = 0;
      if (!events.empty()) {
            EventMap::const_iterator e = events.constEnd();
            --e;
            endTick = e.key();
            }

      PlayPanel* pp = mscore->getPlayPanel();
      if (pp)
            pp->setEndpos(endTick);
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
//   processToGuiMessages
//    process messages from sequencer to gui
//    in gui context
//---------------------------------------------------------

void Seq::processToGuiMessages()
      {
      for (;;) {
            if (fromSeq.isEmpty())
                  break;
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
      }

//---------------------------------------------------------
//   setRelTempo
//    relTempo = 1.0 = normal tempo
//---------------------------------------------------------

void Seq::setRelTempo(double relTempo)
      {
      SeqMsg msg;
      msg.data.realVal = relTempo;
      msg.id    = SEQ_TEMPO_CHANGE;
      guiToSeq(msg);

      double t = cs->tempomap()->tempo(playPos.key()) * relTempo;

      PlayPanel* pp = mscore->getPlayPanel();
      if (pp) {
            pp->setTempo(t);
            pp->setRelTempo(relTempo);
            }
      }

//---------------------------------------------------------
//   setPos
//    seek
//    realtime environment
//---------------------------------------------------------

void Seq::setPos(int utick)
      {
      stopNotes();

      playTime  = cs->utick2utime(utick) * MScore::sampleRate;
      playPos   = events.lowerBound(utick);
      guiPos    = playPos;
      }

//---------------------------------------------------------
//   seek
//    send seek message to sequencer
//---------------------------------------------------------

void Seq::seek(int tick)
      {
      if (cs == 0)
            return;
      Segment* seg = cs->tick2segment(tick);
      if (seg) {
            mscore->currentScoreView()->moveCursor(seg, -1);
            }
      cs->setPlayPos(tick);
      cs->setLayoutAll(false);
      cs->end();

      tick = cs->repeatList()->tick2utick(tick);
      if (cs->playMode() == PLAYMODE_AUDIO) {
            ogg_int64_t sp = cs->utick2utime(tick) * MScore::sampleRate;
            ov_pcm_seek(&vf, sp);
            }

      SeqMsg msg;
      msg.data.intVal = tick;
      msg.id   = SEQ_SEEK;
      guiToSeq(msg);
      mscore->setPos(tick);
      foreach(const Note* n, markedNotes) {
            ((Note*)n)->setSelected(false);     // HACK
            cs->addRefresh(n->canvasBoundingRect());
            }
      }

//---------------------------------------------------------
//   startNote
//---------------------------------------------------------

void Seq::startNote(const Channel& a, int pitch, int velo, double nt)
      {
      if (state != TRANSPORT_STOP)
            return;
      Event ev(ME_NOTEON);
      ev.setChannel(a.channel);
      ev.setPitch(pitch);
      ev.setTuning(nt);
      ev.setVelo(velo);
      sendEvent(ev);
      }

void Seq::startNote(const Channel& a, int pitch, int velo, int duration, double nt)
      {
      stopNotes();
      startNote(a, pitch, velo, nt);
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
//    called from GUI context
//---------------------------------------------------------

void Seq::stopNotes(int channel)
      {
      synti->allNotesOff(channel);
      }

//---------------------------------------------------------
//   setController
//---------------------------------------------------------

void Seq::setController(int channel, int ctrl, int data)
      {
      Event event(ME_CONTROLLER);
      event.setChannel(channel);
      event.setController(ctrl);
      event.setValue(data);
      sendEvent(event);
      }

//---------------------------------------------------------
//   sendEvent
//    called from GUI context to send a midi event to
//    midi out or synthesizer
//---------------------------------------------------------

void Seq::sendEvent(const Event& ev)
      {
      SeqMsg msg;
      msg.id    = SEQ_PLAY;
      msg.event = ev;
      guiToSeq(msg);
      }

//---------------------------------------------------------
//   nextMeasure
//---------------------------------------------------------

void Seq::nextMeasure()
      {
      EventMap::const_iterator i = playPos;
      const Note* note = 0;
      for (;;) {
            if (i.value().type() == ME_NOTEON) {
                  const Event& n = i.value();
                  note = n.note();
                  break;
                  }
            if (i == events.begin())
                  break;
            --i;
            }
      if (!note)
            return;
      Measure* m = note->chord()->measure();
      m = m->nextMeasure();
      if (m) {
            int rtick = m->tick() - note->chord()->tick();
            seek(playPos.key() + rtick);
            }
      }

//---------------------------------------------------------
//   nextChord
//---------------------------------------------------------

void Seq::nextChord()
      {
      int tick = playPos.key();
      for (EventMap::const_iterator i = playPos; i != events.constEnd(); ++i) {
            if (i.value().type() != ME_NOTEON)
                  continue;
            const Event& n = i.value();
            if (i.key() > tick && n.velo()) {
                  seek(i.key());
                  break;
                  }
            }
      }

//---------------------------------------------------------
//   prevMeasure
//---------------------------------------------------------

void Seq::prevMeasure()
      {
      EventMap::const_iterator i = playPos;
      const Note* note = 0;
      for (;;) {
            if (i.value().type() == ME_NOTEON) {
                  note = i.value().note();
                  break;
                  }
            if (i == events.begin())
                  break;
            --i;
            }
      if (!note)
            return;
      Measure* m = note->chord()->measure();
      m = m->prevMeasure();

      if (m) {
            int rtick = note->chord()->tick() - m->tick();
            seek(playPos.key() - rtick);
            }
      else
            seek(0);
      }

//---------------------------------------------------------
//   prevChord
//---------------------------------------------------------

void Seq::prevChord()
      {
      int tick  = playPos.key();
      //find the chord just before playpos
      EventMap::const_iterator i = playPos;
      for (;;) {
            if (i.value().type() == ME_NOTEON) {
                  const Event& n = i.value();
                  if (i.key() < tick && n.velo()) {
                        tick = i.key();
                        break;
                        }
                  }
            if (i == events.constBegin())
                  break;
            --i;
            }
      //go the previous chord
      if (i != events.constBegin()) {
            i = playPos;
            for (;;) {
                  if (i.value().type() == ME_NOTEON) {
                        const Event& n = i.value();
                        if (i.key() < tick && n.velo()) {
                              seek(i.key());
                              break;
                              }
                        }
                  if (i == events.constBegin())
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
      if (!driver || !running)
            return;
      toSeq.enqueue(msg);
      }

//---------------------------------------------------------
//   eventToGui
//---------------------------------------------------------

void Seq::eventToGui(Event e)
      {
      SeqMsg msg;
      msg.event = e;
      msg.id    = SEQ_MIDI_INPUT_EVENT;
      fromSeq.enqueue(msg);
      }

//---------------------------------------------------------
//   getPatchInfo
//---------------------------------------------------------

QList<MidiPatch*> Seq::getPatchInfo() const
      {
      return synti->getPatchInfo();
      }

//---------------------------------------------------------
//   midiInputReady
//---------------------------------------------------------

void Seq::midiInputReady()
      {
      if (driver)
            driver->midiRead();
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
//   setGain
//---------------------------------------------------------

void Seq::setGain(float gain)
      {
      synti->setGain(gain);
      emit gainChanged(gain);
      }

//---------------------------------------------------------
//   gain
//---------------------------------------------------------

float Seq::gain() const
      {
      return synti->gain();
      }

//---------------------------------------------------------
//   putEvent
//---------------------------------------------------------

void Seq::putEvent(const Event& event)
      {
      if (!cs)
            return;
      int channel = event.channel();
      int syntiIdx= cs->midiMapping(channel)->articulation->synti;
      synti->play(event, syntiIdx);
      }

//---------------------------------------------------------
//   synthNameToIndex
//---------------------------------------------------------

int Seq::synthNameToIndex(const QString& name) const
      {
      return synti->synthNameToIndex(name);
      }

//---------------------------------------------------------
//   synthIndexToName
//---------------------------------------------------------

QString Seq::synthIndexToName(int idx) const
      {
      return synti->synthIndexToName(idx);
      }

//---------------------------------------------------------
//   heartBeat
//    update GUI
//---------------------------------------------------------

void Seq::heartBeat()
      {
      SynthControl* sc = mscore->getSynthControl();
      if (sc && driver) {
            if (++peakTimer[0] >= peakHold)
                  meterPeakValue[0] *= .7f;
            if (++peakTimer[1] >= peakHold)
                  meterPeakValue[1] *= .7f;
            sc->setMeter(meterValue[0], meterValue[1], meterPeakValue[0], meterPeakValue[1]);
            }
      processToGuiMessages();
      if (state != TRANSPORT_PLAY)
            return;
      PlayPanel* pp = mscore->getPlayPanel();
      int endTime = playTime;
      if (pp)
            pp->heartBeat2(endTime);

      for (;;) {
            EventMap::const_iterator p = guiPos + 1;
            if ((p == events.constEnd()) || (p.key() >= playPos.key()))
                  break;
            guiPos = p;
            if (guiPos.value().type() == ME_NOTEON) {
                  Event n = guiPos.value();
                  const Note* note1 = n.note();
                  if (n.velo()) {
                        while (note1) {
                              ((Note*)note1)->setSelected(true);  // HACK
                              markedNotes.append(note1);
                              cs->addRefresh(note1->canvasBoundingRect());
                              note1 = note1->tieFor() ? note1->tieFor()->endNote() : 0;
                              }

                        }
                  else {
                        while (note1) {
                              ((Note*)note1)->setSelected(false);       // HACK
                              cs->addRefresh(note1->canvasBoundingRect());
                              markedNotes.removeOne(note1);
                              note1 = note1->tieFor() ? note1->tieFor()->endNote() : 0;
                              }
                        }
                  }
            }

      int tick = cs->repeatList()->utick2tick(guiPos.key());
      mscore->currentScoreView()->moveCursor(tick);
      mscore->setPos(tick);
      if (pp)
            pp->heartBeat(tick, playPos.key());

      PianorollEditor* pre = mscore->getPianorollEditor();
      if (pre && pre->isVisible())
            pre->heartBeat(this);
      cs->end();
      }

