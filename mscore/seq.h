//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: seq.h 5660 2012-05-22 14:17:39Z wschweer $
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

#include "libmscore/sequencer.h"
#include "synthesizer/event.h"
#include "driver.h"
#include "libmscore/fifo.h"
#include "libmscore/tempo.h"

class QTimer;

namespace Ms {

class Note;
class Score;
class Painter;
class Measure;
class Driver;
class Part;
struct Channel;
class ScoreView;
class MasterSynthesizer;
class Segment;

//---------------------------------------------------------
//   SeqMsg
//    message format for gui -> sequencer messages
//---------------------------------------------------------

enum { SEQ_NO_MESSAGE, SEQ_TEMPO_CHANGE, SEQ_PLAY, SEQ_SEEK,
       SEQ_MIDI_INPUT_EVENT
      };

struct SeqMsg {
      int id;
      union {
            int intVal;
            qreal realVal;
            };
      NPlayEvent event;

      SeqMsg() {}
      SeqMsg(int _id, int val) : id(_id), intVal(val) {}
      SeqMsg(int _id, qreal val) : id(_id), realVal(val) {}
      SeqMsg(int _id, const NPlayEvent& e) : id(_id), event(e) {}
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

//---------------------------------------------------------
//   Seq
//    sequencer
//---------------------------------------------------------

class Seq : public QObject, public Sequencer {
      Q_OBJECT

      mutable QMutex mutex;

      Score* cs;
      ScoreView* cv;
      bool running;                       // true if sequencer is available
      int state;                          // TRANSPORT_STOP, TRANSPORT_PLAY, TRANSPORT_STARTING=3

      bool oggInit;
      bool playlistChanged;

      SeqMsgFifo toSeq;
      SeqMsgFifo fromSeq;
      Driver* _driver;
      MasterSynthesizer* _synti;

      double meterValue[2];
      double meterPeakValue[2];
      int peakTimer[2];

      EventMap events;                    // playlist

      int playTime;                       // current play position in samples
      int endTick;
      
      EventMap::const_iterator playPos;   // moved in real time thread
      EventMap::const_iterator guiPos;    // moved in gui thread
      QList<const Note*> markedNotes;     // notes marked as sounding

      uint tackRest;                      // metronome state
      uint tickRest;
      qreal metronomeVolume;

      QTimer* heartBeatTimer;
      QTimer* noteTimer;

      void collectMeasureEvents(Measure*, int staffIdx);

      void setPos(int);
      void playEvent(const NPlayEvent&);
      void guiToSeq(const SeqMsg& msg);
      void metronome(unsigned n, float* l);
      void seek(int utick, Segment* seg);
      void unmarkNotes();
      void updateSynthesizerState(int tick1, int tick2);

   private slots:
      void seqMessage(int msg);
      void heartBeatTimeout();
      void selectionChanged(int);
      void midiInputReady();

   public slots:
      void setRelTempo(double);
      void seek(int);
      void stopNotes(int channel = -1);
      void start();
      void stop();

   signals:
      void started();
      void stopped();
      int toGui(int);
      void heartBeat(int, int, int);

   public:
      // this are also the jack audio transport states:
      enum { TRANSPORT_STOP=0, TRANSPORT_PLAY=1, TRANSPORT_STARTING=3,
           TRANSPORT_NET_STARTING=4 };

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

      void collectEvents();
      void guiStop();
      void stopWait();
      void setLoopIn();
      void setLoopOut();
      void setLoopSelection();

      bool init();
      void exit();
      bool isRunning() const    { return running; }
      bool isPlaying() const    { return state == TRANSPORT_PLAY; }
      bool isStopped() const    { return state == TRANSPORT_STOP; }

      void processMessages();
      void process(unsigned, float*);
      QList<QString> inputPorts();
      int getEndTick() const    { return endTick;  }
      bool isRealtime() const   { return true;     }
      void sendMessage(SeqMsg&) const;
      virtual void startNote(int channel, int, int, int, double nt);
      void setController(int, int, int);
      virtual void sendEvent(const NPlayEvent&);
      void setScoreView(ScoreView*);
      Score* score() const   { return cs; }
      ScoreView* viewer() const { return cv; }
      void initInstruments();

      Driver* driver()                                 { return _driver; }
      void setDriver(Driver* d)                        { _driver = d;    }
      MasterSynthesizer* synti() const                 { return _synti;  }
      void setMasterSynthesizer(MasterSynthesizer* ms) { _synti = ms;    }

      int getCurTick();
      double curTempo() const;

      void putEvent(const NPlayEvent&);
      void startNoteTimer(int duration);
      void startNote(int channel, int, int, double nt);
      void eventToGui(NPlayEvent);
      void stopNoteTimer();
      };

extern Seq* seq;
extern void initSequencer();
extern bool initMidi();

} // namespace Ms
#endif

