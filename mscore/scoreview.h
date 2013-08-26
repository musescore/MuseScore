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

enum {
      TEXT_TITLE,
      TEXT_SUBTITLE,
      TEXT_COMPOSER,
      TEXT_POET,
      TEXT_SYSTEM,
      TEXT_STAFF,
      TEXT_REHEARSAL_MARK
      };

//---------------------------------------------------------
//   TextCursor
//---------------------------------------------------------

class TextCursor {
      QRectF _rect;
      bool _visible;
      QColor _color;
      int _tick;

   public:
      TextCursor()                   { _visible = false; }
      QRectF rect() const            { return _rect;     }
      void setRect(const QRectF& r)  { _rect = r;        }
      bool visible() const           { return _visible;  }
      void setVisible(bool val)      { _visible = val;   }
      QColor color() const           { return _color;    }
      void setColor(const QColor& c) { _color = c;       }
      int tick() const               { return _tick;     }
      void setTick(int val)          { _tick = val;      }
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
      static const int MAX_GRIPS = 8;

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
      int _magIdx;

      QStateMachine* sm;
      QState* states[STATES];
      bool addSelect;

      QFocusFrame* focusFrame;

      Element* dragElement;   // valid in state DRAG_OBJECT
      Staff* dragStaff;
      qreal staffUserDist;    // valid while dragging a staff

      Element* curElement;    // current item at mouse press
      QPointF startMove;      // position of last mouse press
      QPoint  startMoveI;

      QPointF dragOffset;

      // editing mode
      int curGrip;
      QRectF grip[MAX_GRIPS];       // edit "grips"
      int grips;                    // number of used grips
      Element* editObject;          ///< Valid in edit mode

      //--input state:
      TextCursor* _cursor;
      ShadowNote* shadowNote;

      // Loop In/Out marks in the score
      TextCursor* _curLoopIn;
      TextCursor* _curLoopOut;

      Lasso* lasso;           ///< temporarily drawn lasso selection
      Lasso* _foto;

      QColor _bgColor;
      QColor _fgColor;
      QPixmap* bgPixmap;
      QPixmap* fgPixmap;

      virtual void paintEvent(QPaintEvent*);
      void paint(const QRect&, QPainter&);

      void objectPopup(const QPoint&, Element*);
      void measurePopup(const QPoint&, Measure*);

      void saveChord(Xml&);

      virtual bool event(QEvent* event);
#if QT_VERSION < 0x050000
      virtual bool gestureEvent(QGestureEvent*);
#endif
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
      bool dragTimeAnchorElement(const QPointF& pos);
      void dragSymbol(const QPointF& pos);
      bool dragMeasureAnchorElement(const QPointF& pos);
      void updateGrips();
      void lyricsTab(bool back, bool end, bool moveOnly);
      void lyricsReturn();
      void lyricsEndEdit();
      void lyricsUpDown(bool up, bool end);
      void lyricsMinus();
      void lyricsUnderscore();
      void harmonyEndEdit();
      void harmonyTab(bool back);
      void harmonyBeatsTab(bool noterest, bool back);
      void harmonyTicksTab(int ticks);
      void figuredBassTab(bool meas, bool back);
      void figuredBassTicksTab(int ticks);
      void figuredBassEndEdit();
      void cmdInsertNote(int note);
      void cmdAddPitch(int note, bool addFlag);
      void cmdAddFret(int fret);
      void cmdAddChordName();
      void cmdAddText(int style);
      void cmdEnterRest(const TDuration&);
      void cmdEnterRest();
      void cmdTuplet(int n, ChordRest*);
      void cmdTuplet(int);
      void cmdCreateTuplet( ChordRest* cr, Tuplet* tuplet);
      void cmdRepeatSelection();
      void cmdChangeEnharmonic(bool);

      void setupFotoMode();

      MeasureBase* insertMeasure(Element::ElementType, MeasureBase*);
      MeasureBase* checkSelectionStateForInsertMeasure();

      void appendMeasures(int, Element::ElementType);
      MeasureBase* appendMeasure(Element::ElementType);
      void cmdInsertMeasure(Element::ElementType);
      void createElementPropertyMenu(Element* e, QMenu*);
      void genPropertyMenu1(Element* e, QMenu* popup);
      void genPropertyMenuText(Element* e, QMenu* popup);
      void elementPropertyAction(const QString&, Element* e);
      void paintPageBorder(QPainter& p, Page* page);
      bool dropCanvas(Element*);
      void editCmd(const QString&);
      void setLoopCursor(TextCursor *curLoop, int tick, bool isInPos);

   private slots:
      void enterState();
      void exitState();
      void startFotomode();
      void stopFotomode();
      void startFotoDrag();
      void endFotoDrag();
      void endFotoDragEdit();

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
      void editPaste();

      void normalCut();
      void normalCopy();
      void normalPaste();

      void cloneElement(Element* e);
      void doFotoDragEdit(QMouseEvent* ev);

   signals:
      void viewRectChanged();
      void scaleChanged(double);
      void offsetChanged(double, double);

   public:
      ScoreView(QWidget* parent = 0);
      ~ScoreView();

      virtual void startEdit(Element*, int startGrip);
      void startEdit(Element*);

//      void moveCursor(Segment*, int track);
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
      void zoom(int step, const QPoint& pos);
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
      int magIdx() const                         { return _magIdx; }
      void setMag(int idx, double mag);
      qreal xoffset() const;
      qreal yoffset() const;
      void setOffset(qreal x, qreal y);
      QSizeF fsize() const;
      void pageNext();
      void pagePrev();
      void pageTop();
      void pageEnd();
      QPointF toLogical(const QPoint& p) const { return imatrix.map(QPointF(p)); }
      QRectF  toLogical(const QRectF& r) const { return imatrix.mapRect(r);      }
      QRect toPhysical(const QRectF& r) const { return _matrix.mapRect(r).toRect(); }

      void search(const QString& s);
      void searchMeasure(int i);
      void searchPage(int i);
      void gotoMeasure(Measure*);
      void selectMeasure(int m);
      void postCmd(const char* cmd)   { sm->postEvent(new CommandEvent(cmd));  }
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
      const QRectF& getGrip(int n) const { return grip[n]; }
      int gripCount() const { return grips; }              // number of used grips
      void changeEditElement(Element*);

      void cmdAppendMeasures(int, Element::ElementType);
      void cmdInsertMeasures(int, Element::ElementType);

      ScoreState mscoreState() const;
      void setCursorVisible(bool v);
      void showOmr(bool flag);
      Element* getCurElement() const { return curElement; }   // current item at mouse press
      void midiNoteReceived(int pitch, bool);
      void setEditPos(const QPointF&);

      int loopInPos()                          { return _curLoopIn->tick();  }
      int loopOutPos()                         { return _curLoopOut->tick(); }
      void setLoopInCursor();
      void setLoopOutCursor();
      virtual void updateLoopCursors();
      void showLoopCursors();
      void hideLoopCursors();

      virtual void moveCursor();
      virtual void layoutChanged();
      virtual void dataChanged(const QRectF&);
      virtual void updateAll();
      virtual void adjustCanvasPosition(const Element* el, bool playBack);
      virtual void setCursor(const QCursor& c) { QWidget::setCursor(c); }
      virtual QCursor cursor() const { return QWidget::cursor(); }

      OmrView* omrView() const    { return _omrView; }
      void setOmrView(OmrView* v) { _omrView = v;    }
      Lasso* fotoLasso() const    { return _foto;    }
      Element* getEditObject()    { return editObject; }
      void setEditObject(Element* e) { editObject = e; }
      virtual Element* elementNear(QPointF);
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

