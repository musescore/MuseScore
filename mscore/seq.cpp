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

#if 0 // yet(?) unused
static const int AUDIO_BUFFER_SIZE = 1024 * 512;  // 2 MB

static const int MIN_CLICKS   = 3;        // the minimum number of 'clicks' in a count-in
#endif

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
      state    = Transport::STOP;
      oggInit  = false;
      _driver  = 0;
      playPos  = events.cbegin();

      playTime  = 0;
      metronomeVolume = 0.3;
      useJackTransportSavedFlag = false;

      inCountIn         = false;
      countInPlayPos    = countInEvents.cbegin();
      countInPlayTime   = 0;

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

      connect(this, SIGNAL(toGui(int, int)), this, SLOT(seqMessage(int, int)), Qt::QueuedConnection);

      prevTimeSig.setNumerator(0);
      prevTempo = 0;
      connect(this, SIGNAL(timeSigChanged()),this,SLOT(handleTimeSigTempoChanged()));
      connect(this, SIGNAL(tempoChanged()),this,SLOT(handleTimeSigTempoChanged()));
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
      if (cs)
            disconnect(cs, SIGNAL(playlistChanged()), this, SLOT(setPlaylistChanged()));
      cs = cv ? cv->score() : 0;

      if (!heartBeatTimer->isActive())
            heartBeatTimer->start(20);    // msec

      playlistChanged = true;
      _synti->reset();
      if (cs) {
            initInstruments();
            connect(cs, SIGNAL(playlistChanged()), this, SLOT(setPlaylistChanged()));
            }
      }

//---------------------------------------------------------
//   init
//    return false on error
//---------------------------------------------------------

bool Seq::init(bool hotPlug)
      {
      if (!_driver || !_driver->start(hotPlug)) {
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
                  qDebug("Stop I/O");
            stopWait();
            delete _driver;
            _driver = 0;
            }
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
//      qDebug("LoopStart. playPos = %d", playPos);
      }

//---------------------------------------------------------
//   canStart
//    return true if sequencer can be started
//---------------------------------------------------------

bool Seq::canStart()
      {
      if (!_driver)
            return false;
      if (playlistChanged)
            collectEvents();
      return (!events.empty() && endTick != 0);
      }

//---------------------------------------------------------
//   start
//    called from gui thread
//---------------------------------------------------------

void Seq::start()
      {
      if (playlistChanged)
            collectEvents();
      if (cs->playMode() == PlayMode::AUDIO) {
            if (!oggInit) {
                  vorbisData.pos  = 0;
                  vorbisData.data = cs->audio()->data();
                  int n = ov_open_callbacks(&vorbisData, &vf, 0, 0, ovCallbacks);
                  if (n < 0) {
                        qDebug("ogg open failed: %d", n);
                        }
                  oggInit = true;
                  }
            }
      if ((mscore->loop())) {
            if (cs->selection().isRange())
                  setLoopSelection();
            if (!preferences.useJackTransport || (preferences.useJackTransport && state == Transport::STOP))
                  seek(cs->repeatList()->tick2utick(cs->loopInTick()));
            }
      else {
            if (!preferences.useJackTransport || (preferences.useJackTransport && state == Transport::STOP))
                  seek(cs->repeatList()->tick2utick(cs->playPos()));
            }
      if (preferences.useJackTransport && mscore->countIn() && state == Transport::STOP) {
            // Ready to start playing count in, switching to fake transport
            // to prevent playing in other applications with our ticks simultaneously
            useJackTransportSavedFlag    = true;
            preferences.useJackTransport = false;
            }
      _driver->startTransport();
      }

//---------------------------------------------------------
//   stop
//    called from gui thread
//---------------------------------------------------------

void Seq::stop()
      {
      if (state == Transport::STOP)
            return;

      if (oggInit) {
            ov_clear(&vf);
            oggInit = false;
            }
      if (!_driver)
            return;
      if (!preferences.useJackTransport || (preferences.useJackTransport && _driver->getState() == Transport::PLAY))
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
      while (state != Transport::STOP) {
            qDebug("State %d", state);
            mutex.lock();
            sleep.wait(&mutex, 100);
            mutex.unlock();
            ++idx;
            Q_ASSERT(idx <= 10);
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

      cs->setPlayPos(cs->repeatList()->utick2tick(cs->utime2utick(qreal(playTime) / qreal(MScore::sampleRate))));
      cs->end();
      emit stopped();
      }

//---------------------------------------------------------
//   seqSignal
//    sequencer message to GUI
//    execution environment: gui thread
//---------------------------------------------------------

void Seq::seqMessage(int msg, int arg)
      {
      switch(msg) {
            case '5': {
                  // Update the screen after seeking from the realtime thread
                  Segment* seg = cs->tick2segment(arg);
                  if (seg)
                        mscore->currentScoreView()->moveCursor(seg->tick());
                  cs->setPlayPos(arg);
                  cs->end();
                  break;
                  }
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
                  qDebug("MScore::Seq:: unknown seq msg %d", msg);
                  break;
            }
      }

//---------------------------------------------------------
//   playEvent
//    send one event to the synthesizer
//---------------------------------------------------------

void Seq::playEvent(const NPlayEvent& event, unsigned framePos)
      {
      int type = event.type();
      if (type == ME_NOTEON) {
            bool mute;
            const Note* note = event.note();

            if (note) {
                  Instrument* instr = note->staff()->part()->instr(note->chord()->tick());
                  const Channel& a = instr->channel(note->subchannel());
                  mute = a.mute || a.soloMute;
                  }
            else
                  mute = false;

            if (!mute)
                  putEvent(event, framePos);
            }
      else if (type == ME_CONTROLLER)
            putEvent(event, framePos);
      }

//---------------------------------------------------------
//   processMessages
//   from gui to process thread
//---------------------------------------------------------

void Seq::processMessages()
      {
      for (;;) {
            if (toSeq.isEmpty())
                  break;
            SeqMsg msg = toSeq.dequeue();
            switch(msg.id) {
                  case SeqMsgId::TEMPO_CHANGE:
                        {
                        if (!cs)
                              continue;
                        if (playTime != 0) {
                              int utick = cs->utime2utick(qreal(playTime) / qreal(MScore::sampleRate));
                              cs->tempomap()->setRelTempo(msg.realVal);
                              cs->repeatList()->update();
                              playTime = cs->utick2utime(utick) * MScore::sampleRate;
                              if (preferences.jackTimebaseMaster && preferences.useJackTransport)
                                    _driver->seekTransport(utick + 2 * cs->utime2utick(qreal((_driver->bufferSize()) + 1) / qreal(MScore::sampleRate)));
                              }
                        else
                              cs->tempomap()->setRelTempo(msg.realVal);
                        prevTempo = curTempo();
                        emit tempoChanged();
                        }
                        break;
                  case SeqMsgId::PLAY:
                        putEvent(msg.event);
                        break;
                  case SeqMsgId::SEEK:
                        setPos(msg.intVal);
                        break;
                  default:
                        break;
                  }
            }
      }

//---------------------------------------------------------
//   metronome
//---------------------------------------------------------

void Seq::metronome(unsigned n, float* p, bool force)
      {
      if (!mscore->metronome() && !force) {
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
//   addCountInClicks
//---------------------------------------------------------

void Seq::addCountInClicks()
      {
      int         plPos       = playPos->first;
      Measure*    m           = cs->tick2measure(plPos);
      int         msrTick     = m->tick();
      qreal       tempo       = cs->tempomap()->tempo(msrTick);
      Fraction    timeSig     = cs->sigmap()->timesig(msrTick).nominal();
      int         numerator   = timeSig.numerator();
      int         denominator = timeSig.denominator();
      int         clickTicks  = MScore::division * 4 / denominator;
      NPlayEvent  event;
      int         tick;
      bool        triplets    = false;          // whether to play click-clack in triplets or not

      // COMPOUND METER: if time sig is 3*n/d, convert to 3d units
      // note: 3/8, 3/16, ... are NOT considered compound
      if (numerator > 3 && numerator % 3 == 0) {
            // if denominator longer than 1/8 OR tempo for compound unit slower than 60MM
            // (i.e. each denom. unit slower than 180MM = tempo 3.0)
            // then do not count as compound, but beat click-clack-clack triplets
            if (denominator < 8 || tempo * denominator / 4 < 3.0)
                  triplets = true;
            // otherwise, count as compound meter (one beat every 3 denominator units)
            else {
                  numerator   /= 3;
                  clickTicks  *= 3;
                  }
            }

      // NUMBER OF TICKS
      int numOfClicks = numerator;                          // default to a full measure of 'clicks'
      int lastPause   = clickTicks;                         // the number of ticks to wait after the last 'click'
      // if not at the beginning of a measure, add clicks for the initial measure part
      if (msrTick < plPos) {
            int delta    = plPos - msrTick;
            int addClick = (delta + clickTicks - 1) / clickTicks;     // round num. of clicks up
            numOfClicks += addClick;
            lastPause    = delta - (addClick - 1) * clickTicks;       // anything after last click time is final pause
            }
      // or if measure not complete (anacrusis), add clicks for the missing measure part
      else if (m->ticks() < clickTicks * numerator) {
            int delta    = clickTicks * numerator - m->ticks();
            int addClick = (delta + clickTicks - 1) / clickTicks;
            numOfClicks += addClick;
            lastPause    = delta - (addClick - 1) * clickTicks;
            }
/*
      // MIN_CLICKS: be sure to have at least MIN_CLICKS clicks: if less, add full measures
      while (numOfClicks < MIN_CLICKS)
            numOfClicks += numerator;
*/
      // click-clack-clack triplets
      if (triplets)
            numerator = 3;

      // add count-in events
      for (int i = tick = 0; i < numOfClicks; i++, tick += clickTicks) {
            event.setType( (i % numerator) == 0 ? ME_TICK1 : ME_TICK2);
            countInEvents.insert( std::pair<int,NPlayEvent>(tick, event));
            }
      // add 1 empty event at the end to wait after the last click
      tick += lastPause - clickTicks;
      event.setType(ME_INVALID);
      event.setPitch(0);
      countInEvents.insert( std::pair<int,NPlayEvent>(tick, event));
      // initialize play parameters to count-in events
      countInPlayPos  = countInEvents.cbegin();
      countInPlayTime = 0;
      }

//-------------------------------------------------------------------
//   process
//    This function is called in a realtime context. This
//    means that no blocking operations are allowed which
//    includes memory allocation. The usual thread synchronisation
//    methods like semaphores can also not be used.
//-------------------------------------------------------------------

void Seq::process(unsigned n, float* buffer)
      {
      unsigned frames = n;
      Transport driverState = _driver->getState();
      // Checking for the reposition from JACK Transport
      _driver->checkTransportSeek(playTime, frames, inCountIn);
      if (driverState != state) {
            // Got a message from JACK Transport panel: Play
            if (state == Transport::STOP && driverState == Transport::PLAY) {
                  if ((preferences.useJackMidi || preferences.useJackAudio) && !getAction("play")->isChecked()) {
                        // Do not play while editing elements
                        if (mscore->state() != STATE_NORMAL || !isRunning() || !canStart())
                              return;
                        getAction("play")->setChecked(true);
                        getAction("play")->triggered(true);

                        // If we just launch MuseScore and press "Play" on JACK Transport with time 0:00
                        // MuseScore doesn't seek to 0 and guiPos is uninitialized, so let's make it manually
                        if (preferences.useJackTransport && getCurTick() == 0)
                              seekRT(0);

                        // Switching to fake transport while playing count in
                        // to prevent playing in other applications with our ticks simultaneously
                        if (preferences.useJackTransport && mscore->countIn()) {
                              // Stopping real JACK Transport
                              _driver->stopTransport();
                              // Starting fake transport
                              useJackTransportSavedFlag = preferences.useJackTransport;
                              preferences.useJackTransport = false;
                              _driver->startTransport();
                              }
                        }
                  // Need to change state after calling collectEvents()
                  state = Transport::PLAY;
                  if (mscore->countIn() && cs->playMode() == PlayMode::SYNTHESIZER) {
                        countInEvents.clear();
                        inCountIn = true;
                        }
                  emit toGui('1');
                  }
            // Got a message from JACK Transport panel: Stop
            else if (state == Transport::PLAY && driverState == Transport::STOP) {
                  state = Transport::STOP;
                  // Muting all notes
                  stopNotes();
                  if (playPos == events.cend()) {
                        if (mscore->loop()) {
                              qDebug("Seq.cpp - Process - Loop whole score. playPos = %d     cs->pos() = %d", playPos->first,cs->pos());
                              emit toGui('4');
                              return;
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
                  qDebug("Seq: state transition %d -> %d ?",
                     state, driverState);
            }

      memset(buffer, 0, sizeof(float) * n * 2);
      float* p = buffer;

      processMessages();

      if (state == Transport::PLAY) {
            if (!cs)
                  return;
            EventMap::const_iterator* pPlayPos = &playPos;
            EventMap* pEvents   = &events;
            int*      pPlayTime = &playTime;
            //
            // in count-in?
            //
            if (inCountIn) {
                  if (countInEvents.size() == 0)
                        addCountInClicks();
                  pEvents   = &countInEvents;
                  pPlayPos  = &countInPlayPos;
                  pPlayTime = &countInPlayTime;
                  }
            //
            // play events for one segment
            //
            unsigned framePos = 0;
            int endTime = *pPlayTime + frames;
            int utickEnd = cs->repeatList()->tick2utick(cs->lastMeasure()->endTick()) - 1;
            for ( ; *pPlayPos != pEvents->cend(); ) {
                  int n;
                  if (inCountIn) {

                        qreal bps = curTempo() * cs->tempomap()->relTempo();
                        // relTempo needed here to ensure that bps changes as we slide the tempo bar

                        qreal tickssec = bps * MScore::division;
                        qreal secs = (*pPlayPos)->first / tickssec;
                        int f = secs * MScore::sampleRate;
                        if (f >= endTime)
                              break;
                        n = f - *pPlayTime;
                        if (n < 0) {
                              qDebug("Count-in: %d:  %d - %d", (*pPlayPos)->first, f, *pPlayTime);
                              n = 0;
                              }
                        }
                  else {
                        int f = cs->utick2utime(playPos->first) * MScore::sampleRate;
                        if (f >= endTime)
                              break;
                        n = f - *pPlayTime;
                        if (n < 0) {
                              qDebug("%d:  %d - %d", (*pPlayPos)->first, f, *pPlayTime);
                              n = 0;
                              }
                        if (mscore->loop()) {
                              int utickLoop = cs->repeatList()->tick2utick(cs->loopOutTick());
                              if (utickLoop < utickEnd)
                                    if ((*pPlayPos)->first >= utickLoop) {
                                          qDebug ("Process playPos = %d  in/out tick = %d/%d  getCurTick() = %d   tickLoop = %d   playTime = %d",
                                             (*pPlayPos)->first, cs->loopInTick(), cs->loopOutTick(), getCurTick(), utickLoop, *pPlayTime);
                                          if (preferences.useJackTransport) {
                                                int loopInUtick = cs->repeatList()->tick2utick(cs->loopInTick());
                                                _driver->seekTransport(loopInUtick);
                                                if (loopInUtick != 0) {
                                                      int seekto = loopInUtick - 2 * cs->utime2utick((qreal)_driver->bufferSize() / MScore::sampleRate);
                                                      seekRT((seekto > 0) ? seekto : 0 );
                                                      }
                                                }
                                          else {
                                                emit toGui('3');
                                                }
                                          // Exit this function to avoid segmentation fault in Scoreview
                                          return;
                                          }
                              }
                        }
                  if (n) {
                        if (cs->playMode() == PlayMode::SYNTHESIZER) {
                              metronome(n, p, inCountIn);
                              _synti->process(n, p);
                              p += n * 2;
                              *pPlayTime  += n;
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
                                    *pPlayTime += rn;
                                    frames   -= rn;
                                    framePos += rn;
                                    n        -= rn;
                                    }
                              }
                        }
                  const NPlayEvent& event = (*pPlayPos)->second;
                  playEvent(event, framePos);
                  if (event.type() == ME_TICK1)
                        tickRest = tickLength;
                  else if (event.type() == ME_TICK2)
                        tackRest = tackLength;
                  mutex.lock();
                  ++(*pPlayPos);
                  mutex.unlock();
                  }
            if (frames) {
                  if (cs->playMode() == PlayMode::SYNTHESIZER) {
                        metronome(frames, p, inCountIn);
                        _synti->process(frames, p);
                        *pPlayTime += frames;
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
                              *pPlayTime += rn;
                              frames     -= rn;
                              framePos   += rn;
                              n          -= rn;
                              }
                        }
                  }
            if (*pPlayPos == pEvents->cend()) {
                  if (inCountIn) {
                        inCountIn = false;
                        // Connecting to JACK Transport if MuseScore was temporarily disconnected from it
                        if (useJackTransportSavedFlag) {
                              // Stopping fake driver
                              _driver->stopTransport();
                              preferences.useJackTransport = true;
                              // Starting the real JACK Transport. All applications play in sync now
                              _driver->startTransport();
                              }
                        }
                  else
                        _driver->stopTransport();
                  }
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
      if (state ==  Transport::PLAY)
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
      guiToSeq(SeqMsg(SeqMsgId::TEMPO_CHANGE, relTempo));
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
//   seekCommon
//   a common part of seek() and seekRT(), contains code
//   that could be safely called from any thread.
//   Do not use explicitly, use seek() or seekRT()
//---------------------------------------------------------

void Seq::seekCommon(int utick)
      {
      if (cs == 0)
            return;

      if (playlistChanged)
            collectEvents();

      if (cs->playMode() == PlayMode::AUDIO) {
            ogg_int64_t sp = cs->utick2utime(utick) * MScore::sampleRate;
            ov_pcm_seek(&vf, sp);
            }

      guiPos = events.lower_bound(utick);
      mscore->setPos(cs->repeatList()->utick2tick(utick));
      unmarkNotes();
      }

//---------------------------------------------------------
//   seek
//   send seek message to sequencer
//   gui thread
//---------------------------------------------------------

void Seq::seek(int utick)
      {
      if (preferences.useJackTransport) {
            if (utick > endTick)
                  utick = 0;
            _driver->seekTransport(utick);
            if (utick != 0)
                  return;
            }
      seekCommon(utick);

      int tick = cs->repeatList()->utick2tick(utick);
      Segment* seg = cs->tick2segment(tick);
      if (seg)
            mscore->currentScoreView()->moveCursor(seg->tick());
      cs->setPlayPos(tick);
      cs->end();
      guiToSeq(SeqMsg(SeqMsgId::SEEK, utick));
      }

//---------------------------------------------------------
//   seekRT
//   realtime thread
//---------------------------------------------------------

void Seq::seekRT(int utick)
      {
      if (preferences.useJackTransport && utick > endTick)
                  utick = 0;
      seekCommon(utick);
      setPos(utick);
      // Update the screen in GUI thread
      emit toGui('5', cs->repeatList()->utick2tick(utick));
      }

//---------------------------------------------------------
//   startNote
//---------------------------------------------------------

void Seq::startNote(int channel, int pitch, int velo, double nt)
      {
      if (state != Transport::STOP)
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
      // Stop motes in all channels
      if (channel == -1) {
            for(int ch=0; ch<cs->midiMapping()->size();ch++) {
                  putEvent(NPlayEvent(ME_CONTROLLER, ch, CTRL_SUSTAIN, 0));
                  for(int i=0; i<128; i++)
                        putEvent(NPlayEvent(ME_NOTEOFF,ch,i,0));
                  }
            }
      else {
            putEvent(NPlayEvent(ME_CONTROLLER, channel, CTRL_SUSTAIN, 0));
            for(int i=0; i<128; i++)
                  putEvent(NPlayEvent(ME_NOTEOFF,channel,i,0));
            }
      if (preferences.useAlsaAudio || preferences.useJackAudio || preferences.usePulseAudio || preferences.usePortaudioAudio)
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
      guiToSeq(SeqMsg(SeqMsgId::PLAY, ev));
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
      EventMap::const_iterator i = events.upper_bound(cs->repeatList()->tick2utick(tick));
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
      qDebug("seek to end");
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
      fromSeq.enqueue(SeqMsg(SeqMsgId::MIDI_INPUT_EVENT, e));
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
            qDebug("===SeqMsgFifo: overflow");
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

void Seq::putEvent(const NPlayEvent& event, unsigned framePos)
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
      if (preferences.useJackMidi && _driver != 0)
            _driver->putEvent(event, framePos);
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
            if (msg.id == SeqMsgId::MIDI_INPUT_EVENT) {
                  int type = msg.event.type();
                  if (type == ME_NOTEON)
                        mscore->midiNoteReceived(msg.event.channel(), msg.event.pitch(), msg.event.velo());
                  else if (type == ME_NOTEOFF)
                        mscore->midiNoteReceived(msg.event.channel(), msg.event.pitch(), 0);
                  else if (type == ME_CONTROLLER)
                        mscore->midiCtrlReceived(msg.event.controller(), msg.event.value());
                  }
            }

      if (state != Transport::PLAY || inCountIn)
            return;

      int endTime = playTime;

      mutex.lock();
      auto ppos = playPos;
      if (ppos != events.cbegin())
            --ppos;
      mutex.unlock();

      if (cs && cs->sigmap()->timesig(getCurTick()).nominal()!=prevTimeSig) {
            prevTimeSig = cs->sigmap()->timesig(getCurTick()).nominal();
            emit timeSigChanged();
            }
      if (cs && curTempo()!=prevTempo) {
            prevTempo = curTempo();
            emit tempoChanged();
            }

      QRectF r;
      for (;guiPos != events.cend(); ++guiPos) {
            if (guiPos->first > ppos->first)
                  break;
            if (mscore->loop())
                  if (guiPos->first >= cs->repeatList()->tick2utick(cs->loopOutTick()))
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
                  playEvent(i1->second, 0);
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
      if (state == Transport::PLAY) {      // If in playback mode, set the In position where note is being played
            auto ppos = playPos;
            if (ppos != events.cbegin())
                  --ppos;                 // We have to go back one pos to get the correct note that has just been played
            tick = cs->repeatList()->utick2tick(ppos->first);
            }
      else
            tick = cs->pos();             // Otherwise, use the selected note.
      if (tick >= cs->loopOutTick())   // If In pos >= Out pos, reset Out pos to end of score
            cs->setPos(POS::RIGHT, cs->lastMeasure()->endTick() - 1);
      cs->setPos(POS::LEFT, tick);
      }

//---------------------------------------------------------
//   set Loop Out position
//---------------------------------------------------------

void Seq::setLoopOut()
      {
      int tick;
      if (state == Transport::PLAY) {    // If in playback mode, set the Out position where note is being played
            tick = cs->repeatList()->utick2tick(playPos->first);
            }
      else
            tick = cs->pos() + cs->inputState().ticks();   // Otherwise, use the selected note.
      if (tick <= cs->loopInTick())   // If Out pos <= In pos, reset In pos to beginning of score
            cs->setPos(POS::LEFT, 0);
      cs->setPos(POS::RIGHT, tick);
      if (state == Transport::PLAY)
            guiToSeq(SeqMsg(SeqMsgId::SEEK, tick));
      }

void Seq::setPos(POS, unsigned tick)
      {
      qDebug("seq: setPos %d", tick);
      }

//---------------------------------------------------------
//   set Loop In/Out position based on the selection
//---------------------------------------------------------

void Seq::setLoopSelection()
      {
      cs->setLoopInTick(cs->selection().tickStart());
      cs->setLoopOutTick(cs->selection().tickEnd());
      }

//---------------------------------------------------------
//   Called after tempo or time signature
//   changed while playback
//---------------------------------------------------------

void Seq::handleTimeSigTempoChanged()
      {
      _driver->handleTimeSigTempoChanged();
      }
}
