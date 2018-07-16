//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2009 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __SCANVAS_H__
#define __SCANVAS_H__

#include "globals.h"
#include "libmscore/element.h"
#include "libmscore/durationtype.h"
#include "libmscore/mscore.h"
#include "libmscore/mscoreview.h"
#include "libmscore/pos.h"

namespace Ms {

class ChordRest;
class Rest;
class Element;
class Page;
class XmlWriter;
class Note;
class Lasso;
class FotoLasso;
class ShadowNote;
class Segment;
class Measure;
class System;
class Score;
class ScoreView;
class Text;
class MeasureBase;
class Staff;
class OmrView;
class PositionCursor;
class ContinuousPanel;
class Tuplet;
class FretDiagram;
class Bend;
class TremoloBar;

#ifdef Q_OS_MAC
#define CONTROL_MODIFIER Qt::AltModifier
#else
#define CONTROL_MODIFIER Qt::ControlModifier
#endif

enum class Grip : int;
enum class POS : char;
enum class MagIdx : char;

//---------------------------------------------------------
//   ViewState
//---------------------------------------------------------

enum class ViewState {
      NORMAL,
      DRAG,
      DRAG_OBJECT,
      EDIT,
      DRAG_EDIT,
      LASSO,
      NOTE_ENTRY,
      PLAY,
      ENTRY_PLAY,

      FOTO,
      FOTO_DRAG,
      FOTO_DRAG_EDIT,
      FOTO_DRAG_OBJECT,
      FOTO_LASSO,
      };

enum class TEXT : char {
      TITLE,
      SUBTITLE,
      COMPOSER,
      POET,
      PART,
      SYSTEM,
      STAFF,
      EXPRESSION,
      REHEARSAL_MARK,
      INSTRUMENT_CHANGE,
      FINGERING
      };

//---------------------------------------------------------
//   ScoreView
//---------------------------------------------------------

class ScoreView : public QWidget, public MuseScoreView {
      Q_OBJECT

      ViewState state;
      OmrView* _omrView;

      // the next elements are used during dragMove to give some visual
      // feedback:
      //    dropTarget:       if valid, the element is drawn in a different color
      //                      to mark it as a valid drop target
      //    dropRectangle:    if valid, the rectangle is filled with a
      //                      color to visualize a valid drop area
      //    dropAnchor:       if valid the line is drawn from the current
      //                      cursor position to the current anchor point
      // Note:
      //    only one of the elements is active during drag

      const Element* dropTarget;    ///< current drop target during dragMove
      QRectF dropRectangle;         ///< current drop rectangle during dragMove
      QLineF dropAnchor;            ///< line to current anchor point during dragMove

      QTransform _matrix, imatrix;
      MagIdx _magIdx;

      QFocusFrame* focusFrame;

      EditData editData;

      //--input state:
      PositionCursor* _cursor;
      ShadowNote* shadowNote;

      // Realtime state:      Note: always set allowRealtimeRests to desired value before starting a timer.
      QTimer* realtimeTimer;   // multi-shot timer for advancing in automatic realtime mode
      QTimer* extendNoteTimer; // single-shot timer for initial advancement when a note is held
      bool allowRealtimeRests; // Allow entering rests in realtime mode? (See note above)

      // Loop In/Out marks in the score
      PositionCursor* _curLoopIn;
      PositionCursor* _curLoopOut;

      // Continuous panel
      ContinuousPanel* _continuousPanel;

      Lasso* lasso;           ///< temporarily drawn lasso selection
      FotoLasso* _foto;

      QColor _bgColor;
      QColor _fgColor;
      QPixmap* _bgPixmap;
      QPixmap* _fgPixmap;

      virtual void paintEvent(QPaintEvent*);
      void paint(const QRect&, QPainter&);

      void objectPopup(const QPoint&, Element*);
      void measurePopup(const QPoint&, Measure*);

      void saveChord(XmlWriter&);

      virtual bool event(QEvent* event) override;
      virtual bool gestureEvent(QGestureEvent*);            // ??
      virtual void resizeEvent(QResizeEvent*) override;
      virtual void dragEnterEvent(QDragEnterEvent*) override;
      virtual void dragLeaveEvent(QDragLeaveEvent*) override;
      virtual void dragMoveEvent(QDragMoveEvent*) override;
      virtual void dropEvent(QDropEvent*) override;
      virtual void focusInEvent(QFocusEvent*) override;
      virtual void focusOutEvent(QFocusEvent*) override;

      virtual void wheelEvent(QWheelEvent*) override;
      virtual void mouseMoveEvent(QMouseEvent*) override;
      virtual void mousePressEvent(QMouseEvent*) override;
      virtual void mouseReleaseEvent(QMouseEvent*) override;
      virtual void mouseDoubleClickEvent(QMouseEvent*);

      virtual void keyPressEvent(QKeyEvent*) override;
      virtual void keyReleaseEvent(QKeyEvent*) override;
      virtual void inputMethodEvent(QInputMethodEvent*) override;

      virtual void contextMenuEvent(QContextMenuEvent*) override;

      void mousePressEventNormal(QMouseEvent*);
      void escapeCmd();

      void constraintCanvas(int *dxx, int *dyy);
      void contextItem(Element*);
      void lassoSelect();

      void setShadowNote(const QPointF&);
      void drawElements(QPainter& p,QList<Element*>& el, Element* editElement);
      bool dragTimeAnchorElement(const QPointF& pos);
      bool dragMeasureAnchorElement(const QPointF& pos);
      void updateGrips();
      virtual void lyricsTab(bool back, bool end, bool moveOnly) override;
      virtual void lyricsReturn() override;
      virtual void lyricsEndEdit() override;
      virtual void lyricsUpDown(bool up, bool end) override;
      virtual void lyricsMinus() override;
      virtual void lyricsUnderscore() override;
      void harmonyEndEdit();
      void harmonyTab(bool back);
      void harmonyBeatsTab(bool noterest, bool back);
      void harmonyTicksTab(int ticks);
      void figuredBassTab(bool meas, bool back);
      void figuredBassTicksTab(int ticks);
      void figuredBassEndEdit();
      void realtimeAdvance(bool allowRests);
      void cmdAddFret(int fret);
      void cmdAddChordName();
      void cmdAddText(TEXT style);
      void cmdEnterRest(const TDuration&);
      void cmdEnterRest();
      void cmdTuplet(int n, ChordRest*);
      void cmdTuplet(int);
      void cmdCreateTuplet(ChordRest* cr, Tuplet* tuplet);
      void cmdRepeatSelection();
      void cmdChangeEnharmonic(bool);

      MeasureBase* insertMeasure(ElementType, MeasureBase*);
      MeasureBase* checkSelectionStateForInsertMeasure();

      void appendMeasures(int, ElementType);
      MeasureBase* appendMeasure(ElementType);
      void cmdInsertMeasure(ElementType);
      void createElementPropertyMenu(Element* e, QMenu*);
      void genPropertyMenu1(Element* e, QMenu* popup);
      void genPropertyMenuText(Element* e, QMenu* popup);
      void elementPropertyAction(const QString&, Element* e);
      void paintPageBorder(QPainter& p, Page* page);
      bool dropCanvas(Element*);
      void editCmd(const QString&);
      void setLoopCursor(PositionCursor* curLoop, int tick, bool isInPos);
      void cmdMoveCR(bool left);
      void cmdGotoElement(Element*);
      bool checkCopyOrCut();
      QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;
      void startFotomode();
      void stopFotomode();
      void startFotoDrag();
      void endFotoDrag();
      void endFotoDragEdit();

      virtual void startEdit();
      void endEdit();

      void startDrag();
      void endDrag();

      void endDragEdit();

      void startNoteEntry();
      virtual void startNoteEntryMode() override;
      void endNoteEntry();

      void endLasso();

   private slots:
      void posChanged(POS pos, unsigned tick);
      void loopToggled(bool);
      void triggerCmdRealtimeAdvance();
      void cmdRealtimeAdvance();
      void extendCurrentNote();
      void seqStopped();

   public slots:
      void setViewRect(const QRectF&);

      void deselectAll();

      void editCopy();
      void editCut();
      void editPaste();
      void editSwap();

      void normalCut();
      void normalCopy();
      void fotoModeCopy();
      bool normalPaste();
      void normalSwap();

      void cloneElement(Element* e);
      void doFotoDragEdit(QMouseEvent* ev);

      void updateContinuousPanel();
      void ticksTab(int ticks);     // helper function

   signals:
      void viewRectChanged();
      void scaleChanged(double);
      void offsetChanged(double, double);
      void sizeChanged();

   public:
      ScoreView(QWidget* parent = 0);
      ~ScoreView();

      QPixmap* fgPixmap() { return _fgPixmap; }

      virtual void startEdit(Element*, Grip) override;
      virtual void startEditMode(Element*) override;

      void moveCursor(int tick);
      int cursorTick() const;
      void setCursorOn(bool);
      void setBackground(QPixmap*);
      void setBackground(const QColor&);
      void setForeground(QPixmap*);
      void setForeground(const QColor&);

      Page* addPage();
      virtual void setScore(Score* s);
      virtual void removeScore()  { _score = 0; }

      void setMag(qreal m);
      bool navigatorVisible() const;
      void cmd(const QAction*);
      void cmd(const char*);

      void startUndoRedo(bool);
      void zoomStep(qreal step, const QPoint& pos);
      void zoom(qreal _mag, const QPointF& pos);
      void contextPopup(QContextMenuEvent* ev);
      bool editKeyLyrics();
      void dragScoreView(QMouseEvent* ev);
      void doDragElement(QMouseEvent* ev);
      void doDragLasso(QMouseEvent* ev);
      void doDragFoto(QMouseEvent* ev);
      void doDragEdit(QMouseEvent* ev);
      bool testElementDragTransition(QMouseEvent* ev);
      bool fotoEditElementDragTransition(QMouseEvent* ev);
      void addSlur();
      virtual void cmdAddSlur(ChordRest*, ChordRest*) override;
      virtual void cmdAddHairpin(HairpinType) override;
      void cmdAddNoteLine();

      bool noteEntryMode() const { return state == ViewState::NOTE_ENTRY; }
      bool editMode() const      { return state == ViewState::EDIT; }
      bool fotoMode() const;

      virtual void setDropRectangle(const QRectF&);
      virtual void setDropTarget(const Element*) override;
      void setDropAnchor(const QLineF&);
      const QTransform& matrix() const  { return _matrix; }
      qreal mag() const;
      qreal lmag() const;
      MagIdx magIdx() const             { return _magIdx; }
      void setMag(MagIdx idx, double mag);
      qreal xoffset() const;
      qreal yoffset() const;
      void setOffset(qreal x, qreal y);
      QSizeF fsize() const;
      void pageNext();
      void pagePrev();
      void pageTop();
      void pageEnd();
      QPointF toLogical(const QPoint& p) const   { return imatrix.map(QPointF(p)); }
      QPointF toPhysical(const QPointF& p) const {return _matrix.map(p); }
      QRectF toLogical(const QRectF& r) const    { return imatrix.mapRect(r); }
      QRect toPhysical(const QRectF& r) const    { return _matrix.mapRect(r).toRect(); }

      bool searchMeasure(int i);
      bool searchPage(int i);
      bool searchRehearsalMark(const QString& s);
      void gotoMeasure(Measure*);
      void setFocusRect();
      void changeVoice(int voice);
      virtual void drawBackground(QPainter* p, const QRectF& r) const;
      bool fotoScoreViewDragTest(QMouseEvent*);
      bool fotoScoreViewDragRectTest(QMouseEvent*);
      void doDragFotoRect(QMouseEvent*);
      void fotoContextPopup(QContextMenuEvent*);
      bool fotoRectHit(const QPoint& p);
      void paintRect(bool printMode, QPainter& p, const QRectF& r, double mag);
      bool saveFotoAs(bool printMode, const QRectF&);
      void fotoDragDrop(QMouseEvent*);
      void changeEditElement(Element*);

      void cmdAppendMeasures(int, ElementType);
      void cmdInsertMeasures(int, ElementType);

      void cmdAddRemoveBreaks();
      void cmdCopyLyricsToClipboard();

      ScoreState mscoreState() const;
      void setCursorVisible(bool v);
      void showOmr(bool flag);
      void midiNoteReceived(int pitch, bool chord, int velocity);

      virtual void moveCursor() override;

      virtual void layoutChanged();
      virtual void dataChanged(const QRectF&);
      virtual void updateAll()    { update(); }
      virtual void adjustCanvasPosition(const Element* el, bool playBack, int staff = -1) override;
      virtual void setCursor(const QCursor& c) { QWidget::setCursor(c); }
      virtual QCursor cursor() const { return QWidget::cursor(); }
      void loopUpdate(bool val)   {  loopToggled(val); }

      void updateShadowNotes();

      OmrView* omrView() const        { return _omrView; }
      void setOmrView(OmrView* v)     { _omrView = v;    }
      FotoLasso* fotoLasso() const    { return _foto;    }
      Element* getEditElement();

      virtual Element* elementNear(QPointF);
//      void editFretDiagram(FretDiagram*);
      void editBendProperties(Bend*);
      void editTremoloBarProperties(TremoloBar*);
      EditData& getEditData()        { return editData; }
      void changeState(ViewState);

      virtual const QRect geometry() const override { return QWidget::geometry(); }
      };

} // namespace Ms
#endif

