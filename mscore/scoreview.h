//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: scoreview.h 5599 2012-04-30 08:25:44Z lasconic $
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
class Xml;
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

enum class Grip : signed char;
enum class POS : char;
enum class MagIdx : char;

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
      INSTRUMENT_CHANGE
      };

//---------------------------------------------------------
//   CommandTransition
//---------------------------------------------------------

class CommandTransition : public QAbstractTransition
      {
      QString val;

   protected:
      virtual bool eventTest(QEvent* e);
      virtual void onTransition(QEvent*) {}

   public:
      CommandTransition(const QString& cmd, QState* target) : val(cmd) {
            setTargetState(target);
            }
      };

//---------------------------------------------------------
//   ScoreViewDragTransition
//---------------------------------------------------------

class ScoreViewDragTransition : public QMouseEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual bool eventTest(QEvent* event);

   public:
      ScoreViewDragTransition(ScoreView* c, QState* target);
      };

//---------------------------------------------------------
//   CommandEvent
//---------------------------------------------------------

struct CommandEvent : public QEvent
      {
      QString value;
      CommandEvent(const QString& c)
         : QEvent(QEvent::Type(QEvent::User+1)), value(c) {}
      };

//---------------------------------------------------------
//   ScoreView
//---------------------------------------------------------

class ScoreView : public QWidget, public MuseScoreView {
      Q_OBJECT

      enum States { NORMAL, DRAG, DRAG_OBJECT, EDIT, DRAG_EDIT, LASSO,
            NOTE_ENTRY, MAG, PLAY, ENTRY_PLAY, FOTOMODE,
            STATES
            };

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

      QStateMachine* sm;
      QState* states[STATES];
      bool addSelect;

      QFocusFrame* focusFrame;

      Element* dragElement;   // valid in state DRAG_OBJECT
      Staff* dragStaff;
      qreal staffUserDist;    // valid while dragging a staff

      EditData data;
      Element* curElement;    // current item at mouse press
      QPoint  startMoveI;

      QPointF dragOffset;

      bool scoreViewDragging; // decide if dragging or clearing selection

      // editing mode
      QVector<QRectF> grip;         // edit "grips"
      Grip curGrip;
      int grips;                    // number of used grips
      Grip defaultGrip;              // grip to start editing
      Element* editObject;          ///< Valid in edit mode

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
      Lasso* _foto;

      QColor _bgColor;
      QColor _fgColor;
      QPixmap* _bgPixmap;
      QPixmap* _fgPixmap;

      virtual void paintEvent(QPaintEvent*);
      void paint(const QRect&, QPainter&);

      void objectPopup(const QPoint&, Element*);
      void measurePopup(const QPoint&, Measure*);

      void saveChord(Xml&);

      virtual bool event(QEvent* event);
      virtual bool gestureEvent(QGestureEvent*);
      virtual void resizeEvent(QResizeEvent*);
      virtual void wheelEvent(QWheelEvent*);
      virtual void dragEnterEvent(QDragEnterEvent*);
      virtual void dragLeaveEvent(QDragLeaveEvent*);
      virtual void dragMoveEvent(QDragMoveEvent*);
      virtual void dropEvent(QDropEvent*);
      virtual void focusInEvent(QFocusEvent*);
      virtual void focusOutEvent(QFocusEvent*);
      virtual void mouseReleaseEvent(QMouseEvent*);
      void constraintCanvas(int *dxx, int *dyy);

      void contextItem(Element*);

      void lassoSelect();

      void setShadowNote(const QPointF&);
      void drawElements(QPainter& p,const QList<Element*>& el);
      void dragTimeAnchorElement(const QPointF& pos);
      void dragSymbol(const QPointF& pos);
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
      void cmdAddPitch(int note, bool addFlag, bool insert);
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

      void setupFotoMode();

      MeasureBase* insertMeasure(Element::Type, MeasureBase*);
      MeasureBase* checkSelectionStateForInsertMeasure();

      void appendMeasures(int, Element::Type);
      MeasureBase* appendMeasure(Element::Type);
      void cmdInsertMeasure(Element::Type);
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

   private slots:
      void enterState();
      void exitState();
      void startFotomode();
      void stopFotomode();
      void startFotoDrag();
      void endFotoDrag();
      void endFotoDragEdit();
      void startScoreViewDrag();
      void endScoreViewDrag();

      void posChanged(POS pos, unsigned tick);
      void loopToggled(bool);
      void triggerCmdRealtimeAdvance();
      void cmdRealtimeAdvance();
      void extendCurrentNote();

   public slots:
      void setViewRect(const QRectF&);

      virtual void startEdit();
      void endEdit();
      void endStartEdit() { endEdit(); startEdit(); }

      void startDrag();
      void endDrag();

      void endDragEdit();

      void startNoteEntry();
      void endNoteEntry();

      void endLasso();
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

   signals:
      void viewRectChanged();
      void scaleChanged(double);
      void offsetChanged(double, double);

   public:
      ScoreView(QWidget* parent = 0);
      ~ScoreView();

      QPixmap* fgPixmap() { return _fgPixmap; }

      virtual void startEdit(Element*, Grip);
      void startEdit(Element*);

      void moveCursor(int tick);
      int cursorTick() const;
      void setCursorOn(bool);
      void setBackground(QPixmap*);
      void setBackground(const QColor&);
      void setForeground(QPixmap*);
      void setForeground(const QColor&);

      Page* addPage();
      void modifyElement(Element* obj);
      virtual void setScore(Score* s);
      virtual void removeScore()  { _score = 0; }

      void setMag(qreal m);
      bool navigatorVisible() const;
      void cmd(const QAction* a);

      void startUndoRedo();
      void zoomStep(qreal step, const QPoint& pos);
      void zoom(qreal _mag, const QPointF& pos);
      void contextPopup(QContextMenuEvent* ev);
      void editKey(QKeyEvent*);
      bool editKeyLyrics(QKeyEvent*);
      void dragScoreView(QMouseEvent* ev);
      void dragNoteEntry(QMouseEvent* ev);
      void noteEntryButton(QMouseEvent* ev);
      void doDragElement(QMouseEvent* ev);
      void doDragLasso(QMouseEvent* ev);
      void doDragFoto(QMouseEvent* ev);
      void doDragEdit(QMouseEvent* ev);
      void select(QMouseEvent*);
      bool mousePress(QMouseEvent* ev);
      bool testElementDragTransition(QMouseEvent* ev);
      bool editElementDragTransition(QMouseEvent* ev);
      bool fotoEditElementDragTransition(QMouseEvent* ev);
      bool editScoreViewDragTransition(QMouseEvent* e);
      bool editSelectTransition(QMouseEvent* me);
      void cmdAddSlur();
      virtual void cmdAddHairpin(HairpinType) override;
      void cmdAddNoteLine();
      virtual void cmdAddSlur(Note* firstNote, Note* lastNote);

      bool noteEntryMode() const;
      bool editMode() const;
      bool fotoMode() const;

      void editInputTransition(QInputMethodEvent* ie);
      void onEditPasteTransition(QMouseEvent* ev);

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
      void selectMeasure(int m);
      void postCmd(const char* cmd)   { sm->postEvent(new CommandEvent(cmd)); }
      void setFocusRect();
      Element* getDragElement() const { return dragElement; }
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
      const QRectF& getGrip(Grip n) const { return grip[int(n)]; }
      int gripCount() const { return grips; }              // number of used grips
      void changeEditElement(Element*);

      void cmdAppendMeasures(int, Element::Type);
      void cmdInsertMeasures(int, Element::Type);

      void cmdAddRemoveBreaks();
      void cmdCopyLyricsToClipboard();

      ScoreState mscoreState() const;
      void setCursorVisible(bool v);
      void showOmr(bool flag);
      Element* getCurElement() const { return curElement; }   // current item at mouse press
      void midiNoteReceived(int pitch, bool chord, int velocity);
      void setEditPos(const QPointF&);

      virtual void moveCursor() override;

      virtual void layoutChanged();
      virtual void dataChanged(const QRectF&);
      virtual void updateAll()    { update(); }
      virtual void adjustCanvasPosition(const Element* el, bool playBack);
      virtual void setCursor(const QCursor& c) { QWidget::setCursor(c); }
      virtual QCursor cursor() const { return QWidget::cursor(); }
      void loopUpdate(bool val)   {  loopToggled(val); }

      void updateShadowNotes();

      OmrView* omrView() const    { return _omrView; }
      void setOmrView(OmrView* v) { _omrView = v;    }
      Lasso* fotoLasso() const    { return _foto;    }
      Element* getEditObject()    { return editObject; }
      void setEditObject(Element* e) { editObject = e; }
      virtual Element* elementNear(QPointF);
      void editFretDiagram(FretDiagram*);
      void editBendProperties(Bend*);
      void editTremoloBarProperties(TremoloBar*);
      };

//---------------------------------------------------------
//   DragTransition
//---------------------------------------------------------

class DragTransition : public QEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual void onTransition(QEvent* e);

   public:
      DragTransition(ScoreView* c)
         : QEventTransition(c, QEvent::MouseMove), canvas(c) {}
      };

extern int searchStaff(const Element* element);


} // namespace Ms
#endif

