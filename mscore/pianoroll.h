//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2009-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __PIANOROLL_H__
#define __PIANOROLL_H__

namespace Awl {
      class PitchEdit;
      class PosLabel;
      };

#include "libmscore/mscoreview.h"
#include "libmscore/pos.h"
#include "libmscore/score.h"
#include "libmscore/select.h"

namespace Ms {

class Score;
class Staff;
class PianoView;
class PianoKeyboard;
class PianoLevels;
class PianoLevelsChooser;
class Note;
class PianoRuler;
class Seq;
class WaveView;

//---------------------------------------------------------
//   PianorollEditor
//---------------------------------------------------------

class PianorollEditor : public QMainWindow, public MuseScoreView {
      Q_OBJECT

      PianoView* pianoView;
      PianoKeyboard* pianoKbd;
      PianoLevels* pianoLevels;
      PianoLevelsChooser* pianoLevelsChooser;
      QScrollBar* hsb;        // horizontal scroll bar for pianoView
      Score* _score;
      Staff* staff;
      QLabel* partLabel;
      Awl::PitchEdit* pitch;
      QSpinBox* velocity;
      QSpinBox* onTime;
      QSpinBox* tickLen;
      Pos locator[3];
      QComboBox* barPattern;
      QComboBox* veloType;
      QSpinBox* subdiv;
      QSpinBox* tuplet;
      Awl::PosLabel* pos;
      PianoRuler* ruler;
      QAction* showWave;
      WaveView* waveView;
      QSplitter* split;
      QList<QAction*> actions;

      bool updateScheduled = false;

      void updateVelocity(Note* note);
      void updateSelection();
      void readSettings();
      void doUpdate();

   private slots:
      void selectionChanged();
      void veloTypeChanged(int);
      void velocityChanged(int);
      void keyPressed(int);
      void keyReleased(int);
      void moveLocator(int, const Pos&);
      void cmd(QAction*);
      void rangeChanged(int min, int max);
      void setXpos(int x);
      void showWaveView(bool);
      void posChanged(POS pos, unsigned tick);
      void tickLenChanged(int);
      void onTimeChanged(int val);
      void playlistChanged();

   public slots:
      void changeSelection(SelState);

   public:
      PianorollEditor(QWidget* parent = 0);
      virtual ~PianorollEditor();

      void setStaff(Staff* staff);
      void focusOnPosition(Position* p);
      void heartBeat(Seq*);

      virtual void dataChanged(const QRectF&) override;
      virtual void updateAll() override;
      virtual void removeScore() override;
      virtual void changeEditElement(Element*) override;
      virtual QCursor cursor() const override;
      virtual void setCursor(const QCursor&) override;
      virtual int gripCount() const override;
      const QTransform& matrix() const;
      virtual void startEdit() override;
      virtual void startEdit(Element*, Grip) override;
      virtual Element* elementNear(QPointF) override;
      virtual void drawBackground(QPainter* /*p*/, const QRectF& /*r*/) const override {}

      void setLocator(POS posi, int tick) { locator[int(posi)].setTick(tick); }

      void writeSettings();
      virtual const QRect geometry() const override { return QMainWindow::geometry(); }
      };


} // namespace Ms
#endif


