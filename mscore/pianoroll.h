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

#include "libmscore/mscoreview.h"
#include "libmscore/pos.h"

namespace Awl {
      class PitchEdit;
      class PosLabel;
      };

namespace Ms {

class Score;
class Staff;
class PianoView;
class Note;
class Ruler;
class Seq;
class WaveView;

//---------------------------------------------------------
//   PianorollEditor
//---------------------------------------------------------

class PianorollEditor : public QMainWindow, public MuseScoreView {
      Q_OBJECT

      PianoView* gv;
      QScrollBar* hsb;        // horizontal scroll bar for pianoView
      Score* _score;
      Staff* staff;
      Awl::PitchEdit* pitch;
      QSpinBox* velocity;
      Pos locator[3];
      QComboBox* veloType;
      Awl::PosLabel* pos;
      Ruler* ruler;
      QAction* showWave;
      WaveView* waveView;
      QSplitter* split;

      void updateVelocity(Note* note);
      void updateSelection();

   private slots:
      void selectionChanged();
      void veloTypeChanged(int);
      void velocityChanged(int);
      void keyPressed(int);
      void keyReleased(int);
      void moveLocator(int);
      void cmd(QAction*);
      void rangeChanged(int min, int max);
      void setXpos(int x);
      void showWaveView(bool);

   public slots:
      void changeSelection(int);

   public:
      PianorollEditor(QWidget* parent = 0);
      virtual ~PianorollEditor();

      void setStaff(Staff* staff);
      Score* score() const { return _score; }
      void heartBeat(Seq*);

      virtual void dataChanged(const QRectF&);
      virtual void updateAll();
      virtual void moveCursor();
      virtual void updateLoopCursors();
      virtual void showLoopCursors() {}
      virtual void hideLoopCursors() {}
      virtual void adjustCanvasPosition(const Element*, bool);
      virtual void setScore(Score*);
      virtual void removeScore();
      virtual void changeEditElement(Element*);
      virtual QCursor cursor() const;
      virtual void setCursor(const QCursor&);
      virtual int gripCount() const;
      virtual const QRectF& getGrip(int) const;
      virtual const QTransform& matrix() const;
      virtual void setDropRectangle(const QRectF&);
      virtual void cmdAddSlur(Note*, Note*);
      virtual void startEdit();
      virtual void startEdit(Element*, int);
      virtual Element* elementNear(QPointF);
      virtual void drawBackground(QPainter* /*p*/, const QRectF& /*r*/) const {}
      };


} // namespace Ms
#endif


