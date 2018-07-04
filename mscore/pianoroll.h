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
class Note;
class PianoRuler;
class Seq;
class WaveView;

//---------------------------------------------------------
//   PianorollEditor
//---------------------------------------------------------

class PianorollEditor : public QMainWindow, public MuseScoreView {
      Q_OBJECT

      PianoView* gv;
      PianoKeyboard* pianoKbd;
      QScrollBar* hsb;        // horizontal scroll bar for pianoView
      Score* _score;
      Staff* staff;
      Awl::PitchEdit* pitch;
      QSpinBox* velocity;
      QSpinBox* onTime;
      QSpinBox* tickLen;
      Pos locator[3];
      QComboBox* veloType;
      Awl::PosLabel* pos;
      PianoRuler* ruler;
      QAction* showWave;
      WaveView* waveView;
      QSplitter* split;
      QList<QAction*> actions;

      void updateVelocity(Note* note);
      void updateSelection();
      void readSettings();

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
      void focusOnElement(Element* focus);
      void heartBeat(Seq*);

      virtual void dataChanged(const QRectF&);
      virtual void updateAll();
      virtual void adjustCanvasPosition(const Element*, bool);
      virtual void removeScore();
      virtual void changeEditElement(Element*);
      virtual QCursor cursor() const;
      virtual void setCursor(const QCursor&);
      virtual int gripCount() const;
      virtual const QTransform& matrix() const;
      virtual void setDropRectangle(const QRectF&);
      virtual void cmdAddSlur(Note*, Note*);
      virtual void cmdAddHairpin(bool) {}
      virtual void startEdit();
      virtual void startEdit(Element*, Grip);
      virtual Element* elementNear(QPointF);
      virtual void drawBackground(QPainter* /*p*/, const QRectF& /*r*/) const {}

      void setLocator(POS pos, int tick) { locator[int(pos)].setTick(tick); }

      void writeSettings();
      virtual const QRect geometry() const override { return QMainWindow::geometry(); }
      };


} // namespace Ms
#endif


