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

#include "fotomode.h"
#include "globals.h"
#include "zoombox.h"

#include "libmscore/durationtype.h"
#include "libmscore/element.h"
#include "libmscore/elementgroup.h"
#include "libmscore/mscore.h"
#include "libmscore/mscoreview.h"
#include "libmscore/pos.h"
#include "libmscore/property.h"

namespace Ms {

class ChordRest;
class Rest;
class Element;
class Page;
class XmlWriter;
class Note;
class Lasso;
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
class TimeSig;
class StaffTextBase;
class Articulation;

#ifdef Q_OS_MAC
#define CONTROL_MODIFIER Qt::AltModifier
#else
#define CONTROL_MODIFIER Qt::ControlModifier
#endif

enum class Grip : int;
enum class POS : char;
enum class ZoomIndex : char;

//---------------------------------------------------------
//   SmoothPanSettings
//---------------------------------------------------------

struct SmoothPanSettings {
      // these are all actually loaded from the loadFromPreferences fuction so don't change these to change the default values,
      // change the corresponding preferences
      bool enabled                        { false };

      double controlModifierBase          { 1 };      // initial speed modifier
      double controlModifierSteps         { 0.01 };   // modification steps for the modifier
      double minContinuousModifier        { 0.2 };    // minimum speed, 0.2 was chosen instead of 0 to remove stuttering
      double maxContinuousModifier        { 5 };      // maximum speed

      // Changing the distance will change the sensitivity/accuracy/jitter of the algorithm. Larger absolute values are generally smoother.
      double leftDistance                 { -250 };   // decelarate
      double leftDistance1                { -125 };
      double leftDistance2                { -50 };
      double leftDistance3                { -25 };
      double rightDistance                { 500 };    // accelerate
      double rightDistance1               { 250 };
      double rightDistance2               { 125 };
      double rightDistance3               { 50 };
                                                      // used to smoothly go back to normal speed when the playback cursor is getting closer
      double leftMod1                     { 0.8 };    // minimum speed at the first level
      double leftMod2                     { 0.9 };    // etc
      double leftMod3                     { 0.95 };
                                                      // used to smoothly go back to normal speed when the control cursor is getting closer to the playback cursor
      double rightMod1                    { 1.2 };    // maximum speed at the first level
      double rightMod2                    { 1.1 };    // etc
      double rightMod3                    { 1.05 };

      double controlCursorScreenPos       { 0.3 };  
      bool teleportLeftEnabled            { true };
      bool teleportRightEnabled           { false };

      bool advancedWeighting              { false };  // enables the 'smart weight'
      double normalWeight                 { 1 };
      double smartWeight                  { 0 };      // uses the distance between the 2 cursors to calculate the speed of the control cursor
      int cursorTimerDuration             { 1000 };   // how often the smart weight is updated

      void loadFromPreferences();
      };

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

//---------------------------------------------------------
//   ScoreViewState
//---------------------------------------------------------

struct ScoreViewState {
      qreal logicalZoomLevel = 1.0;
      ZoomIndex zoomIndex = ZoomIndex::ZOOM_FREE;
      qreal xOffset = 0.0;
      qreal yOffset = 0.0;

      ScoreViewState() {}
      ScoreViewState(qreal zl, ZoomIndex zi, qreal x, qreal y)
            {
            logicalZoomLevel = zl;
            zoomIndex = zi;
            xOffset = x;
            yOffset = y;
            }
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
      QVector<QLineF> m_dropAnchorLines;

      QTransform _matrix, imatrix;
      ZoomIndex _zoomIndex;
      ZoomState _previousLogicalZoom { ZoomIndex::ZOOM_PAGE_WIDTH, 0.0 }; // for zoom-level toggling

      QFocusFrame* focusFrame;

      EditData editData;
      std::vector<std::unique_ptr<ElementGroup>> dragGroups;

      //--input state:
      PositionCursor* _cursor;
      QColor _cursorColor;
      const int MAX_CURSOR_ALPHA = 220;

      PositionCursor* _controlCursor;
      SmoothPanSettings _panSettings;
      double _timeElapsed;
      double _controlModifier;      // a control modifier of 1 means that the cursor is moving at it's normal speed
                                    // if all measures are of the same size, this will stay equal to 1 (unless the distance settings are changed, and make the algorithm over-sensitive)
      double _playbackCursorOldPosition;
      double _playbackCursorNewPosition;
      double _playbackCursorDistanceTravelled;

      ShadowNote* shadowNote;

      // Realtime state:      Note: always set allowRealtimeRests to desired value before starting a timer.
      QTimer* realtimeTimer;   // multi-shot timer for advancing in automatic realtime mode
      QTimer* extendNoteTimer; // single-shot timer for initial advancement when a note is held
      bool allowRealtimeRests; // Allow entering rests in realtime mode? (See note above)

      bool tripleClickPending = false;
      bool popupActive = false;
      bool modifySelection = false;
      Element* elementToSelect = nullptr;

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

      // By default when the view will prevent viewpoint changes if
      // it is inactive. Set this flag to true to change this behaviour.
      bool _moveWhenInactive = false;

      bool _blockShowEdit = false;

      virtual void paintEvent(QPaintEvent*) override;
      void paint(const QRect&, QPainter&);

      void objectPopup(const QPoint&, Element*);
      void measurePopup(QContextMenuEvent* ev, Measure*);

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
      virtual void mouseDoubleClickEvent(QMouseEvent*) override;

      virtual void keyPressEvent(QKeyEvent*) override;
      virtual void keyReleaseEvent(QKeyEvent*) override;
      virtual void inputMethodEvent(QInputMethodEvent*) override;

      bool handleArrowKeyPress(const QKeyEvent*);

      virtual void contextMenuEvent(QContextMenuEvent*) override;

      void mousePressEventNormal(QMouseEvent*);
      void escapeCmd();
      bool startTextEditingOnMouseRelease(QMouseEvent*);
      void adjustCursorForTextEditing(QMouseEvent*);

      void constraintCanvas(int *dxx, int *dyy);

      void setShadowNote(const QPointF&);
      void drawElements(QPainter& p,QList<Element*>& el, Element* editElement);
      bool dragTimeAnchorElement(const QPointF& pos);
      bool dragMeasureAnchorElement(const QPointF& pos);
      virtual void lyricsTab(bool back, bool end, bool moveOnly) override;
      virtual void lyricsReturn() override;
      virtual void lyricsEndEdit() override;
      virtual void lyricsUpDown(bool up, bool end) override;
      virtual void lyricsMinus() override;
      virtual void lyricsUnderscore() override;
      virtual void textTab(bool back = false) override;
      void harmonyTab(bool back);
      void harmonyBeatsTab(bool noterest, bool back);
      void harmonyTicksTab(const Fraction& ticks);
      void figuredBassTab(bool meas, bool back);
      void figuredBassTicksTab(const Fraction& ticks);
      void realtimeAdvance(bool allowRests);
      void cmdAddFret(int fret);
      void cmdAddChordName(HarmonyType ht);
      void cmdAddText(Tid tid, Tid customTid = Tid::DEFAULT, PropertyFlags pf = PropertyFlags::STYLED, Placement p = Placement::ABOVE);
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
      void setLoopCursor(PositionCursor* curLoop, const Fraction& tick, bool isInPos);
      void cmdMoveCR(bool left);
      void cmdGotoElement(Element*);
      bool checkCopyOrCut();
      QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;
      void startFotomode();
      void stopFotomode();
      void startFotoDrag();
      void endFotoDrag();
      void endFotoDragEdit();
      QImage getRectImage(const QRectF& rect, double dpi, bool transparent, bool printMode);

      void startEdit(bool editMode = true);
      void endEdit();

      void startDrag();
      void endDrag();

      void endDragEdit();

      void startNoteEntry();
      virtual void startNoteEntryMode() override;
      void endNoteEntry();

      void endLasso();
      Element* getDropTarget(EditData&);

   private slots:
      void posChanged(POS pos, unsigned tick);
      void loopToggled(bool);
      void triggerCmdRealtimeAdvance();
      void cmdRealtimeAdvance();
      void extendCurrentNote();
      void seqStopped();
      void tripleClickTimeOut();

   public slots:
      void setViewRect(const QRectF&);

      void deselectAll();

      void editCopy();
      void editCut();
      void editPaste();
      void editSwap();

      void normalCut();
      void normalCopy();
      void fotoModeCopy(bool includeLink = false);
      bool normalPaste(Fraction scale = Fraction(1, 1));
      void normalSwap();

      void setControlCursorVisible(bool v);

      void cloneElement(Element* e);
      void doFotoDragEdit(QMouseEvent* ev);

      void updateContinuousPanel();
      void ticksTab(const Fraction& ticks);     // helper function

   signals:
      void viewRectChanged();
      void scaleChanged(double);
      void offsetChanged(double, double);
      void sizeChanged();

   public:
      ScoreView(QWidget* parent = 0);
      ~ScoreView();

      QPixmap* fgPixmap() { return _fgPixmap; }

      void startEdit(Element*, Grip) override;
      void startEditMode(Element*);

      void moveCursor(const Fraction& tick);
      void moveControlCursor(const Fraction& tick);
      bool isCursorDistanceReasonable();
      void moveControlCursorNearCursor();
      Fraction cursorTick() const;
      void setCursorOn(bool);
      void setBackground(QPixmap*);
      void setBackground(const QColor&);
      void setForeground(QPixmap*);
      void setForeground(const QColor&);

      Page* addPage();
      virtual void setScore(Score* s) override;
      virtual void removeScore() override { _score = 0; }

      void setPhysicalZoomLevel(qreal logicalLevel);

      bool navigatorVisible() const;
      void cmd(const QAction*);
      void cmd(const char*);

      void startUndoRedo(bool);
      void zoomBySteps(qreal numSteps, bool usingMouse = false, const QPointF& pos = QPointF());
      void setLogicalZoom(ZoomIndex index, qreal logicalLevel, const QPointF& pos = QPointF());
      qreal calculateLogicalZoomLevel(const ZoomIndex index, const qreal logicalFreeZoomLevel = 0.0) const;
      qreal calculatePhysicalZoomLevel(const ZoomIndex index, const qreal logicalFreeZoomLevel = 0.0) const;
      void contextPopup(QContextMenuEvent* ev);
      bool editKeyLyrics();
      bool editKeySticking();
      void dragScoreView(QMouseEvent* ev);
      void doDragElement(QMouseEvent* ev);
      void doDragLasso(QMouseEvent* ev);
      void doDragFoto(QMouseEvent* ev);
      void doDragEdit(QMouseEvent* ev);
      bool testElementDragTransition(QMouseEvent* ev);
      bool fotoEditElementDragTransition(QMouseEvent* ev);
      void cmdAddSlur(const Slur* slurTemplate = nullptr);
      void addSlur(ChordRest*, ChordRest*, const Slur*) override;
      virtual void cmdAddHairpin(HairpinType);
      void cmdAddNoteLine();

      void setEditElement(Element*);
      void updateEditElement();

      bool noteEntryMode() const { return state == ViewState::NOTE_ENTRY; }
      bool editMode() const      { return state == ViewState::EDIT; }
      bool textEditMode() const  { return editMode() && editData.element && editData.element->isTextBase(); }
      bool hasEditGrips() const  { return editData.element && editData.grips; }
      bool fotoMode() const;

      virtual void setDropRectangle(const QRectF&) override;
      virtual void setDropTarget(const Element*) override;
      void setDropAnchorLines(const QVector<QLineF> &anchorList);
      const QTransform& matrix() const  { return _matrix; }

      ZoomIndex zoomIndex() const { return _zoomIndex; }
      qreal logicalZoomLevel() const;
      qreal physicalZoomLevel() const;
      ZoomState logicalZoom() const { return { _zoomIndex, logicalZoomLevel() }; }
      void setLogicalZoom(const ZoomState& logicalZoom) { setLogicalZoom(logicalZoom.index, logicalZoom.level); }

      ZoomIndex previousZoomIndex() const { return _previousLogicalZoom.index; }
      qreal previousLogicalZoomLevel() const { return _previousLogicalZoom.level; }
      const ZoomState& previousLogicalZoom() const { return _previousLogicalZoom; }
      void setPreviousLogicalZoom(ZoomState previousLogicalZoom) { _previousLogicalZoom = std::move(previousLogicalZoom); }

      qreal xoffset() const;
      qreal yoffset() const;
      void setOffset(qreal x, qreal y);
      QSizeF fsize() const;
      void screenNext();
      void screenPrev();
      void pageNext();
      void pagePrev();
      void pageTop();
      void pageEnd();
      QPointF toLogical(const QPoint& p) const   { return imatrix.map(QPointF(p)); }
      QPointF toPhysical(const QPointF& p) const {return _matrix.map(p); }
      QRectF toLogical(const QRectF& r) const    { return imatrix.mapRect(r); }
      QRect toPhysical(const QRectF& r) const    { return _matrix.mapRect(r).toRect(); }

      QRectF canvasViewport() const { return toLogical(geometry()); }

      bool searchMeasure(int i);
      bool searchPage(int i);
      bool searchRehearsalMark(const QString& s);
      void gotoMeasure(Measure*);
      void setFocusRect();
      void changeVoice(int voice);
      virtual void drawBackground(QPainter* p, const QRectF& r) const override;
      bool fotoScoreViewDragTest(QMouseEvent*);
      bool fotoScoreViewDragRectTest(QMouseEvent*);
      void doDragFotoRect(QMouseEvent*);
      void fotoContextPopup(QContextMenuEvent*);
      bool fotoRectHit(const QPoint& p);
      void paintRect(bool printMode, QPainter& p, const QRectF& r, double mag);
      bool saveFotoAs(bool printMode, const QRectF&);
      void fotoDragDrop(QMouseEvent*);
      void changeEditElement(Element*) override;

      void cmdAppendMeasures(int, ElementType);
      void cmdInsertMeasures(int, ElementType);

      void cmdAddRemoveBreaks();
      void cmdCopyLyricsToClipboard();

      ScoreState mscoreState() const;
      void setCursorVisible(bool v);
      void showOmr(bool flag);
      void midiNoteReceived(int pitch, bool chord, int velocity);

      virtual void moveCursor() override;

      SmoothPanSettings& panSettings() { return _panSettings; }

      virtual void layoutChanged() override;
      virtual void dataChanged(const QRectF&) override;
      virtual void updateAll() override { update(); }
      virtual void adjustCanvasPosition(const Element* el, bool playBack, int staff = -1) override;
      virtual void setCursor(const QCursor& c) override { QWidget::setCursor(c); }
      virtual QCursor cursor() const override { return QWidget::cursor(); }
      void loopUpdate(bool val)   {  loopToggled(val); }

      void moveViewportToLastEdit();

      void updateShadowNotes();

      OmrView* omrView() const        { return _omrView; }
      void setOmrView(OmrView* v)     { _omrView = v;    }
      FotoLasso* fotoLasso() const    { return _foto;    }
      Element* getEditElement();
      void onElementDestruction(Element*) override;

      virtual Element* elementNear(QPointF) override;
      QList<Element*> elementsNear(QPointF);
      void editArticulationProperties(Articulation*);
      void editTimeSigProperties(TimeSig*);
      void editStaffTextProperties(StaffTextBase*);
      void selectInstrument(InstrumentChange*);
      EditData& getEditData()        { return editData; }
      void changeState(ViewState);

      virtual const QRect geometry() const override { return QWidget::geometry(); }

      void updateGrips();
      bool moveWhenInactive() const { return _moveWhenInactive; }
      bool moveWhenInactive(bool move) { bool m = _moveWhenInactive; _moveWhenInactive = move; return m; }

      QElapsedTimer _controlCursorTimer, _playbackCursorTimer;
      friend struct SmoothPanSettings;

      private:
         void drawAnchorLines(QPainter& painter);
      };

} // namespace Ms
#endif

