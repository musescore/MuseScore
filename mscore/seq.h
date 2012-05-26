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
#include "libmscore/event.h"
#include "driver.h"
#include "libmscore/fifo.h"
#include "libmscore/tempo.h"

class Note;
class QTimer;
class Score;
class Painter;
class Measure;
class Driver;
class Part;
struct Channel;
class ScoreView;
class MasterSynth;

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
            } data;
      Event event;
      };

//---------------------------------------------------------
//   SeqMsgFifo
//---------------------------------------------------------

static const int SEQ_MSG_FIFO_SIZE = 512;

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

      Score* cs;
      ScoreView* cv;
      bool running;                       // true if sequencer is available
      int state;                          // TRANSPORT_STOP, TRANSPORT_PLAY, TRANSPORT_STARTING=3

      bool oggInit;

      bool playlistChanged;

      SeqMsgFifo toSeq;
      SeqMsgFifo fromSeq;
      Driver* driver;

      double meterValue[2];
      double meterPeakValue[2];
      int peakTimer[2];

      EventMap events;                    // playlist

      int playTime;                       // current play position in samples
      int endTick;

      EventMap::const_iterator playPos;   // moved in real time thread
      EventMap::const_iterator guiPos;    // moved in gui thread
      QList<const Note*> markedNotes;     // notes marked as sounding

      uint tackRest;     // metronome state
      uint tickRest;
      qreal metronomeVolume;

      QTimer* heartBeatTimer;
      QTimer* noteTimer;

      void collectMeasureEvents(Measure*, int staffIdx);

      void stopTransport();
      void startTransport();
      void setPos(int);
      void playEvent(const Event&);
      void guiToSeq(const SeqMsg& msg);
      void metronome(unsigned n, float* l);

   private slots:
      void seqMessage(int msg);
      void heartBeat();
      void selectionChanged(int);
      void midiInputReady();

   public slots:
      void setRelTempo(double);
      void setGain(float);
      void seek(int);
      void stopNotes(int channel = -1);
      void start();
      void stop();

   signals:
      void started();
      void stopped();
      int toGui(int);
      void gainChanged(float);

   public:
      // this are also the jack audio transport states:
      enum { TRANSPORT_STOP=0, TRANSPORT_PLAY=1, TRANSPORT_STARTING=3,
           TRANSPORT_NET_STARTING=4 };

      Seq();
      ~Seq();
      bool canStart();
      void rewindStart();
      void seekEnd();
      void nextMeasure();
      void nextChord();
      void prevMeasure();
      void prevChord();

      void collectEvents();
      void guiStop();
      void stopWait();

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
      virtual void startNote(const Channel&, int, int, int, double nt);
      void setController(int, int, int);
      virtual void sendEvent(const Event&);
      void setScoreView(ScoreView*);
      Score* score() const   { return cs; }
      ScoreView* viewer() const { return cv; }
      void initInstruments();

      QList<MidiPatch*> getPatchInfo() const;
      Driver* getDriver()  { return driver; }
      int getCurTick();

      float gain() const;

      int synthNameToIndex(const QString&) const;
      QString synthIndexToName(int) const;
      void putEvent(const Event&);
      void startNoteTimer(int duration);
      void startNote(const Channel&, int, int, double nt);
      void eventToGui(Event);
      void processToGuiMessages();
      void stopNoteTimer();
      };

extern Seq* seq;
extern MasterSynth* synti;

extern void initSequencer();
extern bool initMidi();
#endif

