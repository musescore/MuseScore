//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#ifndef __SEQ_H__
#define __SEQ_H__

#include "libmscore/rendermidi.h"
#include "libmscore/sequencer.h"
#include "libmscore/fraction.h"
#include "synthesizer/event.h"
#include "driver.h"
#include "libmscore/fifo.h"
#include "libmscore/tempo.h"

class QTimer;

namespace Ms {

class Note;
class MasterScore;
class Score;
class Painter;
class Measure;
class Fraction;
class Driver;
class Part;
class Channel;
class ScoreView;
class MasterSynthesizer;
class Segment;
enum class POS : char;

//---------------------------------------------------------
//   SeqMsg
//    message format for gui -> sequencer messages
//---------------------------------------------------------

enum class SeqMsgId : char {
      NO_MESSAGE,
      TEMPO_CHANGE,
      PLAY, SEEK,
      MIDI_INPUT_EVENT
      };

struct SeqMsg {
      SeqMsgId id;
      union {
            int intVal;
            qreal realVal;
            };
      NPlayEvent event;

      SeqMsg() {}
      SeqMsg(SeqMsgId _id, int val) : id(_id), intVal(val) {}
      SeqMsg(SeqMsgId _id, qreal val) : id(_id), realVal(val) {}
      SeqMsg(SeqMsgId _id, const NPlayEvent& e) : id(_id), event(e) {}
      };

//---------------------------------------------------------
//   SeqMsgFifo
//---------------------------------------------------------

static const int SEQ_MSG_FIFO_SIZE = 1024*8;

class SeqMsgFifo : public FifoBase {
      SeqMsg messages[SEQ_MSG_FIFO_SIZE];

   public:
      SeqMsgFifo();
      virtual ~SeqMsgFifo()     {}
      void enqueue(const SeqMsg&);        // put object on fifo
      SeqMsg dequeue();                   // remove object from fifo
      };

// this are also the jack audio transport states:
enum class Transport : char {
      STOP=0,
      PLAY=1,
      STARTING=3,
      NET_STARTING=4
      };

//---------------------------------------------------------
//   Seq
//    sequencer
//---------------------------------------------------------

class Seq : public QObject, public Sequencer {
      Q_OBJECT

      mutable QMutex mutex;

      MasterScore* cs;
      ScoreView* cv;
      bool running;                       // true if sequencer is available
      Transport state;                    // STOP, PLAY, STARTING=3
      bool inCountIn;
                                          // When we begin playing count in, JACK should play the ticks, but shouldn't run
                                          // JACK Transport to prevent playing in other applications. Before playing
                                          // count in we have to disconnect from JACK Transport by switching to the fake transport.
                                          // Also we save current preferences.useJackTransport value to useJackTransportSavedFlag
                                          // to restore it when count in ends. After this all applications start playing in sync.
      bool useJackTransportSavedFlag;
      int maxMidiOutPort;                 // Maximum count of midi out ports in all opened scores
      Fraction prevTimeSig;
      double prevTempo;

      bool oggInit;
      bool playlistChanged;

      SeqMsgFifo toSeq;
      SeqMsgFifo fromSeq;
      Driver* _driver;
      MasterSynthesizer* _synti;

      double meterValue[2];
      double meterPeakValue[2];
      int peakTimer[2];

      EventMap events;                    // playlist for playback mode
      EventMap::const_iterator eventsEnd;
      EventMap renderEvents;              // event list that is rendered in background
      RangeMap renderEventsStatus;
      MidiRenderer midi;
      QFuture<void> midiRenderFuture;
      bool allowBackgroundRendering = false; // should be set to true only when playing, so no
                                             // score changes are possible.
      EventMap countInEvents;             // playlist of any metronome countin clicks
      QQueue<NPlayEvent> _liveEventQueue; // playlist for score editing and note entry (rendered live)

      int playFrame;                      // current play position in samples, relative to the first frame of playback
      int countInPlayFrame;               // current play position in samples, relative to the first frame of countin
      int endUTick;                       // the final tick of midi events collected by collectEvents()

      EventMap::const_iterator playPos;   // moved in real time thread
      EventMap::const_iterator countInPlayPos;
      EventMap::const_iterator guiPos;    // moved in gui thread

      QList<const Note*> markedNotes;     // notes marked as sounding

      uint tackRemain;        // metronome state (remaining audio samples)
      uint tickRemain;
      qreal tackVolume;       // relative volumes
      qreal tickVolume;
      qreal metronomeVolume;  // overall volume

      unsigned initialMillisecondTimestampWithLatency; // millisecond timestamp (relative to PortAudio's initialization) of start of playback

      QTimer* heartBeatTimer;
      QTimer* noteTimer;

      void renderChunk(const MidiRenderer::Chunk&, EventMap*);
      void updateEventsEnd();

      void setPos(int);
      void playEvent(const NPlayEvent&, unsigned framePos);
      void guiToSeq(const SeqMsg& msg);
      void metronome(unsigned n, float* l, bool force);
      void seekCommon(int utick);
      void unmarkNotes();
      void updateSynthesizerState(int tick1, int tick2);
      void addCountInClicks();

      int getPlayStartUtick();

      inline QQueue<NPlayEvent>* liveEventQueue() { return &_liveEventQueue; }

   private slots:
      void seqMessage(int msg, int arg = 0);
      void heartBeatTimeout();
      void midiInputReady();
      void setPlaylistChanged() { playlistChanged = true; }
      void handleTimeSigTempoChanged();

   public slots:
      void setRelTempo(double);
      void seek(int utick);
      void seekRT(int utick);
      void stopNotes(int channel = -1, bool realTime = false);
      void start();
      void stop();
      void setPos(POS, unsigned);
      void setMetronomeGain(float val) { metronomeVolume = val; }

   signals:
      void started();
      void stopped();
      int toGui(int, int arg = 0);
      void heartBeat(int, int, int);
      void tempoChanged();
      void timeSigChanged();

   public:
      Seq();
      ~Seq();
      bool canStart();
      void rewindStart();
      void loopStart();
      void seekEnd();
      void nextMeasure();
      void nextChord();
      void prevMeasure();
      void prevChord();

      void collectEvents(int utick);
      void ensureBufferAsync(int utick);
      void guiStop();
      void stopWait();
      void setLoopIn();
      void setLoopOut();
      void setLoopSelection();

      bool init(bool hotPlug = false);
      void exit();
      bool isRunning() const    { return running; }
      bool isPlaying() const    { return state == Transport::PLAY; }
      bool isStopped() const    { return state == Transport::STOP; }

      void processMessages();
      void process(unsigned framesPerPeriod, float* buffer);
      int getEndUTick() const   { return endUTick;  }
      bool isRealtime() const   { return true;     }
      void sendMessage(SeqMsg&) const;

      void setController(int, int, int);
      virtual void sendEvent(const NPlayEvent&);
      void setScoreView(ScoreView*);
      MasterScore* score() const   { return cs; }
      ScoreView* viewer() const { return cv; }
      void initInstruments(bool realTime = false);

      Driver* driver()                                 { return _driver; }
      void setDriver(Driver* d)                        { _driver = d;    }
      MasterSynthesizer* synti() const                 { return _synti;  }
      void setMasterSynthesizer(MasterSynthesizer* ms) { _synti = ms;    }

      int getCurTick();
      double curTempo() const;

      void putEvent(const NPlayEvent&, unsigned framePos = 0);
      void startNoteTimer(int duration);
      virtual void startNote(int channel, int, int, double nt) override;
      virtual void startNote(int channel, int, int, int, double nt) override;
      virtual void playMetronomeBeat(BeatType type) override;

      void eventToGui(NPlayEvent);
      void stopNoteTimer();
      void recomputeMaxMidiOutPort();
      float metronomeGain() const      { return metronomeVolume; }

      void setInitialMillisecondTimestampWithLatency();
      unsigned getCurrentMillisecondTimestampWithLatency(unsigned framePos) const;
      };

extern Seq* seq;
extern void initSequencer();
extern bool initMidi();

} // namespace Ms
#endif

