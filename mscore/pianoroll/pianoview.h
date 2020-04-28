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

#ifndef __PIANOVIEW_H__
#define __PIANOVIEW_H__

#include "libmscore/pos.h"
#include "pianorolledittool.h"

namespace Ms {

class Score;
class Staff;
class Chord;
class ChordRest;
class Note;
class NoteEvent;
class PianoView;
class NoteTweakerDialog;

enum class NoteSelectType {
      REPLACE = 0,
      XOR,
      ADD,
      SUBTRACT,
      FIRST
      };

enum class DragStyle {
    NONE = 0,
    SELECTION_RECT,
    NOTES
      };

struct BarPattern {
      QString name;
      char isWhiteKey[12];  //Set to 1 for white keys, 0 for black
      };

//---------------------------------------------------------
//   PianoItem
//---------------------------------------------------------

class PianoItem {
      Note* _note;
      PianoView* _pianoView;
      
      void paintNoteBlock(QPainter* painter, NoteEvent* evt);
      QRect boundingRectTicks(NoteEvent* evt);
      QRect boundingRectPixels(NoteEvent* evt);
      bool intersectsBlock(int startTick, int endTick, int highPitch, int lowPitch, NoteEvent* evt);
      
   public:
      PianoItem(Note*, PianoView*);
      ~PianoItem() {}
      Note* note() { return _note; }
      void paint(QPainter* painter);
      bool intersects(int startTick, int endTick, int highPitch, int lowPitch);
      
      QRect boundingRect();
      
      NoteEvent* getTweakNoteEvent();
      };

//---------------------------------------------------------
//   PianoView
//---------------------------------------------------------

class PianoView : public QGraphicsView {
      Q_OBJECT

public:
      static const BarPattern barPatterns[];

private:
      Staff* _staff;
      Chord* _chord;
      
      Pos trackingPos;  //Track mouse position
      Pos* _locator;
      int _ticks;
      TType _timeType;
      int _noteHeight;
      qreal _xZoom;
      int _tuplet;  //Tuplet divisions
      int _subdiv;  //Beat subdivisions
      int _barPattern;

      bool _playEventsView;
      bool _mouseDown;
      bool _dragStarted;
      QPointF _mouseDownPos;
      QPointF _lastMousePos;
      DragStyle _dragStyle;
      int _lastDragPitch;
      bool _inProgressUndoEvent;

      //The length of the note we are using for editng purposes, expressed as a fraction of the measure.
      // Note length will be (2^_editNoteLength) of a measure
      int _editNoteLength = 0;
      int _editNoteDots = 0;
      int _editNoteVoice = 0;
      PianoRollEditTool _editNoteTool = PianoRollEditTool::SELECT;

      QList<PianoItem*> _noteList;
      quint8 _pitchHighlight[128];

      virtual void drawBackground(QPainter* painter, const QRectF& rect);

      void addChord(Chord* _chord, int voice);
      void updateBoundingSize();
      void clearNoteData();
      void selectNotes(int startTick, int endTick, int lowPitch, int highPitch, NoteSelectType selType);
      void showPopupMenu(const QPoint& pos);
      bool cutChordRest(ChordRest* e, int track, Fraction cutTick, ChordRest*& cr0, ChordRest*& cr1);
      void handleSelectionClick();
      void insertNote(int modifiers);
      Fraction roundToStartBeat(int tick) const;
      Fraction noteEditLength() const;
      void changeChordLength(const QPointF& pos);
      void eraseNote(const QPointF& pos);
      void appendNoteToChord(const QPointF& pos);
      void cutChord(const QPointF& pos);
      void toggleTie(const QPointF& pos);
      void toggleTie(Note*);
      void dragSelectionNoteGroup();

      QAction* getAction(const char* id);

   protected:
      virtual void wheelEvent(QWheelEvent* event);
      virtual void mousePressEvent(QMouseEvent* event);
      virtual void mouseReleaseEvent(QMouseEvent* event);
      virtual void mouseMoveEvent(QMouseEvent* event);
      virtual void leaveEvent(QEvent*);
      virtual void contextMenuEvent(QContextMenuEvent *event);

   signals:
      void xZoomChanged(qreal);
      void tupletChanged(int);
      void subdivChanged(int);
      void barPatternChanged(int);
      void noteHeightChanged(int);
      void pitchChanged(int);
      void trackingPosChanged(const Pos&);
      void selectionChanged();
      void showNoteTweakerRequest();

   public slots:
      void moveLocator(int);
      void updateNotes();
      void setXZoom(int);
      void setTuplet(int);
      void setSubdiv(int);
      void setBarPattern(int);
      void togglePitchHighlight(int pitch);
      void showNoteTweaker();

   public:
      PianoView();
      ~PianoView();
      Staff* staff() { return _staff; }
      void setStaff(Staff*, Pos* locator);
      void ensureVisible(int tick);
      int noteHeight() { return _noteHeight; }
      qreal xZoom() { return _xZoom; }
      int tuplet() { return _tuplet; }
      int subdiv() { return _subdiv; }
      int barPattern() { return _barPattern; }
      PianoRollEditTool editTool() const { return _editNoteTool; }
      QList<QGraphicsItem*> items() { return scene()->selectedItems(); }
      int editNoteDots() const { return _editNoteDots; }

      void setEditNoteLength(int len) { _editNoteLength = len; }
      void setEditNoteVoice(int voice) { _editNoteVoice = voice; }
      void setEditNoteDots(int dot) { _editNoteDots = dot; }
      void setEditNoteTool(PianoRollEditTool tool) { _editNoteTool = tool; }

      int pixelXToTick(int pixX);
      int tickToPixelX(int tick);
      int pixelYToPitch(int pixY) { return (int)floor(128 - pixY / (qreal)_noteHeight); }
      
      PianoItem* pickNote(int tick, int pitch);

      QList<PianoItem*> getSelectedItems();
      QList<PianoItem*> getItems();
      
      void zoomView(int step, bool horizontal, int centerX, int centerY);

      bool playEventsView() { return _playEventsView; }
      };


} // namespace Ms
#endif

