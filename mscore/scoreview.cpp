//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2017 Werner Schweer & others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "breaksdialog.h"
#include "continuouspanel.h"
#include "drumroll.h"
#include "editdrumset.h"
#include "editstaff.h"
#include "globals.h"
#include "log.h"
#include "measureproperties.h"
#include "musescore.h"
#include "navigator.h"
#include "preferences.h"
#include "scoreaccessibility.h"
#include "scoretab.h"
#include "scoreview.h"
#include "seq.h"
#include "splitstaff.h"
#include "textcursor.h"
#include "textpalette.h"
#include "texttools.h"
#include "tourhandler.h"
#include "zoombox.h"

#include "inspector/inspector.h"

#include "libmscore/articulation.h"
#include "libmscore/barline.h"
#include "libmscore/chord.h"
#include "libmscore/clef.h"
#include "libmscore/dynamic.h"
#include "libmscore/excerpt.h"
#include "libmscore/figuredbass.h"
#include "libmscore/fingering.h"
#include "libmscore/hairpin.h"
#include "libmscore/harmony.h"
#include "libmscore/fret.h"
#include "libmscore/instrchange.h"
#include "libmscore/keysig.h"
#include "libmscore/lasso.h"
#include "libmscore/lyrics.h"
#include "libmscore/measure.h"
#include "libmscore/navigate.h"
#include "libmscore/notedot.h"
#include "libmscore/note.h"
#include "libmscore/noteline.h"
#include "libmscore/page.h"
#include "libmscore/part.h"
#include "libmscore/pitchspelling.h"
#include "libmscore/rehearsalmark.h"
#include "libmscore/repeatlist.h"
#include "libmscore/rest.h"
#include "libmscore/score.h"
#include "libmscore/segment.h"
#include "libmscore/shadownote.h"
#include "libmscore/slur.h"
#include "libmscore/spanner.h"
#include "libmscore/staff.h"
#include "libmscore/stafflines.h"
#include "libmscore/stafftext.h"
#include "libmscore/stafftype.h"
#include "libmscore/sticking.h"
#include "libmscore/stringdata.h"
#include "libmscore/sym.h"
#include "libmscore/system.h"
#include "libmscore/systemtext.h"
#include "libmscore/textframe.h"
#include "libmscore/text.h"
#include "libmscore/timesig.h"
#include "libmscore/tuplet.h"
#include "libmscore/undo.h"
#include "libmscore/utils.h"
#include "libmscore/volta.h"
#include "libmscore/xml.h"
#include "libmscore/textline.h"
#include "libmscore/shape.h"

#ifdef AVSOMR
#include "avsomr/avsomr.h"
#include "avsomr/avsomrdrawer.h"
#endif

namespace Ms {

extern QErrorMessage* errorMessage;

//---------------------------------------------------------
//   ScoreView
//---------------------------------------------------------

ScoreView::ScoreView(QWidget* parent)
   : QWidget(parent), editData(this)
      {
      setObjectName("scoreview");
      setStatusTip("scoreview");
      setAcceptDrops(true);
#ifndef Q_OS_MAC
      setAttribute(Qt::WA_OpaquePaintEvent);
#endif
      setAttribute(Qt::WA_NoSystemBackground);
      setFocusPolicy(Qt::ClickFocus);
      setAttribute(Qt::WA_InputMethodEnabled);
      setAttribute(Qt::WA_KeyCompression);
      setAttribute(Qt::WA_StaticContents);
      setAutoFillBackground(false);

      state       = ViewState::NORMAL;
      _score      = 0;
      _omrView    = 0;
      dropTarget  = 0;

      realtimeTimer = new QTimer(this);
      realtimeTimer->setTimerType(Qt::PreciseTimer);
      connect(realtimeTimer, SIGNAL(timeout()), this, SLOT(triggerCmdRealtimeAdvance()));
      extendNoteTimer = new QTimer(this);
      extendNoteTimer->setTimerType(Qt::PreciseTimer);
      extendNoteTimer->setSingleShot(true);
      connect(extendNoteTimer, SIGNAL(timeout()), this, SLOT(extendCurrentNote()));

      setContextMenuPolicy(Qt::DefaultContextMenu);

      const auto defaultLogicalZoom = ZoomBox::getDefaultLogicalZoom();
      _zoomIndex = defaultLogicalZoom.index;
      const qreal physicalZoomLevel = defaultLogicalZoom.level * (mscore->physicalDotsPerInch() / DPI);
      _matrix = QTransform(physicalZoomLevel, 0.0, 0.0, physicalZoomLevel, 0.0, 0.0);
      imatrix = _matrix.inverted();

      focusFrame  = 0;
      _bgColor    = Qt::darkBlue;
      _fgColor    = Qt::white;
      _fgPixmap    = 0;
      _bgPixmap    = 0;

      editData.curGrip = Grip::NO_GRIP;
      editData.grips   = 0;
      editData.element = 0;

      lasso       = new Lasso(_score);
      _foto       = 0;// new Lasso(_score);

      _cursor     = new PositionCursor(this);
      _cursor->setType(CursorType::POS);

      _controlCursor     = new PositionCursor(this);
      _controlCursor->setType(CursorType::POS);
      _controlCursor->setVisible(preferences.getBool(PREF_PAN_CURSOR_VISIBLE));
      _panSettings.loadFromPreferences();
      _controlModifier = _panSettings.controlModifierBase;

      _continuousPanel = new ContinuousPanel(this);
      _continuousPanel->setActive(true);

      shadowNote  = 0;

      _curLoopIn  = new PositionCursor(this);
      _curLoopIn->setType(CursorType::LOOP_IN);
      _curLoopOut = new PositionCursor(this);
      _curLoopOut->setType(CursorType::LOOP_OUT);

      if (converterMode)      // HACK
            return;

      grabGesture(Qt::PinchGesture);      // laptop pad (Mac) and touchscreen

      //-----------------------------------------------------------------------

      if (MScore::debugMode)
            setMouseTracking(true);

      if (preferences.getBool(PREF_UI_CANVAS_BG_USECOLOR))
            setBackground(MScore::bgColor);
      else {
            QPixmap* pm = new QPixmap(preferences.getString(PREF_UI_CANVAS_BG_WALLPAPER));
            setBackground(pm);
            }
      if (preferences.getBool(PREF_UI_CANVAS_FG_USECOLOR))
            setForeground(preferences.getColor(PREF_UI_CANVAS_FG_COLOR));
      else {
            QPixmap* pm = new QPixmap(preferences.getString(PREF_UI_CANVAS_FG_WALLPAPER));
            if (pm == 0 || pm->isNull())
                  qDebug("no valid pixmap %s", qPrintable(preferences.getString(PREF_UI_CANVAS_FG_WALLPAPER)));
            setForeground(pm);
            }

      connect(getAction("loop"), SIGNAL(toggled(bool)), SLOT(loopToggled(bool)));
      if (seq)
            connect(seq, SIGNAL(stopped()), SLOT(seqStopped()));
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void ScoreView::setScore(Score* s)
      {
      if (_score) {
            if (_score->isMaster()) {
                  MasterScore* ms = static_cast<MasterScore*>(s);
                  for (MasterScore* _ms : *ms->movements()) {
                        _ms->removeViewer(this);
                        disconnect(s, SIGNAL(posChanged(POS, int)), this, SLOT(posChanged(POS,int)));
                        }
                  }
            else {
                  _score->removeViewer(this);
                  disconnect(s, SIGNAL(posChanged(POS, int)), this, SLOT(posChanged(POS,int)));
                  }
            }

      _score = s;
      if (_score) {
            if (_score->isMaster()) {
                  MasterScore* ms = static_cast<MasterScore*>(s);
                  for (MasterScore* _ms : *ms->movements()) {
                        _ms->addViewer(this);
                        }
                  }
            else
                  _score->addViewer(this);
            pageTop(); // (re)set position
            }

      if (shadowNote == 0) {
            shadowNote = new ShadowNote(_score);
            shadowNote->setVisible(false);
            }
      else
            shadowNote->setScore(_score);
      lasso->setScore(s);
      _continuousPanel->setScore(_score);

      if (_score) {
            _curLoopIn->move(s->pos(POS::LEFT));
            _curLoopOut->move(s->pos(POS::RIGHT));
            loopToggled(getAction("loop")->isChecked());

            connect(s, SIGNAL(posChanged(POS,unsigned)), SLOT(posChanged(POS,unsigned)));
            connect(this, SIGNAL(viewRectChanged()), this, SLOT(updateContinuousPanel()));
            }
      }

//---------------------------------------------------------
//   ScoreView
//---------------------------------------------------------

ScoreView::~ScoreView()
      {
      if (_score)
            _score->removeViewer(this);
      delete lasso;
      delete _foto;
      delete _cursor;
      delete _controlCursor;
      delete _continuousPanel;
      delete _curLoopIn;
      delete _curLoopOut;
      delete _bgPixmap;
      delete _fgPixmap;
      delete shadowNote;
      }

//---------------------------------------------------------
//   objectPopup
//    the menu can be extended by Elements with
//      genPropertyMenu()/propertyAction() methods
//---------------------------------------------------------

void ScoreView::objectPopup(const QPoint& pos, Element* obj)
      {
      // show tuplet properties if number is clicked:
      if (obj->isText() && obj->parent() && obj->parent()->isTuplet()) {
            obj = obj->parent();
            if (!obj->selected())
                  obj->score()->select(obj, SelectType::SINGLE, 0);
            }

      QMenu* popup = new QMenu(this);
      popup->setSeparatorsCollapsible(false);
      QAction* a = popup->addSeparator();

      // Set Slur or Tie according to the selected object
      if (obj->type() != ElementType::SLUR_SEGMENT) {
            if ((obj->type() == ElementType::STAFF_TEXT) && (toStaffText(obj)->systemFlag()))
                  a->setText(tr("System Text"));
            else
                  a->setText(obj->userName());
            }
      else if (static_cast<SlurSegment*>(obj)->spanner()->type() == ElementType::SLUR)
            a->setText(tr("Slur"));
      else if (static_cast<SlurSegment*>(obj)->spanner()->type() == ElementType::TIE)
            a->setText(tr("Tie"));

      popup->addAction(getAction("cut"));
      popup->addAction(getAction("copy"));
      popup->addAction(getAction("paste"));
      popup->addAction(getAction("swap"));
      popup->addAction(getAction("delete"));
      if (obj->isNote() || obj->isRest()) {
            a = getAction("time-delete");
            popup->addAction(a);
            }

      QMenu* selMenu = popup->addMenu(tr("Select"));
      selMenu->addAction(getAction("select-similar"));
      selMenu->addAction(getAction("select-similar-staff"));
      selMenu->addAction(getAction("select-similar-range"));
      a = selMenu->addAction(tr("More…"));
      a->setData("select-dialog");

      popup->addSeparator();
      a = getAction("edit-element");
      popup->addAction(a);
      a->setEnabled(obj->isEditable());

      createElementPropertyMenu(obj, popup);

      popup->addSeparator();
      a = popup->addAction(tr("Help"));
      a->setData("help");

#ifndef NDEBUG
      popup->addSeparator();
      popup->addAction("Debugger")->setData("list");
#endif

      popupActive = true;
      a = popup->exec(pos);
      popupActive = false;
      if (a == 0)
            return;
      const QByteArray& cmd(a->data().toByteArray());
      if (cmd == "cut" || cmd =="copy" || cmd == "paste" || cmd == "swap"
         || cmd == "delete" || cmd == "time-delete") {
            // these actions are already activated
            return;
            }
      if (cmd == "list")
            mscore->showElementContext(obj);
      else if (cmd == "help")
            mscore->showHelp(QString("element:%1").arg(obj->name()));
      else if (cmd == "edit-element") {
            if (obj->isEditable()) {
                  if (obj->score())
                        obj->score()->select(obj);
                  startEditMode(obj);
                  return;
                  }
            }
      else if (cmd == "select-similar")
            mscore->selectSimilar(obj, false);
      else if (cmd == "select-similar-staff")
            mscore->selectSimilar(obj, true);
      else if (cmd == "select-similar-range")
            mscore->selectSimilarInRange(obj);
      else if (cmd == "select-dialog")
            mscore->selectElementDialog(obj);
      else if (cmd == "realize-chord-symbols-dialog") {
            if (obj->isEditable()) {
                  // try to construct a reasonable selection
                  if (obj->score()) {
                        Score* score = obj->score();
                        if (score->selection().isRange())
                              mscore->selectSimilarInRange(obj);
                        score->select(obj, SelectType::ADD);
                        }
                  mscore->realizeChordSymbols();
                  }
            }
      else {
            _score->startCmd();
            elementPropertyAction(cmd, obj);
            if (score()->undoStack()->active())
                  _score->endCmd();
            }
      }

//---------------------------------------------------------
//   measurePopup
//---------------------------------------------------------

void ScoreView::measurePopup(QContextMenuEvent* ev, Measure* obj)
      {
      int staffIdx;
      int pitch;
      Segment* seg;

      QPoint gpos = ev->globalPos();

      if (!_score->pos2measure(editData.startMove, &staffIdx, &pitch, &seg, 0))
            return;
      if (staffIdx == -1) {
            qDebug("ScoreView::measurePopup: staffIdx == -1!");
            return;
            }

      Staff* staff = _score->staff(staffIdx);

      QMenu* popup = new QMenu(this);
      popup->setSeparatorsCollapsible(false);

      QAction* a = popup->addSeparator();
      a->setText(tr("Staff"));
      a = popup->addAction(tr("Edit Drumset…"));
      a->setData("edit-drumset");
      a->setEnabled(staff->part()->instrument(obj->tick())->drumset() != 0);

      a = popup->addAction(tr("Piano Roll Editor…"));
      a->setData("pianoroll");

      a = popup->addAction(tr("Staff/Part Properties…"));
      a->setData("staff-properties");
      a = popup->addAction(tr("Split Staff…"));
      a->setData("staff-split");

      a = popup->addSeparator();
      a->setText(tr("Measure"));
      popup->addAction(getAction("cut"));
      popup->addAction(getAction("copy"));
      popup->addAction(getAction("paste"));
      popup->addAction(getAction("swap"));
      popup->addAction(getAction("delete"));
      popup->addAction(getAction("time-delete"));

      popup->addSeparator();
      QMenu* menuAdd = popup->addMenu(tr("Add"));
      menuAdd->addAction(getAction("insert-measure"));
      menuAdd->addAction(getAction("insert-measures"));
      menuAdd->addAction(getAction("insert-hbox"));
      menuAdd->addAction(getAction("insert-vbox"));
      menuAdd->addAction(getAction("insert-textframe"));

      popup->addSeparator();

      a = popup->addAction(tr("Measure Properties…"));
      a->setData("props");
      a->setEnabled(!obj->isMMRest());
      popup->addSeparator();

#ifndef NDEBUG
      popup->addAction("Object Debugger")->setData("list");
#endif

      a = popup->exec(gpos);
      if (a == 0)
            return;
      QString cmd(a->data().toString());
      if (cmd == "cut" || cmd =="copy" || cmd == "paste" || cmd == "swap"
         || cmd == "insert-measure" || cmd == "select-similar"
         || cmd == "delete" || cmd == "time-delete") {
            // these actions are already activated
            return;
            }
      _score->startCmd();
      if (cmd == "list")
            mscore->showElementContext(obj);
      else if (cmd == "color")
            _score->colorItem(obj);
      else if (cmd == "edit") {
            if (obj->isEditable()) {
                  startEditMode(obj);
                  return;
                  }
            }
      else if (cmd == "edit-drumset") {
            EditDrumset drumsetEdit(staff->part()->instrument(obj->tick())->drumset(), this);
            if (drumsetEdit.exec()) {
                  _score->undo(new ChangeDrumset(staff->part()->instrument(obj->tick()), drumsetEdit.drumset()));
                  mscore->updateDrumTools(drumsetEdit.drumset());
                  if (_score->undoStack()->active()) {
                        _score->setLayoutAll();
                        _score->endCmd();
                        }
                  }
            }
      else if (cmd == "drumroll") {
            _score->endCmd();
            mscore->editInDrumroll(staff);
            }
      else if (cmd == "pianoroll") {
            _score->endCmd();
            QPointF p = toLogical(ev->pos());
            Position pp;
            bool foundPos = _score->getPosition(&pp, p, 0);
            mscore->editInPianoroll(staff, foundPos ? &pp : 0);
            }
      else if (cmd == "staff-properties") {
            Fraction tick = obj ? obj->tick() : Fraction(-1,1);
            EditStaff editStaff(staff, tick, this);
            connect(&editStaff, SIGNAL(instrumentChanged()), mscore, SLOT(instrumentChanged()));
            editStaff.exec();
            }
      else if (cmd == "staff-split") {
            SplitStaff splitStaff(this);
            if (splitStaff.exec())
                  _score->splitStaff(staffIdx, splitStaff.getSplitPoint());
            }
      else if (cmd == "props") {
            MeasureProperties im(obj);
            im.exec();
            }
      if (_score->undoStack()->active())
            _score->endCmd();
      }

//---------------------------------------------------------
//   setBackground
//---------------------------------------------------------

void ScoreView::setBackground(QPixmap* pm)
      {
      delete _bgPixmap;
      _bgPixmap = pm;
      update();
      }

void ScoreView::setBackground(const QColor& color)
      {
      delete _bgPixmap;
      _bgPixmap = 0;
      _bgColor = color;
      update();
      }

//---------------------------------------------------------
//   setForeground
//---------------------------------------------------------

void ScoreView::setForeground(QPixmap* pm)
      {
      delete _fgPixmap;
      _fgPixmap = pm;
      update();
      }

void ScoreView::setForeground(const QColor& color)
      {
      delete _fgPixmap;
      _fgPixmap = 0;
      _fgColor = color;
      update();
      }

//---------------------------------------------------------
//   dataChanged
//---------------------------------------------------------

void ScoreView::dataChanged(const QRectF& r)
      {
      update(_matrix.mapRect(r).toRect());  // generate paint event
      }

//---------------------------------------------------------
//   moveCursor
//    move cursor during playback
//---------------------------------------------------------

void ScoreView::moveCursor(const Fraction& tick)
      {
      Measure* measure = score()->tick2measureMM(tick);
      if (measure == 0)
            return;

      qreal x = 0.0;
      Segment* s;
      for (s = measure->first(SegmentType::ChordRest); s;) {
            Fraction t1 = s->tick();
            int x1 = s->canvasPos().x();
            qreal x2;
            Fraction t2;
            Segment* ns = s->next(SegmentType::ChordRest);
            while (ns && !ns->visible())
                  ns = ns->next(SegmentType::ChordRest);
            if (ns) {
                  t2 = ns->tick();
                  x2 = ns->canvasPos().x();
                  }
            else {
                  t2 = measure->endTick();
                  // measure->width is not good enough because of courtesy keysig, timesig
                  Segment* seg = measure->findSegment(SegmentType::EndBarLine, measure->tick() + measure->ticks());
                  if (seg)
                        x2 = seg->canvasPos().x();
                  else
                        x2 = measure->canvasPos().x() + measure->width(); //safety, should not happen
                  }
            if (tick >= t1 && tick < t2) {
                  Fraction   dt = t2 - t1;
                  qreal dx = x2 - x1;
                  x = x1 + dx * (tick-t1).ticks() / dt.ticks();
                  break;
                  }
            s = ns;
            }
      if (s == 0)
            return;

      _cursorColor = QColor(MScore::cursorColor);
      if(_cursorColor.alpha() > MAX_CURSOR_ALPHA)
            _cursorColor.setAlpha(50);
      _cursor->setColor(_cursorColor);
      _cursor->setTick(tick);

      System* system = measure->system();
      if (system == 0)
            return;
      double y        = system->staffYpage(0) + system->page()->pos().y();
      double _spatium = score()->spatium();

      update(_matrix.mapRect(_cursor->rect()).toRect().adjusted(-1,-1,1,1));

      qreal mag = _spatium / SPATIUM20;
      double w  = _spatium * 2.0 + score()->scoreFont()->width(SymId::noteheadBlack, mag);
      double h  = 6 * _spatium;
      //
      // set cursor height for whole system
      //
      double y2 = 0.0;

      for (int i = 0; i < _score->nstaves(); ++i) {
            SysStaff* ss = system->staff(i);
            if (!ss->show() || !_score->staff(i)->show())
                  continue;
            y2 = ss->bbox().bottom();
            }
      h += y2;
      x -= _spatium;
      y -= 3 * _spatium;

      _cursor->setRect(QRectF(x, y, w, h));
      update(_matrix.mapRect(_cursor->rect()).toRect().adjusted(-1,-1,1,1));

      if (_score->layoutMode() == LayoutMode::LINE && seq->isPlaying() && panSettings().enabled)
            moveControlCursor(tick);

      if (mscore->state() == ScoreState::STATE_PLAY && mscore->panDuringPlayback()) {
            adjustCanvasPosition(measure, true);
            }
      }

//---------------------------------------------------------
//   moveControlCursor
//    move the control cursor during playback
//
//   This works by calculating the total length (in sp) of the score
//   and then using that information to calculate the position of the control cursor based on the time passed.
//   Because not all measures have the same width some modifications are made
//   to the distance moved each time that this is called (once every 20ms, or whatever the hearbeat is).
//   That means that not all milliseconds are treated equally, some cause larger and some smaller movement.
//---------------------------------------------------------

void ScoreView::moveControlCursor(const Fraction& tick)
      {
      QColor c(Qt::green);
      c.setAlpha(30);
      _controlCursor->setColor(c);
      _controlCursor->setTick(tick);

      if (_timeElapsed < 0) {
            _timeElapsed = 0;
            }

      int realX = _cursor->rect().x();
      int controlX = _controlCursor->rect().x();
      double distance = realX - controlX;

      if (seq->isPlaying() && isCursorDistanceReasonable()) {
            // playbackCursor in front of the controlCursor
            if (distance > _panSettings.rightDistance)
                  _controlModifier += _panSettings.controlModifierSteps;
            else if (distance > _panSettings.rightDistance1 && _controlModifier < _panSettings.rightMod1)
                  _controlModifier += _panSettings.controlModifierSteps;
            else if (distance > _panSettings.rightDistance2 && _controlModifier < _panSettings.rightMod2)
                  _controlModifier += _panSettings.controlModifierSteps;
            else if (_controlModifier > _panSettings.rightMod1 && distance < _panSettings.rightDistance1)
                  _controlModifier -= _panSettings.controlModifierSteps;
            else if (_controlModifier > _panSettings.rightMod2 && distance < _panSettings.rightDistance2)
                  _controlModifier -= _panSettings.controlModifierSteps;
            else if (_controlModifier > _panSettings.rightMod3 && distance < _panSettings.rightDistance3)
                  _controlModifier = _panSettings.controlModifierBase;
            // playbackCursor behind the controlCursor
            else if (distance < _panSettings.leftDistance)
                  _controlModifier -= _panSettings.controlModifierSteps;
            else if (_controlModifier < _panSettings.leftMod1 && distance > _panSettings.leftDistance1)
                  _controlModifier += _panSettings.controlModifierSteps;
            else if (_controlModifier < _panSettings.leftMod2 && distance > _panSettings.leftDistance2)
                  _controlModifier += _panSettings.controlModifierSteps;
            else if (_controlModifier < _panSettings.leftMod3 && distance > _panSettings.leftDistance3)
                  _controlModifier = _panSettings.controlModifierBase;

            // enforce limits
            if (_controlModifier < _panSettings.minContinuousModifier)
                  _controlModifier = _panSettings.minContinuousModifier;
            else if (_controlModifier > _panSettings.maxContinuousModifier)
                  _controlModifier = _panSettings.maxContinuousModifier;

            double addition = 0;
            if (_controlCursorTimer.isValid()) {
                  if (!_panSettings.advancedWeighting || _controlModifier == _panSettings.minContinuousModifier || _controlModifier == _panSettings.maxContinuousModifier) {
                        addition = _controlCursorTimer.elapsed() * _controlModifier;
                        }
                  else {
                        addition = _controlCursorTimer.elapsed() * _controlModifier * _panSettings.normalWeight + _controlCursorTimer.elapsed() * (_playbackCursorDistanceTravelled/500) * _panSettings.smartWeight;
                        }
                  }
            _timeElapsed += addition;
            }
      else { // reposition the cursor when distance is too great
            double curOffset = _cursor->rect().x() - score()->firstMeasureMM()->pos().x();
            double length = score()->lastMeasureMM()->pos().x() - score()->firstMeasureMM()->pos().x();
            _timeElapsed = (curOffset / length) * score()->durationWithoutRepeats() * 1000;
            _controlModifier = _panSettings.controlModifierBase;
            }

      // Prepare for the next round of calculations
      if (!_controlCursorTimer.isValid() && distance > 100) // distance > 100 used to hack around the count-in option
            _controlCursorTimer.start();
      else if (_controlCursorTimer.isValid())
            _controlCursorTimer.restart();

      if (!_playbackCursorTimer.isValid() && _controlCursorTimer.isValid()) {
            _playbackCursorOldPosition = _cursor->rect().x();
            _playbackCursorTimer.start();
            }
      else if (_playbackCursorTimer.isValid() && _playbackCursorTimer.elapsed() > _panSettings.cursorTimerDuration) {
            _playbackCursorNewPosition = _cursor->rect().x();
            _playbackCursorDistanceTravelled = _playbackCursorNewPosition - _playbackCursorOldPosition;
            _playbackCursorOldPosition = _playbackCursorNewPosition;
            _playbackCursorTimer.restart();
            }


      // Calculate the position of the controlCursor based on the timeElapsed (which is not the real time that has passed)
      qreal x = score()->firstMeasureMM()->pos().x() + (score()->lastMeasureMM()->pos().x() - score()->firstMeasureMM()->pos().x()) * (_timeElapsed / (score()->durationWithoutRepeats() * 1000));
      x -= score()->spatium();
      _controlCursor->setRect(QRectF(x, _cursor->rect().y(), _cursor->rect().width(), _cursor->rect().height()));
      update(_matrix.mapRect(_controlCursor->rect()).toRect().adjusted(-1,-1,1,1));
      }

//---------------------------------------------------------
//   isCursorDistanceReasonable
//    check if the control cursor needs to be teleported
//    to catch up with the playback cursor (for smooth panning)
//---------------------------------------------------------

bool ScoreView::isCursorDistanceReasonable()
      {
      qreal viewWidth = canvasViewport().width();
      qreal controlX = _controlCursor->rect().x();
      qreal playbackX = _cursor->rect().x();
      qreal cursorDistance = abs(controlX - playbackX);
      double maxLeftDistance = viewWidth * (_panSettings.controlCursorScreenPos + 0.07); // 0.05 left margin + 0.02 for making this less sensitive
      double maxRightDistance = viewWidth * (1 - _panSettings.controlCursorScreenPos + 0.15); // teleporting to the right is harder to trigger (we don't want to overdo it)

      if (controlX < playbackX && _panSettings.teleportRightEnabled)
            return cursorDistance < maxRightDistance;

      if (playbackX < controlX && _panSettings.teleportLeftEnabled)
            return cursorDistance < maxLeftDistance;

      return true;
      }

//---------------------------------------------------------
//   moveControlCursorNearCursor
///     used to position the control cursor correctly
///     when starting playback
//---------------------------------------------------------

void ScoreView::moveControlCursorNearCursor()
      {
      double curOffset = _cursor->rect().x() - score()->firstMeasure()->pos().x();
      double length = score()->lastMeasure()->pos().x() - score()->firstMeasure()->pos().x();
      _timeElapsed = (curOffset / length) * score()->durationWithoutRepeats() * 1000;
      qreal x = score()->firstMeasure()->pos().x() + (score()->lastMeasure()->pos().x() - score()->firstMeasure()->pos().x()) * (_timeElapsed / (score()->durationWithoutRepeats() * 1000));
      x -= score()->spatium();
      _controlCursor->setRect(QRectF(x, _cursor->rect().y(), _cursor->rect().width(), _cursor->rect().height()));
      }

//---------------------------------------------------------
//   moveCursor
//    move cursor in note input mode
//---------------------------------------------------------

void ScoreView::moveCursor()
      {
      const InputState& is = _score->inputState();
      Segment* segment = is.segment();
      if (segment && score()->styleB(Sid::createMultiMeasureRests) && segment->measure()->hasMMRest()) {
            Measure* m = segment->measure()->mmRest();
            segment = m->findSegment(SegmentType::ChordRest, m->tick());
            }
      if (!segment)
            return;

      int track    = is.track() == -1 ? 0 : is.track();
      int voice    = track % VOICES;
      int staffIdx = track / VOICES;

      QColor c(MScore::selectColor[voice]);
      c.setAlpha(50);
      _cursor->setColor(c);
      _cursor->setTick(segment->tick());

      System* system = segment->measure()->system();
      if (system == 0) {
            // a new measure was appended but no layout took place
            // or this measure was skipped by a multi measure rest
            return;
            }
      double x        = segment->canvasPos().x();
      double y        = system->staffYpage(staffIdx) + system->page()->pos().y();
      double _spatium = score()->spatium();
      x              -= qMin(segment->pos().x() - score()->styleP(Sid::barNoteDistance), 0.0);

      update(_matrix.mapRect(_cursor->rect()).toRect().adjusted(-1,-1,1,1));

      double h;
      qreal mag               = _spatium / SPATIUM20;
      double w                = _spatium * 2.0 + score()->scoreFont()->width(SymId::noteheadBlack, mag);
      Staff* staff            = score()->staff(staffIdx);
      const StaffType* staffType    = staff->staffType(is.tick());
      double lineDist         = staffType->lineDistance().val() * _spatium;
      int lines               = staffType->lines();
      int strg                = is.string();          // strg refers to an instrument physical string
      x                       -= _spatium;
      int instrStrgs          = staff->part()->instrument(is.tick())->stringData()->strings();
      // if on a TAB staff and InputState::_string makes sense,
      // draw cursor around single string
      if (staff->isTabStaff(is.tick()) && strg >= 0 && strg <= instrStrgs) {
            h = lineDist;                 // cursor height is one full line distance
            y += staffType->physStringToYOffset(strg) * _spatium;
            // if frets are on lines, centre on string; if frets are above lines, 'sit' above string
            y -= (staffType->onLines() ? lineDist * 0.5 : lineDist);
            // look for a note on this string in this staff
            // if found, it will be selected, to synchronize the 'new note input cursor' and the 'current note cursor'
            // i.e. the point where a new note would be added and the existing note which receives any editing
            // (like pitch change or articulation addition)
            bool        done  = false;
            Segment*    seg   = is.segment();
            int         minTrack = (is.track() / VOICES) * VOICES;
            int         maxTrack = minTrack + VOICES;
            // get selected chord, if one exists and is in this segment
            ChordRest* scr = _score->selection().cr();
            if (scr && (scr->type() != ElementType::CHORD || scr->segment() != seg))
                  scr = nullptr;
            // get the physical string corresponding to current visual string
            for (int t = minTrack; t < maxTrack; t++) {
                  Element* e = seg->element(t);
                  if (e != nullptr && e->type() == ElementType::CHORD) {
                        // if there is a selected chord in this segment on this track but it is not e
                        // then the selected chord must be a grace note chord, and we should use it
                        if (scr && scr->track() == t && scr != e)
                              e = scr;
                        // search notes looking for one on current string
                        for (Note* n : static_cast<Chord*>(e)->notes())
                              // if note found on this string, make it current
                              if (n->string() == strg) {
                                    if (!n->selected()) {
                                          _score->select(n);
                                          // restore input state after selection
                                          _score->inputState().setTrack(track);
                                          }
#if 0
                                    // if using this code, we can delete the setTrack() call above
                                    // the code below forces input state & cursor to match current note
                                    _score->inputState().setTrack(t);
                                    QColor c(MScore::selectColor[t % VOICES]);
                                    c.setAlpha(50);
                                    _cursor->setColor(c);
#endif
                                    done = true;
                                    break;
                                    }
                        }
                  if (done)
                        break;
                  }
      }
      // otherwise, draw cursor across whole staff
      else {
            h = (lines - 1) * lineDist + 4 * _spatium;
            y -= 2.0 * _spatium;
            }
      _cursor->setRect(QRectF(x, y, w, h));
      update(_matrix.mapRect(_cursor->rect()).toRect().adjusted(-1,-1,1,1));
      if (is.cr())
            adjustCanvasPosition(is.cr(), false);
      }

//---------------------------------------------------------
//   cursorTick
//---------------------------------------------------------

Fraction ScoreView::cursorTick() const
      {
      return _cursor->tick();
      }

//---------------------------------------------------------
//   setCursorOn
//---------------------------------------------------------

void ScoreView::setCursorOn(bool val)
      {
      if (_cursor && (_cursor->visible() != val)) {
            _cursor->setVisible(val);
            update(_matrix.mapRect(_cursor->rect()).toRect().adjusted(-1,-1,1,1));
            }
      }

//---------------------------------------------------------
//   setLoopCursor
//    adjust the cursor shape and position to mark the loop
//    isInPos is used to adjust the x position of In vs Out mark
//---------------------------------------------------------

void ScoreView::setLoopCursor(PositionCursor *curLoop, const Fraction& tick, bool isInPos)
      {
      //
      // set mark height for whole system
      //
      Measure* measure = score()->tick2measure(tick);
      if (measure == 0)
            return;
      qreal x = 0.0;

      Segment* s;
      for (s = measure->first(SegmentType::ChordRest); s;) {
            Fraction t1 = s->tick();
            int x1 = s->canvasPos().x();
            qreal x2;
            Fraction t2;
            Segment* ns = s->next(SegmentType::ChordRest);
            if (ns) {
                  t2 = ns->tick();
                  x2 = ns->canvasPos().x();
                  }
            else {
                  t2 = measure->endTick();
                  x2 = measure->canvasPos().x() + measure->width();
                  }
            if (tick >= t1 && tick < t2) {
                  Fraction   dt = t2 - t1;
                  qreal dx = x2 - x1;
                  x = x1 + dx * (tick.ticks() - t1.ticks()) / dt.ticks();
                  break;
                  }
            s = ns;
            }
      if (s == 0)
            return;

      System* system = measure->system();
      if (system == 0)
            return;
      double y        = system->staffYpage(0) + system->page()->pos().y();
      double _spatium = score()->spatium();

      qreal mag = _spatium / SPATIUM20;
      double w  = (_spatium * 2.0 + score()->scoreFont()->width(SymId::noteheadBlack, mag))/3;
      double h  = 6 * _spatium;
      //
      // set cursor height for whole system
      //
      double y2 = 0.0;

      for (int i = 0; i < _score->nstaves(); ++i) {
            SysStaff* ss = system->staff(i);
            if (!ss->show() || !_score->staff(i)->show())
                  continue;
            y2 = ss->y() + ss->bbox().height();
            }
      h += y2;
      y -= 3 * _spatium;

      if (isInPos) {
            x = x - _spatium + w/1.5;
            }
      else {
            x = x - _spatium;
            }
      curLoop->setTick(tick);
      update(_matrix.mapRect(curLoop->rect()).toRect().adjusted(-1,-1,1,1));
      curLoop->setRect(QRectF(x, y, w, h));
      update(_matrix.mapRect(curLoop->rect()).toRect().adjusted(-1,-1,1,1));
      }

//---------------------------------------------------------
//   setShadowNote
//---------------------------------------------------------

void ScoreView::setShadowNote(const QPointF& p)
      {
      const InputState& is = _score->inputState();
      Position pos;
      if (!score()->getPosition(&pos, p, is.voice())) {
            shadowNote->setVisible(false);
            return;
            }
      // in any empty measure, pos will be right next to barline
      // so pad this by barNoteDistance
      qreal mag     = score()->staff(pos.staffIdx)->mag(Fraction(0,1));
      qreal relX    = pos.pos.x() - pos.segment->measure()->canvasPos().x();
      pos.pos.rx() -= qMin(relX - score()->styleP(Sid::barNoteDistance) * mag, 0.0);
      shadowNote->setVisible(true);
      Staff* staff = score()->staff(pos.staffIdx);
      shadowNote->setMag(staff->mag(Fraction(0,1)));
      const Instrument* instr       = staff->part()->instrument(shadowNote->tick()); // or pos.segment->tick()
      NoteHead::Group noteheadGroup = NoteHead::Group::HEAD_NORMAL;
      int line                      = pos.line;
      NoteHead::Type noteHead       = is.duration().headType();

      if (instr->useDrumset()) {
            const Drumset* ds  = instr->drumset();
            int pitch    = is.drumNote();
            if (pitch >= 0 && ds->isValid(pitch)) {
                  line     = ds->line(pitch);
                  noteheadGroup = ds->noteHead(pitch);
                  }
            }

      shadowNote->setLine(line);

      int voice;
      if (is.drumNote() != -1 && is.drumset() && is.drumset()->isValid(is.drumNote()))
            voice = is.drumset()->voice(is.drumNote());
      else
            voice = is.voice();

      SymId symNotehead;
      TDuration d(is.duration());

      if (is.rest()) {
            int yo;
            Rest rest(gscore, d.type());
            rest.setTicks(d.fraction());
            symNotehead = rest.getSymbol(is.duration().type(), 0, staff->lines(pos.segment->tick()), &yo);
            shadowNote->setState(symNotehead, voice, d, true);
            }
      else {
            if (NoteHead::Group::HEAD_CUSTOM == noteheadGroup)
                  symNotehead = instr->drumset()->noteHeads(is.drumNote(), noteHead);
            else
                  symNotehead = Note::noteHead(0, noteheadGroup, noteHead);

            shadowNote->setState(symNotehead, voice, d);
            }

      shadowNote->layout();
      shadowNote->setPos(pos.pos);
      }

//---------------------------------------------------------
//   drawAnchorLines
//---------------------------------------------------------
void ScoreView::drawAnchorLines(QPainter& painter)
      {
      if (m_dropAnchorLines.isEmpty())
            return;

      const auto dropAnchorColor = preferences.getColor(PREF_UI_SCORE_VOICE4_COLOR);
      QPen pen(QBrush(dropAnchorColor), 2.0 / painter.worldTransform().m11(), Qt::DotLine);

      for (const QLineF& anchor : m_dropAnchorLines) {
            painter.setPen(pen);
            painter.drawLine(anchor);

            qreal d = 4.0 / painter.worldTransform().m11();
            QRectF rect(-d, -d, 2 * d, 2 * d);

            painter.setBrush(QBrush(dropAnchorColor));
            painter.setPen(Qt::NoPen);
            rect.moveCenter(anchor.p1());
            painter.drawEllipse(rect);
            rect.moveCenter(anchor.p2());
            painter.drawEllipse(rect);
            }
      }

//---------------------------------------------------------
//   paintEvent
//    Note: desktop background and paper background are not
//    scaled
//---------------------------------------------------------
void ScoreView::paintEvent(QPaintEvent* ev)
      {
      if (!_score)
            return;
      QPainter vp(this);
      vp.setRenderHint(QPainter::Antialiasing, preferences.getBool(PREF_UI_CANVAS_MISC_ANTIALIASEDDRAWING));
      vp.setRenderHint(QPainter::TextAntialiasing, true);

      paint(ev->rect(), vp);

      vp.setTransform(_matrix);
      vp.setClipping(false);

      _curLoopIn->paint(&vp);
      _curLoopOut->paint(&vp);
      _cursor->paint(&vp);
      if (_score->layoutMode() == LayoutMode::LINE)
            _controlCursor->paint(&vp);

      if (_score->layoutMode() == LayoutMode::LINE)
            _continuousPanel->paint(ev->rect(), vp);

      if (!lasso->bbox().isEmpty())
            lasso->draw(&vp);
      shadowNote->draw(&vp);

      drawAnchorLines(vp);
      }

//---------------------------------------------------------
//   drawBackground
//---------------------------------------------------------

void ScoreView::drawBackground(QPainter* p, const QRectF& r) const
      {
      if (score()->printing()) {
            p->fillRect(r, Qt::white);
            return;
            }
      if (_fgPixmap == 0 || _fgPixmap->isNull())
            p->fillRect(r, _fgColor);
      else {
            p->drawTiledPixmap(r, *_fgPixmap, r.topLeft()
               - QPoint(lrint(_matrix.dx()), lrint(_matrix.dy())));
            }
      }

//---------------------------------------------------------
//   paintPageBorder
//---------------------------------------------------------

void ScoreView::paintPageBorder(QPainter& p, Page* page)
      {
      //add a black border to pages
      QRectF r(page->canvasBoundingRect());
      p.setBrush(Qt::NoBrush);
      p.setPen(QPen(QColor(0,0,0,102), 1));
      p.drawRect(r);

      if (_score->showPageborders()) {
            // show page margins
            p.setBrush(Qt::NoBrush);
            p.setPen(MScore::frameMarginColor);
            QRectF f(page->canvasBoundingRect());
            f.adjust(page->lm(), page->tm(), -page->rm(), -page->bm());
            p.drawRect(f);
            if (!page->isOdd())
                  p.drawLine(f.right(), 0.0, f.right(), f.bottom());
            }
      }

#ifndef NDEBUG
//---------------------------------------------------------
//   drawDebugInfo
//---------------------------------------------------------

static void drawDebugInfo(QPainter& p, const Element* _e)
      {
      if (!MScore::showBoundingRect)
            return;
      const Element* e = _e;
      //
      //  draw bounding box rectangle for all
      //  selected Elements
      //
      QPointF pos(e->pagePos());
      p.translate(pos);
      p.setBrush(Qt::NoBrush);

      p.setPen(QPen(Qt::red, 0.0));
//      p.drawRect(e->bbox());
      e->shape().paint(p);

      p.setPen(QPen(Qt::red, 0.0));             // red x at 0,0 of bbox
      qreal w = 5.0 / p.worldTransform().m11();
      qreal h = w;
      qreal x = 0; // e->bbox().x();
      qreal y = 0; // e->bbox().y();
      p.drawLine(QLineF(x-w, y-h, x+w, y+h));
      p.drawLine(QLineF(x+w, y-h, x-w, y+h));

      p.translate(-pos);
      if (e->parent()) {
            const Element* ee = e->parent();
            if (e->isNote())
                  ee = toNote(e)->chord()->segment();
            else if (e->isClef())
                  ee = toClef(e)->segment();

            p.setPen(QPen(Qt::green, 0.0));

            p.drawRect(ee->pageBoundingRect());

            if (ee->isSegment()) {
                  QPointF pt = ee->pagePos();
                  p.setPen(QPen(Qt::blue, 0.0));
                  p.drawLine(QLineF(pt.x()-w, pt.y()-h, pt.x()+w, pt.y()+h));
                  p.drawLine(QLineF(pt.x()+w, pt.y()-h, pt.x()-w, pt.y()+h));
                  }
            }
      }
#endif

//---------------------------------------------------------
//   drawElements
//---------------------------------------------------------

void ScoreView::drawElements(QPainter& painter, QList<Element*>& el, Element* editElement)
      {
      std::stable_sort(el.begin(), el.end(), elementLessThan);
      for (const Element* e : el) {
            e->itemDiscovered = 0;

            // harmony element representation is different in edit mode, so don't
            // all normal draw(). Complete drawing is done in drawEditMode()
            if (e == editElement)
                  continue;

            if (!e->visible() && (score()->printing() || !score()->showInvisible()))
                  continue;
            if (e->isRest() && toRest(e)->isGap())
                  continue;
            QPointF pos(e->pagePos());
            painter.translate(pos);
            e->draw(&painter);
            painter.translate(-pos);
#ifndef NDEBUG
            if (e->selected())
                  drawDebugInfo(painter, e);
#endif
            }
      }

//---------------------------------------------------------
//   paint
//---------------------------------------------------------

void ScoreView::paint(const QRect& r, QPainter& p)
      {
      p.save();
      if (_fgPixmap == 0 || _fgPixmap->isNull())
            p.fillRect(r, _fgColor);
      else {
            p.drawTiledPixmap(r, *_fgPixmap, r.topLeft()
               - QPoint(lrint(_matrix.dx()), lrint(_matrix.dy())));
            }

      p.setTransform(_matrix);
      QRectF fr = imatrix.mapRect(QRectF(r));

      Element* editElement = 0;
      Lasso* lassoToDraw = 0;
      if (editData.element) {
            switch (state) {
                  case ViewState::NORMAL:
                        if (editData.element->normalModeEditBehavior() == Element::EditBehavior::Edit)
                              editData.element->drawEditMode(&p, editData);
                        break;
                  case ViewState::DRAG:
                  case ViewState::DRAG_OBJECT:
                  case ViewState::LASSO:
                  case ViewState::NOTE_ENTRY:
                  case ViewState::PLAY:
                  case ViewState::ENTRY_PLAY:
                        break;
                  case ViewState::EDIT:
                  case ViewState::DRAG_EDIT:
                  case ViewState::FOTO:
                  case ViewState::FOTO_DRAG:
                  case ViewState::FOTO_DRAG_EDIT:
                  case ViewState::FOTO_DRAG_OBJECT:
                  case ViewState::FOTO_LASSO:
                        if (editData.element->isLasso())
                              lassoToDraw = toLasso(editData.element);
                        else
                              editData.element->drawEditMode(&p, editData);

                        if (editData.element->isHarmony())
                              editElement = editData.element;     // do not call paint() method
                        break;
                  }
            }


      // AvsOmr -----
#ifdef AVSOMR
      Avs::AvsOmrDrawer omrDrawer;
      std::shared_ptr<Avs::AvsOmrDrawer::Context> omrDrawCtx = omrDrawer.makeContext(&p, _score);
      auto pageMeasures = [](Page* page) -> QList<const Measure*> {
            QList<const Measure*> ml;
            for (const System* s : page->systems()) {
                  const std::vector<MeasureBase*>& measures = s->measures();
                  for (const MeasureBase* mb : measures) {
                        if (mb->isMeasure())
                              ml << toMeasure(mb);
                        }
                  }
            return ml;
            };
#endif
      // ------------

      QRegion r1(r);
      if ((_score->layoutMode() == LayoutMode::LINE) || (_score->layoutMode() == LayoutMode::SYSTEM)) {
            if (_score->pages().size() > 0) {

                  Page* page = _score->pages().front();
                  QList<Element*> ell = page->items(fr);

                  // AvsOmr -----
#ifdef AVSOMR
                  if (omrDrawCtx) {
                        QList<const Measure*> ml = pageMeasures(page);
                        omrDrawer.draw(omrDrawCtx, ml);
                        }
#endif
                  // ------------

                  drawElements(p, ell, editElement);
                  }
            }
      else {
            for (Page* page : _score->pages()) {
                  QRectF pr(page->abbox().translated(page->pos()));
                  if (pr.right() < fr.left())
                        continue;
                  if (pr.left() > fr.right())
                        break;

                  if (!score()->printing())
                        paintPageBorder(p, page);
                  QList<Element*> ell = page->items(fr.translated(-page->pos()));
                  QPointF pos(page->pos());
                  p.translate(pos);

                  // AvsOmr -----
#ifdef AVSOMR
                  if (omrDrawCtx) {
                        QList<const Measure*> ml = pageMeasures(page);
                        omrDrawer.draw(omrDrawCtx, ml);
                        }
#endif
                  // ------------

                  drawElements(p, ell, editElement);

#ifndef NDEBUG
                  if (!score()->printing()) {
                        if (MScore::showSystemBoundingRect) {
                              for (const System* system : page->systems()) {
                                    QPointF pt(system->ipos());
                                    qreal h = system->height() + system->minBottom() + system->minTop();
                                    p.translate(pt);
                                    QRectF rect(0.0, -system->minTop(), system->width(), h);
                                    p.drawRect(rect);
                                    p.translate(-pt);
                                    }
                              }
                        if (MScore::showSegmentShapes) {
                              for (const System* system : page->systems()) {
                                    for (const MeasureBase* mb : system->measures()) {
                                          if (mb->type() == ElementType::MEASURE) {
                                                const Measure* m = static_cast<const Measure*>(mb);
                                                p.setBrush(Qt::NoBrush);
                                                p.setPen(QPen(QBrush(Qt::darkYellow), 0.5));
                                                for (const Segment* s = m->first(); s; s = s->next()) {
                                                      for (int i = 0; i < score()->nstaves(); ++i) {
                                                            QPointF pt(s->pos().x() + m->pos().x() + system->pos().x(),
                                                               system->staffYpage(i));
                                                            p.translate(pt);
                                                            s->shapes().at(i).paint(p);
                                                            p.translate(-pt);
                                                            }
                                                      }
                                                }
                                          }
                                    }
                              }
                        if (MScore::showSkylines) {
                              for (const System* system : page->systems()) {
                                    for (SysStaff* ss : *system->staves()) {
                                          if (!ss->show())
                                                continue;
                                          QPointF pt(system->ipos().x(), system->ipos().y() + ss->y());
                                          p.translate(pt);
                                          ss->skyline().paint(p);
                                          p.translate(-pt);
                                          }
                                    }
                              }
                        if (MScore::showCorruptedMeasures) {
                              double _spatium = score()->spatium();
                              QPen pen;
                              pen.setColor(Qt::red);
                              pen.setWidthF(4);
                              pen.setStyle(Qt::SolidLine);
                              p.setPen(pen);
                              p.setBrush(Qt::NoBrush);
                              for (const System* system : page->systems()) {
                                    for (const MeasureBase* mb : system->measures()) {
                                          if (mb->type() == ElementType::MEASURE) {
                                                const Measure* m = static_cast<const Measure*>(mb);
                                                for (int staffIdx = 0; staffIdx < m->score()->nstaves(); staffIdx++) {
                                                      if (m->corrupted(staffIdx)) {
                                                            p.drawRect(m->staffabbox(staffIdx).adjusted(0, -_spatium, 0, _spatium));
                                                            }
                                                      }
                                                }
                                          }
                                    }
                              }
                        }
#endif

                  p.translate(-pos);
                  r1 -= _matrix.mapRect(pr).toAlignedRect();
                  }
            }
      if (dropRectangle.isValid())
            p.fillRect(dropRectangle, QColor(80, 0, 0, 80));

      const Selection& sel = _score->selection();
      if (sel.isRange()) {
            Segment* ss = sel.startSegment();
            Segment* es = sel.endSegment();

            if (!ss)
                  return;

            if (!ss->enabled())
                  ss = ss->next1MMenabled();
            if (es && !es->enabled())
                  es = es->next1MMenabled();
            if (es && ss->tick() > es->tick())  // start after end?
                  return;

            if (!ss->measure()->system()) {
                  // segment is in a measure that has not been laid out yet
                  // this can happen in mmrests
                  // first chordrest segment of mmrest instead
                  const Measure* mmr = ss->measure()->coveringMMRestOrThis();
                  if (mmr && mmr->system())
                        ss = mmr->first(SegmentType::ChordRest);
                  else
                        return;                 // still no system?
                  if (!ss)
                        return;                 // no chordrest segment?
                  }

            p.setBrush(Qt::NoBrush);

            QPen pen;
            pen.setColor(MScore::selectColor[0]);
            pen.setWidthF(2.0 / p.worldTransform().m11());

            pen.setStyle(Qt::SolidLine);

            p.setPen(pen);
            double _spatium = score()->spatium();
            double x2      = ss->pagePos().x() - _spatium;
            int staffStart = sel.staffStart();
            int staffEnd   = sel.staffEnd();

            System* system2 = ss->measure()->system();
            QPointF pt      = ss->pagePos();
            double y        = pt.y();
            SysStaff* ss1   = system2->staff(staffStart);

            // find last visible staff:
            int lastStaff = 0;
            for (int i = staffEnd-1; i >= 0; --i) {
                  if (score()->staff(i)->show()) {
                        lastStaff = i;
                        break;
                        }
                  }
            SysStaff* ss2 = system2->staff(lastStaff);

            double y1 = ss1->y() - 2 * score()->staff(staffStart)->spatium(Fraction(0,1)) + y;
            double y2 = ss2->y() + ss2->bbox().height() + 2 * score()->staff(lastStaff)->spatium(Fraction(0,1)) + y;

            // drag vertical start line
            p.drawLine(QLineF(x2, y1, x2, y2).translated(system2->page()->pos()));

            System* system1 = system2;
            double x1;

            for (Segment* s = ss; s && (s != es); ) {
                  Segment* ns = s->next1MMenabled();
                  system1  = system2;
                  system2  = s->measure()->system();
                  if (!system2) {
                        // as before, use mmrest if necessary
                        const Measure* mmr = s->measure()->coveringMMRestOrThis();
                        if (mmr)
                              system2 = mmr->system();
                        if (!system2)
                              break;
                        // extend rectangle to end of mmrest
                        pt = mmr->last()->pagePos();
                        }
                  else
                        pt = s->pagePos();
                  x1  = x2;
                  x2  = pt.x() + _spatium * 2;

                  if (ns == 0 || ns == es) {    // last segment?
                        // if any staff in selection has measure rest or repeat measure in last measure,
                        // extend rectangle to bar line
                        Segment* fs = s->measure()->first(SegmentType::ChordRest);
                        if (fs) {
                              for (int i = staffStart; i < staffEnd; ++i) {
                                    if (!score()->staff(i)->show())
                                          continue;
                                    ChordRest* cr = static_cast<ChordRest*>(fs->element(i * VOICES));
                                    if (cr && (cr->type() == ElementType::REPEAT_MEASURE || cr->durationType() == TDuration::DurationType::V_MEASURE)) {
                                          x2 = s->measure()->abbox().right() - _spatium * 0.5;
                                          break;
                                          }
                                    }
                              }
                        }

                  if (system2 != system1)
                        x1  = x2 - 2 * _spatium;
                  y   = pt.y();
                  ss1 = system2->staff(staffStart);
                  ss2 = system2->staff(lastStaff);
                  y1  = ss1->y() - 2 * score()->staff(staffStart)->spatium(s->tick()) + y;
                  y2  = ss2->y() + ss2->bbox().height() + 2 * score()->staff(lastStaff)->spatium(s->tick()) + y;
                  p.drawLine(QLineF(x1, y1, x2, y1).translated(system2->page()->pos()));
                  p.drawLine(QLineF(x1, y2, x2, y2).translated(system2->page()->pos()));
                  s = ns;
                  }
            //
            // draw vertical end line
            //
            p.drawLine(QLineF(x2, y1, x2, y2).translated(system2->page()->pos()));
            }

      // Draw foto lasso to ensure that it is above everything else
      if (lassoToDraw)
            lassoToDraw->drawEditMode(&p, editData);

      p.setWorldMatrixEnabled(false);
      if (_score->layoutMode() != LayoutMode::LINE && _score->layoutMode() != LayoutMode::SYSTEM && !r1.isEmpty()) {
            p.setClipRegion(r1);  // only background
            if (_bgPixmap == 0 || _bgPixmap->isNull())
                  p.fillRect(r, _bgColor);
            else
                  p.drawTiledPixmap(r, *_bgPixmap, r.topLeft() - QPoint(_matrix.m31(), _matrix.m32()));
            }
      p.restore();
      }

//---------------------------------------------------------
//   zoomBySteps
//    Zooms in or out by the specified number of keyboard- or mouse-based zoom steps (positive to zoom in, negative to zoom out).
//    usingMouse is optional and may be omitted to zoom by keyboard-based steps.
//    pos is optional and may be omitted to zoom relative to the top-left corner.
//---------------------------------------------------------

void ScoreView::zoomBySteps(const qreal numSteps, const bool usingMouse/* = false*/, const QPointF& pos/* = QPointF()*/)
      {
      // Calculate the new logical zoom level by multiplying it the current logical zoom level by the factor necessary to get it
      // to double every N steps, where N is the user's preferred "precision" for the specified zoom method (keyboard or mouse).
      const auto precision = preferences.getInt(usingMouse ? PREF_UI_CANVAS_ZOOM_PRECISION_MOUSE : PREF_UI_CANVAS_ZOOM_PRECISION_KEYBOARD);
      const auto stepFactor = std::pow(2.0, 1.0 / qBound(ZOOM_PRECISION_MIN, precision, ZOOM_PRECISION_MAX));
      auto logicalLevel = logicalZoomLevel() * std::pow(stepFactor, numSteps);

      // Floating-point calculations inevitably introduce rounding errors. Check if the new logical zoom level is very close to
      // the mathematically correct value based on the current step; if it is, snap it to the right value. This is necessary in
      // order to avoid accumulating rounding errors as the user repeatedly zooms in and out.
      static constexpr qreal epsilon = 0.0001;
      const auto levelCheck = precision * std::log2(logicalLevel);
      if (std::abs(std::remainder(levelCheck, 1.0)) < epsilon)
            logicalLevel = std::pow(2.0, (levelCheck - std::remainder(levelCheck, 1.0)) / precision);

      // If the new zoom level is exactly equal to one of the numeric presets, use the preset; otherwise, it's free zoom.
      const auto i = std::find(zoomEntries.cbegin(), zoomEntries.cend(), static_cast<int>(100.0 * logicalLevel));
      const auto index = ((i != zoomEntries.cend()) && i->isNumericPreset() && (qFuzzyCompare(i->level, 100.0 * logicalLevel))) ? i->index : ZoomIndex::ZOOM_FREE;

      setLogicalZoom(index, logicalLevel, pos);
      }

//-----------------------------------------------------------------------------
//   constraintCanvas
//-----------------------------------------------------------------------------

void ScoreView::constraintCanvas (int* dxx, int* dyy)
      {
      if (score()->layoutMode() == LayoutMode::SYSTEM)
            return;
      if (score()->pages().isEmpty())
            return;
      int dx = *dxx;
      int dy = *dyy;
      QRectF rect = QRectF(0, 0, width(), height());

      Page* firstPage = score()->pages().front();
      Page* lastPage  = score()->pages().back();

      if (firstPage && lastPage) {
            QPointF offsetPt(xoffset(), yoffset());
            QRectF firstPageRect(firstPage->pos().x() * physicalZoomLevel(),
                                      firstPage->pos().y() * physicalZoomLevel(),
                                      firstPage->width() * physicalZoomLevel(),
                                      firstPage->height() * physicalZoomLevel());
            QRectF lastPageRect(lastPage->pos().x() * physicalZoomLevel(),
                                         lastPage->pos().y() * physicalZoomLevel(),
                                         lastPage->width() * physicalZoomLevel(),
                                         lastPage->height() * physicalZoomLevel());
            QRectF pagesRect     = firstPageRect.united(lastPageRect).translated(offsetPt);
            bool limitScrollArea = preferences.getBool(PREF_UI_CANVAS_SCROLL_LIMITSCROLLAREA);
            if (!limitScrollArea) {
                  qreal hmargin = this->width() * 0.75;
                  qreal vmargin = this->height() * 0.75;
                  pagesRect.adjust(-hmargin, -vmargin, hmargin, vmargin);
                  }
            QRectF toPagesRect = pagesRect.translated(dx, dy);

            if (limitScrollArea) {
                  if (pagesRect.width() <= rect.width()) {
                        if (score()->layoutMode() == LayoutMode::LINE)
                              // keep score fixed in place horizontally
                              dx = 0;
                        else
                              // center horizontally on screen
                              dx = (rect.width() - pagesRect.width()) / 2 - pagesRect.left();
                        }
                  else if (toPagesRect.left() > rect.left())
                        // get rid of the left margin
                        dx = rect.left() - pagesRect.left();
                  else if (toPagesRect.right() < rect.right())
                        // get rid of the right margin
                        dx = rect.right() - pagesRect.right();
                  }
            else if (dx > 0) { // move right
                  if (toPagesRect.right() > rect.right() && toPagesRect.left() > rect.left()) {
                        if(pagesRect.width() <= rect.width()) {
                              dx = rect.right() - pagesRect.right();
                              }
                        else {
                              dx = rect.left() - pagesRect.left();
                              }
                        }
                  }
            else if (dx < 0) { // move left
                  if (toPagesRect.left() < rect.left() && toPagesRect.right() < rect.right()) {
                        if (pagesRect.width() <= rect.width()) {
                              dx = rect.left() - pagesRect.left();
                              }
                        else {
                              dx = rect.right() - pagesRect.right();
                              }
                        }
                  }

            if (limitScrollArea) {
                  if (pagesRect.height() <= rect.height()) {
                        if (score()->layoutMode() == LayoutMode::LINE)
                              // keep score fixed in place vertically
                              dy = 0;
                        else
                              // center vertically on screen
                              dy = (rect.height() - pagesRect.height()) / 2 - pagesRect.top();
                        }
                  else if (toPagesRect.top() > rect.top())
                        // get rid of the top margin
                        dy = rect.top() - pagesRect.top();
                  else if (toPagesRect.bottom() < rect.bottom())
                        // get rid of the bottom margin
                        dy = rect.bottom() - pagesRect.bottom();
                  }
            else if (dy > 0) { // move down
                  if (toPagesRect.bottom() > rect.bottom() && toPagesRect.top() > rect.top()) {
                        if (pagesRect.height() <= rect.height()) {
                              dy = rect.bottom() - pagesRect.bottom();
                              }
                        else {
                              dy = rect.top() - pagesRect.top();
                              }
                        }
                  }
            else if (dy < 0) { // move up
                  if (toPagesRect.top() < rect.top() && toPagesRect.bottom() < rect.bottom()) {
                        if (pagesRect.height() <= rect.height()) {
                              dy = rect.top() - pagesRect.top();
                              }
                        else {
                              dy = rect.bottom() - pagesRect.bottom();
                              }
                        }
                  }
            }
      *dxx = dx;
      *dyy = dy;
      }

//---------------------------------------------------------
//   setPhysicalZoomLevel
//---------------------------------------------------------

void ScoreView::setPhysicalZoomLevel(const qreal physicalLevel)
      {
      const qreal currentPhysicalLevel = _matrix.m11();

      if (physicalLevel == currentPhysicalLevel)
            return;

      const double deltaPhysicalLevel = physicalLevel / currentPhysicalLevel;

      _matrix.setMatrix(physicalLevel, _matrix.m12(), _matrix.m13(), _matrix.m21(),
            physicalLevel, _matrix.m23(), _matrix.dx() * deltaPhysicalLevel, _matrix.dy() * deltaPhysicalLevel, _matrix.m33());
      imatrix = _matrix.inverted();
      emit scaleChanged(physicalLevel * score()->spatium());
      if (editData.grips) {
            qreal w = 8.0 / physicalLevel;
            qreal h = 8.0 / physicalLevel;
            QRectF r(-w*.5, -h*.5, w, h);
            for (int i = 0; i < editData.grips; ++i) {
                  QPointF p(editData.grip[i].center());
                  editData.grip[i] = r.translated(p);
                  }
            }
      update();
      }

//---------------------------------------------------------
//   setLogicalZoom
//    Sets the zoom type and logical zoom level. pos is optional and may be omitted to zoom relative to the top-left corner.
//---------------------------------------------------------

void ScoreView::setLogicalZoom(ZoomIndex index, qreal logicalLevel, const QPointF& pos/* = QPointF()*/)
      {
      _zoomIndex = index;

      const qreal newLogicalLevel = qBound(ZOOM_LEVEL_MIN, logicalLevel, ZOOM_LEVEL_MAX);

      const qreal newPhysicalLevel = newLogicalLevel * mscore->physicalDotsPerInch() / DPI;

      const QPointF p1 = pos.isNull() ? pos : imatrix.map(pos);

      setPhysicalZoomLevel(newPhysicalLevel);

      int dx = 0;
      int dy = 0;

      if (!pos.isNull()) {
            const QPointF p2 = imatrix.map(pos);
            const QPointF p3 = p2 - p1;

            dx = lrint(p3.x() * newPhysicalLevel);
            dy = lrint(p3.y() * newPhysicalLevel);
            }

      constraintCanvas(&dx, &dy);
      if (dx != 0 || dy != 0) {
            _matrix.setMatrix(_matrix.m11(), _matrix.m12(), _matrix.m13(), _matrix.m21(),
               _matrix.m22(), _matrix.m23(), _matrix.dx()+dx, _matrix.dy()+dy, _matrix.m33());
            imatrix = _matrix.inverted();
            scroll(dx, dy, QRect(0, 0, width(), height()));
            emit offsetChanged(_matrix.dx(), _matrix.dy());
            }

      emit viewRectChanged();
      update();

      mscore->updateZoomBox(index, newLogicalLevel);
      }

//---------------------------------------------------------
//   calculateLogicalZoomLevel
//    Calculates the logical zoom level. logicalFreeZoomLevel is optional and may be omitted unless index is ZoomIndex::ZOOM_FREE.
//---------------------------------------------------------

qreal ScoreView::calculateLogicalZoomLevel(const ZoomIndex index, const qreal logicalFreeZoomLevel/* = 0.0*/) const
      {
      return calculatePhysicalZoomLevel(index, logicalFreeZoomLevel) / (mscore->physicalDotsPerInch() / DPI);
      }

//---------------------------------------------------------
//   calculatePhysicalZoomLevel
//    Calculates the physical zoom level. logicalFreeZoomLevel is optional and may be omitted unless index is ZoomIndex::ZOOM_FREE.
//---------------------------------------------------------

qreal ScoreView::calculatePhysicalZoomLevel(const ZoomIndex index, const qreal logicalFreeZoomLevel/* = 0.0*/) const
      {
      if (!_score)
            return 1.0;

      const qreal l2p = mscore->physicalDotsPerInch() / DPI;
      const qreal cw = width();
      const qreal ch = height();
      const qreal pw = _score->styleD(Sid::pageWidth);
      const qreal ph = _score->styleD(Sid::pageHeight);

      qreal result = 0.0;

      switch (index) {
            case ZoomIndex::ZOOM_PAGE_WIDTH:
                  result = cw / (pw * DPI);
                  break;

            case ZoomIndex::ZOOM_WHOLE_PAGE: {
                  const qreal mag1 = cw / (pw * DPI);
                  const qreal mag2 = ch / (ph * DPI);
                  result = std::min(mag1, mag2);
                  }
                  break;

            case ZoomIndex::ZOOM_TWO_PAGES: {
                  qreal mag1 = 0.0;
                  qreal mag2 = 0.0;
                  if (MScore::verticalOrientation()) {
                        mag1 = ch / (ph * 2.0 * DPI + MScore::verticalPageGap);
                        mag2 = cw / (pw * DPI);
                        }
                  else {
                        mag1 = cw / (pw * 2.0 * DPI + std::max(MScore::horizontalPageGapEven, MScore::horizontalPageGapOdd));
                        mag2 = ch / (ph * DPI);
                        }
                  result = std::min(mag1, mag2);
                  }
                  break;

            case ZoomIndex::ZOOM_FREE:
                  // If the zoom type is free zoom, the caller is required to pass the logical free-zoom level.
                  Q_ASSERT(!qFuzzyIsNull(logicalFreeZoomLevel));
                  result = logicalFreeZoomLevel * l2p;
                  break;

            default: {
                  // If the selected zoom entry is one of the numeric presets, set the physical zoom level accordingly; otherwise,
                  // initialize the physical zoom level to 0.0 so that it can be overridden below with the actual current value.
                  const auto i = std::find(zoomEntries.cbegin(), zoomEntries.cend(), index);
                  result = ((i != zoomEntries.cend()) && i->isNumericPreset()) ? (i->level / 100.0 * l2p) : 0.0;
                  }
                  break;
            }

      if (result < 0.0001)
            result = physicalZoomLevel();

      return result;
      }

//---------------------------------------------------------
//   setFocusRect
//---------------------------------------------------------

void ScoreView::setFocusRect()
      {
      if (mscore->splitScreen()) {
            if (!focusFrame) {
                  focusFrame = new QFocusFrame;
                  QPalette p(focusFrame->palette());
                  p.setColor(QPalette::WindowText, MScore::selectColor[0]);
                  focusFrame->setPalette(p);
                  }
            focusFrame->setWidget(static_cast<QWidget*>(this));
            focusFrame->show();
            }
      else {
            if (focusFrame)
                  focusFrame->setWidget(0);
            }
      }

//---------------------------------------------------------
//   editCopy
//---------------------------------------------------------

void ScoreView::editCopy()
      {
      if (editData.element)
            editData.element->editCopy(editData);
      }

//---------------------------------------------------------
//   editCut
//---------------------------------------------------------

void ScoreView::editCut()
      {
      _score->startCmd();
      if (editData.element)
            editData.element->editCut(editData);
      _score->endCmd();
      }

//---------------------------------------------------------
//   checkCopyOrCut
//---------------------------------------------------------

bool ScoreView::checkCopyOrCut()
      {
      if (!_score->selection().canCopy()) {
            QMessageBox::information(0, "MuseScore",
               tr("Please select the complete tuplet/tremolo and retry the command"),
               QMessageBox::Ok, QMessageBox::NoButton);
            return false;
            }
      return true;
      }

//---------------------------------------------------------
//   normalCopy
//---------------------------------------------------------

void ScoreView::normalCopy()
      {
      if (!checkCopyOrCut())
            return;
      QString mimeType = _score->selection().mimeType();
      if (!mimeType.isEmpty()) {
            QMimeData* mimeData = new QMimeData;
            mimeData->setData(mimeType, _score->selection().mimeData());
            if (MScore::debugMode)
                  qDebug("cmd copy: <%s>", mimeData->data(mimeType).data());
            QApplication::clipboard()->setMimeData(mimeData);
            }
      }

//---------------------------------------------------------
//   normalCut
//---------------------------------------------------------

void ScoreView::normalCut()
      {
      if (!checkCopyOrCut())
            return;
      _score->startCmd();
      normalCopy();
      _score->cmdDeleteSelection();
      _score->endCmd();
      }

//---------------------------------------------------------
//   editSwap
//---------------------------------------------------------

void ScoreView::editSwap()
      {
#if 0 // TODO
      if (editData.element && editData.element->isText() && !editData.element->isLyrics()) {
            Text* text = toText(editData.element);
            QString s = text->selectedText();
            text->paste(this);
            if (!s.isEmpty())
                  QApplication::clipboard()->setText(s);
            }
#endif
      }

//---------------------------------------------------------
//   editPaste
//---------------------------------------------------------

void ScoreView::editPaste()
      {
      if (textEditMode())
            toTextBase(editData.element)->paste(editData);
      }

//---------------------------------------------------------
//   normalSwap
//---------------------------------------------------------

void ScoreView::normalSwap()
      {
      if (!checkCopyOrCut())
            return;
      QString mimeType = _score->selection().mimeType();
      const QMimeData* ms = QApplication::clipboard()->mimeData();
      if (mimeType == mimeStaffListFormat) { // determine size of clipboard selection
            Fraction tickLen = Fraction(0,1);
            int staves = 0;
            QByteArray d(ms->data(mimeStaffListFormat));
            XmlReader e(d);
            e.readNextStartElement();
            if (e.name() == "StaffList") {
                  tickLen = Fraction::fromTicks(e.intAttribute("len", 0));
                  staves  = e.intAttribute("staves", 0);
                  }
            if (tickLen > Fraction(0,1)) { // attempt to extend selection to match clipboard size
                  Segment* seg = _score->selection().startSegment();
                  Fraction tick = _score->selection().tickStart() + tickLen;
                  Segment* segAfter = _score->tick2leftSegment(tick);
                  int staffIdx = _score->selection().staffStart() + staves - 1;
                  if (staffIdx >= _score->nstaves())
                        staffIdx = _score->nstaves() - 1;
                  tick = _score->selection().tickStart();
                  Fraction  etick = tick + tickLen;
                  if (MScore::debugMode)
                        _score->selection().dump();
                  _score->selection().extendRangeSelection(seg, segAfter, staffIdx, tick, etick);
                  _score->selection().update();
                  if (MScore::debugMode)
                        _score->selection().dump();
                  if (!checkCopyOrCut())
                        return;
                  ms = QApplication::clipboard()->mimeData();
                  }
            }
      QByteArray d(_score->selection().mimeData());
      if (this->normalPaste()) {
            QMimeData* mimeData = new QMimeData;
            mimeData->setData(mimeType, d);
            QApplication::clipboard()->setMimeData(mimeData);
            }
      }

//---------------------------------------------------------
//   normalPaste
//---------------------------------------------------------

bool ScoreView::normalPaste(Fraction scale)
      {
      _score->startCmd();
      const QMimeData* ms = QApplication::clipboard()->mimeData();
      _score->cmdPaste(ms, this, scale);
      bool rv = MScore::_error == MS_NO_ERROR;
      _score->endCmd();
      return rv;
      }

//---------------------------------------------------------
//   cmdGotoElement
//---------------------------------------------------------

void ScoreView::cmdGotoElement(Element* e)
      {
      if (e) {
            if (e->type() == ElementType::NOTE || e->type() == ElementType::HARMONY)
                  score()->setPlayNote(true);
            score()->select(e, SelectType::SINGLE, 0);
            if (e)
                  adjustCanvasPosition(e, false);
            if (noteEntryMode())
                  moveCursor();
            updateAll();
            }
      }

//---------------------------------------------------------
//   ticksTab
//---------------------------------------------------------

void ScoreView::ticksTab(const Fraction& ticks)
      {
      if (editData.element->isHarmony())
            harmonyTicksTab(ticks);
      else if (editData.element->isFiguredBass())
            figuredBassTicksTab(ticks);
      }

//---------------------------------------------------------
//   cmd
//---------------------------------------------------------

void ScoreView::cmd(const QAction* a)
      {
      const char* s = a ? a->data().toByteArray().constData() : "";
      cmd(s);
      updateEditElement();
      }

void ScoreView::cmd(const char* s)
      {
      struct ScoreViewCmd {
            std::vector<const char*> commands;
            std::function<void(ScoreView*, const QByteArray& cmd)> exec;
            };

      const QByteArray cmd(s);

      shadowNote->setVisible(false);
      if (MScore::debugMode)
            qDebug("ScoreView::cmd <%s>", s);

      static const std::vector<ScoreViewCmd> cmdList {
            {{"escape"}, [](ScoreView* cv, const QByteArray&) {
                  cv->escapeCmd();
                  }},
            {{"note-input"}, [](ScoreView* cv, const QByteArray&) {
                  if (cv->state == ViewState::NORMAL)
                        cv->changeState(ViewState::NOTE_ENTRY);
                  else if (cv->state == ViewState::NOTE_ENTRY)
                        cv->changeState(ViewState::NORMAL);
                  }},
            {{"copy"}, [](ScoreView* cv, const QByteArray&) {
                  if (cv->fotoMode())
                        cv->fotoModeCopy();
                  else if (cv->state == ViewState::NORMAL)
                        cv->normalCopy();
                  else if (cv->state == ViewState::EDIT)
                        cv->editCopy();
                  }},
            {{"cut"}, [](ScoreView* cv, const QByteArray&) {
                  if (cv->state == ViewState::NORMAL)
                        cv->normalCut();
                  else if (cv->state == ViewState::EDIT)
                        cv->editCut();
                  }},
            {{"paste"}, [](ScoreView* cv, const QByteArray&) {
                  if (cv->state == ViewState::NORMAL)
                        cv->normalPaste();
                  else if (cv->state == ViewState::EDIT)
                        cv->editPaste();
                  }},
            {{"paste-half"}, [](ScoreView* cv, const QByteArray&) {
                  cv->normalPaste(Fraction(1, 2));
                  }},
            {{"paste-double"}, [](ScoreView* cv, const QByteArray&) {
                  cv->normalPaste(Fraction(2, 1));
                  }},
            {{"paste-special"}, [](ScoreView* cv, const QByteArray&) {
                  Fraction scale = Fraction(1, 1);
                  Fraction duration = cv->score()->inputState().duration().fraction();
                  if (duration.isValid() && !duration.isZero()) {
                        scale = duration * 4;
                        scale.reduce();
                        }
                  cv->normalPaste(scale);
                  }},
            {{"swap"}, [](ScoreView* cv, const QByteArray&) {
                  if (cv->state == ViewState::NORMAL)
                        cv->normalSwap();
                  else if (cv->state == ViewState::EDIT)
                        cv->editSwap();
                  }},
            {{"lyrics"}, [](ScoreView* cv, const QByteArray&) {
                  cv->score()->startCmd();
                  Lyrics* lyrics = cv->score()->addLyrics();
                  cv->score()->endCmd();
                  if (lyrics) {
                        cv->startEditMode(lyrics);
                        return;
                        }
                  }},
            {{"figured-bass"}, [](ScoreView* cv, const QByteArray&) {
                  FiguredBass* fb = cv->score()->addFiguredBass();
                  if (fb) {
                        cv->startEditMode(fb);
                        return;
                        }
                  }},
            {{"mag"}, /*[](ScoreView* cv, const QByteArray&)*/ {
                  // ??
                  }},
            {{"play"}, [](ScoreView* cv, const QByteArray&) {
                  if (seq && seq->canStart()) {
                        if (cv->state == ViewState::NORMAL || cv->state == ViewState::NOTE_ENTRY)
                              cv->changeState(ViewState::PLAY);
                        else if (cv->state == ViewState::PLAY)
                              cv->changeState(ViewState::NORMAL);
                        }
                  else
                        getAction("play")->setChecked(false);
                  }},
            {{"fotomode"}, [](ScoreView* cv, const QByteArray&) {
                  if (cv->state == ViewState::NORMAL)
                        cv->changeState(ViewState::FOTO);
                  else if (cv->fotoMode())
                        cv->changeState(ViewState::NORMAL);
                  }},
            {{"add-slur"}, [](ScoreView* cv, const QByteArray&) {
                  cv->cmdAddSlur();
                  }},
            {{"add-hairpin"}, [](ScoreView* cv, const QByteArray&) {
                  cv->cmdAddHairpin(HairpinType::CRESC_HAIRPIN);
                  }},
            {{"add-hairpin-reverse"}, [](ScoreView* cv, const QByteArray&) {
                  cv->cmdAddHairpin(HairpinType::DECRESC_HAIRPIN);
                  }},
            {{"add-noteline"}, [](ScoreView* cv, const QByteArray&) {
                  cv->cmdAddNoteLine();
                  }},
            {{"chord-text"}, [](ScoreView* cv, const QByteArray&) {
                  cv->changeState(ViewState::NORMAL);
                  cv->cmdAddChordName(HarmonyType::STANDARD);
                  }},
            {{"roman-numeral-text"}, [](ScoreView* cv, const QByteArray&) {
                  cv->changeState(ViewState::NORMAL);
                  cv->cmdAddChordName(HarmonyType::ROMAN);
                  }},
            {{"nashville-number-text"}, [](ScoreView* cv, const QByteArray&) {
                  cv->changeState(ViewState::NORMAL);
                  cv->cmdAddChordName(HarmonyType::NASHVILLE);
                  }},
            {{"frame-text"}, [](ScoreView* cv, const QByteArray&) {
                  cv->cmdAddText(Tid::FRAME);
                  }},
            {{"title-text"}, [](ScoreView* cv, const QByteArray&) {
                  cv->cmdAddText(Tid::TITLE);
                  }},
            {{"subtitle-text"}, [](ScoreView* cv, const QByteArray&) {
                  cv->cmdAddText(Tid::SUBTITLE);
                  }},
            {{"composer-text"}, [](ScoreView* cv, const QByteArray&) {
                  cv->cmdAddText(Tid::COMPOSER);
                  }},
            {{"poet-text"}, [](ScoreView* cv, const QByteArray&) {
                  cv->cmdAddText(Tid::POET);
                  }},
            {{"part-text"}, [](ScoreView* cv, const QByteArray&) {
                  cv->cmdAddText(Tid::INSTRUMENT_EXCERPT);
                  }},
            {{"system-text"}, [](ScoreView* cv, const QByteArray&) {
                  cv->cmdAddText(Tid::SYSTEM);
                  }},
            {{"staff-text"}, [](ScoreView* cv, const QByteArray&) {
                  cv->cmdAddText(Tid::STAFF);
                  }},
            {{"expression-text"}, [](ScoreView* cv, const QByteArray&) {
                  cv->cmdAddText(Tid::EXPRESSION);
                  }},
            {{"rehearsalmark-text"}, [](ScoreView* cv, const QByteArray&) {
                  cv->cmdAddText(Tid::REHEARSAL_MARK);
                  }},
            {{"instrument-change-text"}, [](ScoreView* cv, const QByteArray&) {
                  cv->cmdAddText(Tid::INSTRUMENT_CHANGE);
                  }},
            {{"fingering-text"}, [](ScoreView* cv, const QByteArray&) {
                  cv->cmdAddText(Tid::FINGERING);
                  }},
            {{"sticking-text"}, [](ScoreView* cv, const QByteArray&) {
                  cv->cmdAddText(Tid::STICKING);
                  }},
            {{"edit-element"}, [](ScoreView* cv, const QByteArray&) {
                  Element* e = cv->score()->selection().element();
                  if (e && e->isEditable() && !cv->popupActive) {
                        cv->startEditMode(e);
                        }
                  }},
            {{"select-similar"}, [](ScoreView* cv, const QByteArray&) {
                  if (cv->score()->selection().isSingle()) {
                        Element* e = cv->score()->selection().element();
                        mscore->selectSimilar(e, false);
                        }
                  }},
            {{"select-similar-staff"}, [](ScoreView* cv, const QByteArray&) {
                  if (cv->score()->selection().isSingle()) {
                        Element* e = cv->score()->selection().element();
                        mscore->selectSimilar(e, true);
                        }
                  }},
            {{"select-dialog"}, [](ScoreView* cv, const QByteArray&) {
                  if (cv->score()->selection().isSingle()) {
                        Element* e = cv->score()->selection().element();
                        mscore->selectElementDialog(e);
                        }
                  }},
      //      {{"find"}, [](ScoreView* cv, const QByteArray&) {
      //            ; // TODO:state         sm->postEvent(new CommandEvent(cmd));
      //            }},
            {{"scr-prev"}, [](ScoreView* cv, const QByteArray&) {
                  cv->screenPrev();
                  }},
            {{"scr-next"}, [](ScoreView* cv, const QByteArray&) {
                  cv->screenNext();
                  }},
            {{"page-prev"}, [](ScoreView* cv, const QByteArray&) {
                  cv->pagePrev();
                  }},
            {{"page-next"}, [](ScoreView* cv, const QByteArray&) {
                  cv->pageNext();
                  }},
            {{"page-top"}, [](ScoreView* cv, const QByteArray&) {
                  cv->pageTop();
                  }},
            {{"page-end"}, [](ScoreView* cv, const QByteArray&) {
                  cv->pageEnd();
                  }},
            {{"select-next-chord",
              "select-prev-chord",
              "select-next-measure",
              "select-prev-measure",
              "select-begin-line",
              "select-end-line",
              "select-begin-score",
              "select-end-score",
              "select-staff-above",
              "select-staff-below"}, [](ScoreView* cv, const QByteArray& cmd) {
                  Element* el = cv->score()->selectMove(cmd);
                  if (el)
                        cv->adjustCanvasPosition(el, false);
                  cv->score()->setPlayChord(true);
                  cv->updateAll();
                  }},
            {{"next-chord",
              "prev-chord",
              "next-track",
              "prev-track",
              "next-measure",
              "prev-measure",
              "next-system",
              "prev-system",
              "next-frame",
              "prev-frame",
              "next-section",
              "prev-section",
              "empty-trailing-measure",
              "top-staff"}, [](ScoreView* cv, const QByteArray& cmd) {

                  if (cv->score()->selection().isLocked()) {
                        LOGW() << "unable exec cmd: " << cmd << ", selection locked, reason: " << cv->score()->selection().lockReason();
                        return;
                        }

                  Element* el = cv->score()->selection().element();
                  if (el && (el->isTextBase())) {
                        cv->score()->startCmd();
                        const PropertyFlags pf = PropertyFlags::UNSTYLED;
                        if (cmd == "prev-chord")
                              el->undoChangeProperty(Pid::OFFSET, el->offset() - QPointF (MScore::nudgeStep * el->spatium(), 0.0), pf);
                        else if (cmd == "next-chord")
                              el->undoChangeProperty(Pid::OFFSET, el->offset() + QPointF (MScore::nudgeStep * el->spatium(), 0.0), pf);
                        else if (cmd == "prev-measure")
                              el->undoChangeProperty(Pid::OFFSET, el->offset() - QPointF (MScore::nudgeStep10 * el->spatium(), 0.0), pf);
                        else if (cmd == "next-measure")
                              el->undoChangeProperty(Pid::OFFSET, el->offset() + QPointF (MScore::nudgeStep10 * el->spatium(), 0.0), pf);
                        cv->score()->endCmd();
                        }
                  else {
                        Element* ele = cv->score()->move(cmd);
                        if (cmd == "empty-trailing-measure")
                              cv->changeState(ViewState::NOTE_ENTRY);
                        if (ele)
                              cv->adjustCanvasPosition(ele, false);
                        cv->score()->setPlayChord(true);
                        cv->updateAll();
                        }
                  }},
            {{"pitch-up-diatonic"}, [](ScoreView* cv, const QByteArray&) {
                  cv->score()->upDown(true, UpDownMode::DIATONIC);
                  }},
            {{"pitch-down-diatonic"}, [](ScoreView* cv, const QByteArray&) {
                  cv->score()->upDown(false, UpDownMode::DIATONIC);
                  }},
            {{"move-up"}, [](ScoreView* cv, const QByteArray) {
                  std::list<Element*> el = cv->score()->selection().uniqueElements();
                  for (Element* e : el) {
                        ChordRest* cr = nullptr;
                        if (e->type() == ElementType::NOTE)
                              cr = static_cast<Note*>(e)->chord();
                        else if (e->type() == ElementType::REST)
                              cr = static_cast<Rest*>(e);
                        if (cr)
                              cv->score()->moveUp(cr);
                        }
                  }},
            {{"move-down"}, [](ScoreView* cv, const QByteArray&) {
                  std::list<Element*> el = cv->score()->selection().uniqueElements();
                  for (Element* e : el) {
                        ChordRest* cr = nullptr;
                        if (e->type() == ElementType::NOTE)
                              cr = static_cast<Note*>(e)->chord();
                        else if (e->type() == ElementType::REST)
                              cr = static_cast<Rest*>(e);
                        if (cr)
                              cv->score()->moveDown(cr);
                        }
                  }},
            {{"up-chord"}, [](ScoreView* cv, const QByteArray&) {
                  Element* el = cv->score()->selection().element();
                  Element* oel = el;
                  if (el && (el->isNote() || el->isRest()))
                        cv->cmdGotoElement(cv->score()->upAlt(el));
                  el = cv->score()->selection().element();
                  while (el && el->isRest() && toRest(el)->isGap()) {
                        if (cv->score()->upAlt(el) == el) {
                              cv->cmdGotoElement(oel);
                              break;
                              }
                        el = cv->score()->upAlt(el);
                        cv->cmdGotoElement(el);
                        }
                  }},
            {{"down-chord"}, [](ScoreView* cv, const QByteArray&) {
                  Element* el = cv->score()->selection().element();
                  Element* oel = el;
                  if (el && (el->isNote() || el->isRest()))
                        cv->cmdGotoElement(cv->score()->downAlt(el));
                  el = cv->score()->selection().element();
                  while (el && el->isRest() && toRest(el)->isGap()) {
                        if (cv->score()->downAlt(el) == el) {
                              cv->cmdGotoElement(oel);
                              break;
                              }
                        el = cv->score()->downAlt(el);
                        cv->cmdGotoElement(el);
                        }
                  }},
            {{"top-chord"}, [](ScoreView* cv, const QByteArray&) {
                  Element* el = cv->score()->selection().element();
                  if (el && el->type() == ElementType::NOTE)
                        cv->cmdGotoElement(cv->score()->upAltCtrl(static_cast<Note*>(el)));
                  }},
            {{"bottom-chord"}, [](ScoreView* cv, const QByteArray&) {
                  Element* el = cv->score()->selection().element();
                  if (el && el->type() == ElementType::NOTE)
                        cv->cmdGotoElement(cv->score()->downAltCtrl(static_cast<Note*>(el)));
                  }},
            {{"next-segment-element"}, [](ScoreView* cv, const QByteArray&) {
                  Element* el = cv->score()->selection().element();
                  if (!el && !cv->score()->selection().elements().isEmpty() )
                      el = cv->score()->selection().elements().first();

                  if (el)
                        cv->cmdGotoElement(el->nextSegmentElement());
                  else
                        cv->cmdGotoElement(cv->score()->firstElement());
                  }},
            {{"prev-segment-element"}, [](ScoreView* cv, const QByteArray&) {
                  Element* el = cv->score()->selection().element();
                  if (!el && !cv->score()->selection().elements().isEmpty())
                      el = cv->score()->selection().elements().last();

                  if (el)
                        cv->cmdGotoElement(el->prevSegmentElement());
                  else
                        cv->cmdGotoElement(cv->score()->lastElement());
                  }},
            {{"next-element"}, [](ScoreView* cv, const QByteArray&) {
                  if (cv->editMode()) {
                        cv->textTab(false);
                        return;
                        }
                  Element* el = cv->score()->selection().element();
                  if (!el && !cv->score()->selection().elements().isEmpty() )
                      el = cv->score()->selection().elements().first();
                  if (!el) {
                        ChordRest* cr = cv->score()->selection().currentCR();
                        if (cr) {
                              if (cr->isChord())
                                    el = toChord(cr)->downNote();
                              else if (cr->isRest())
                                    el = cr;
                              cv->score()->select(el);
                              }
                        }

                  if (el)
                        cv->cmdGotoElement(cv->score()->nextElement());
                  else
                        cv->cmdGotoElement(cv->score()->firstElement());
                  }},
            {{"prev-element"}, [](ScoreView* cv, const QByteArray&) {
                  if (cv->editMode()) {
                        cv->textTab(true);
                        return;
                        }
                  Element* el = cv->score()->selection().element();
                  if (!el && !cv->score()->selection().elements().isEmpty())
                      el = cv->score()->selection().elements().last();
                  if (!el) {
                        ChordRest* cr = cv->score()->selection().currentCR();
                        if (cr) {
                              if (cr->isChord())
                                    el = toChord(cr)->upNote();
                              else if (cr->isRest())
                                    el = cr;
                              cv->score()->select(el);
                              }
                        }

                  if (el)
                        cv->cmdGotoElement(cv->score()->prevElement());
                  else
                        cv->cmdGotoElement(cv->score()->lastElement());
                  }},
            {{"first-element"}, [](ScoreView* cv, const QByteArray&) {
                  cv->cmdGotoElement(cv->score()->firstElement(false));
                  }},
            {{"last-element"}, [](ScoreView* cv, const QByteArray&) {
                  cv->cmdGotoElement(cv->score()->lastElement(false));
                  }},
            {{"get-location"}, [](ScoreView* cv, const QByteArray&) {
                  // get current selection
                  Element* e = cv->score()->selection().element();
                  if (!e) {
                        // no current selection - restore lost selection
                        e = cv->score()->selection().currentCR();
                        if (e && e->isChord())
                              e = toChord(e)->upNote();
                        }
                  if (!e) {
                        // no current or last selection - fall back to first element
                        e = cv->score()->firstElement(false);
                        }
                  // TODO: find & read current key & time signatures
                  if (e) {
                        ScoreAccessibility::instance()->clearAccessibilityInfo();
                        cv->cmdGotoElement(e);
                        }
                  }},
            {{"playback-position"}, [](ScoreView* cv, const QByteArray&) {
                  ChordRest* cr { nullptr };
                  Element* el { nullptr };
                  int currentTrack = cv->score()->getSelectedElement() ?
                                     cv->score()->getSelectedElement()->track() : 0;
                  currentTrack = (currentTrack >= 0) ? currentTrack : 0;
                  auto considerAllTracks { true }; // True : Consider all tracks instead of only voice-1 of first instrument
                  auto next { false };             // False: Get current (rather than next) segment of playback cursor
                  auto cursorPosition = cv->score()->repeatList().tick2utick(cv->score()->playPos().ticks());
                  Fraction frac { Fraction::fromTicks(cursorPosition) };
                  Segment* seg = next ? cv->score()->tick2rightSegment(frac, false) : cv->score()->tick2leftSegment(frac, false);

                  if (seg) {
                        if (seg->isChordRestType()) {
                              if ((cr = seg->cr(currentTrack)))
                                    el = cr;
                              else {
                                    for (int t = 0; t < cv->score()->ntracks(); t++) {
                                          if ((cr = seg->cr(t))) {
                                                el = cr;
                                                break;
                                                }
                                          }
                                    }
                              }
                        else {
                              // Cycle through tracks if no CR* found
                              if (considerAllTracks) {
                                    for (int t = 0; t < cv->score()->ntracks(); t++) {
                                          if ((cr = seg->cr(t))) {
                                                el = cr;
                                                break;
                                                }
                                          }
                                    }
                              // If still nothing found, get next CR*
                              else el = seg->nextChordRest(0, false);
                              }
                        }
                  if (el) {
                        if (el->isChord())
                              el = toChord(el)->upNote();
                        cv->cmdGotoElement(el);
                        }
                  }},
            {{"rest", "rest-TAB"}, [](ScoreView* cv, const QByteArray&) {
                  cv->cmdEnterRest();
                  }},
            {{"rest-1"}, [](ScoreView* cv, const QByteArray&) {
                  cv->cmdEnterRest(TDuration(TDuration::DurationType::V_WHOLE));
                  }},
            {{"rest-2"}, [](ScoreView* cv, const QByteArray&) {
                  cv->cmdEnterRest(TDuration(TDuration::DurationType::V_HALF));
                  }},
            {{"rest-4"}, [](ScoreView* cv, const QByteArray&) {
                  cv->cmdEnterRest(TDuration(TDuration::DurationType::V_QUARTER));
                  }},
            {{"rest-8"}, [](ScoreView* cv, const QByteArray&) {
                  cv->cmdEnterRest(TDuration(TDuration::DurationType::V_EIGHTH));
                  }},
            {{"interval1",
              "interval2", "interval-2",
              "interval3", "interval-3",
              "interval4", "interval-4",
              "interval5", "interval-5",
              "interval6", "interval-6",
              "interval7", "interval-7",
              "interval8", "interval-8",
              "interval9", "interval-9"}, [](ScoreView* cv, const QByteArray& cmd) {
                  int n = cmd.mid(8).toInt();
                  std::vector<Note*> nl;
                  if (cv->score()->selection().isRange()) {
                        for (ChordRest* cr : cv->score()->getSelectedChordRests()) {
                              if (cr->isChord())
                                    nl.push_back(n > 0 ? toChord(cr)->upNote() : toChord(cr)->downNote());
                              }
                        }
                  else {
                        nl = cv->score()->selection().noteList();
                        }
                  if (!nl.empty()) {
                        //if (!noteEntryMode())
                        //      ;     // TODO: state    sm->postEvent(new CommandEvent("note-input"));
                        cv->score()->cmdAddInterval(n, nl);
                        }
                  }},
            {{"tie"}, [](ScoreView* cv, const QByteArray&) {
                  if (cv->noteEntryMode()) {
                        cv->score()->cmdAddTie();
                        cv->moveCursor();
                        }
                  else
                        cv->score()->cmdToggleTie();
                  }},
            {{"chord-tie"}, [](ScoreView* cv, const QByteArray&) {
                  cv->score()->cmdAddTie(true);
                  cv->moveCursor();
                  }},
            {{"duplet"}, [](ScoreView* cv, const QByteArray&) {
                  cv->cmdTuplet(2);
                  }},
            {{"triplet"}, [](ScoreView* cv, const QByteArray&) {
                  cv->cmdTuplet(3);
                  }},
            {{"quadruplet"}, [](ScoreView* cv, const QByteArray&) {
                  cv->cmdTuplet(4);
                  }},
            {{"quintuplet"}, [](ScoreView* cv, const QByteArray&) {
                  cv->cmdTuplet(5);
                  }},
            {{"sextuplet"}, [](ScoreView* cv, const QByteArray&) {
                  cv->cmdTuplet(6);
                  }},
            {{"septuplet"}, [](ScoreView* cv, const QByteArray&) {
                  cv->cmdTuplet(7);
                  }},
            {{"octuplet"}, [](ScoreView* cv, const QByteArray&) {
                  cv->cmdTuplet(8);
                  }},
            {{"nonuplet"}, [](ScoreView* cv, const QByteArray&) {
                  cv->cmdTuplet(9);
                  }},
            {{"tuplet-dialog"}, [](ScoreView* cv, const QByteArray&) {
                  cv->score()->startCmd();
                  Tuplet* tuplet = mscore->tupletDialog();
                  if (tuplet)
                        cv->cmdCreateTuplet(cv->score()->getSelectedChordRest(), tuplet);
                  cv->score()->endCmd();
                  cv->moveCursor();
                  }},
            {{"repeat-sel"}, [](ScoreView* cv, const QByteArray&) {
                  cv->cmdRepeatSelection();
                  }},
            {{"voice-1"}, [](ScoreView* cv, const QByteArray&) {
                  cv->changeVoice(0);
                  }},
            {{"voice-2"}, [](ScoreView* cv, const QByteArray&) {
                  cv->changeVoice(1);
                  }},
            {{"voice-3"}, [](ScoreView* cv, const QByteArray) {
                  cv->changeVoice(2);
                  }},
            {{"voice-4"}, [](ScoreView* cv, const QByteArray&) {
                  cv->changeVoice(3);
                  }},
            {{"enh-both"}, [](ScoreView* cv, const QByteArray&) {
                  cv->cmdChangeEnharmonic(true);
                  }},
            {{"enh-current"}, [](ScoreView* cv, const QByteArray&) {
                  cv->cmdChangeEnharmonic(false);
                  }},
            {{"revision"}, [](ScoreView* cv, const QByteArray&) {
                  Score* sc = cv->score()->masterScore();
                  sc->createRevision();
                  }},
            {{"append-measure"}, [](ScoreView* cv, const QByteArray&) {
                  cv->cmdAppendMeasures(1, ElementType::MEASURE);
                  }},
            {{"insert-measure"}, [](ScoreView* cv, const QByteArray&) {
                cv->cmdInsertMeasures(1, ElementType::MEASURE);
                }},
            {{"insert-hbox"}, [](ScoreView* cv, const QByteArray&) {
                cv->cmdInsertMeasures(1, ElementType::HBOX);
                }},
            {{"insert-vbox"}, [](ScoreView* cv, const QByteArray&) {
                cv->cmdInsertMeasures(1, ElementType::VBOX);
                }},
            {{"append-hbox"}, [](ScoreView* cv, const QByteArray&) {
                MeasureBase* mb = cv->appendMeasure(ElementType::HBOX);
                  cv->score()->select(mb, SelectType::SINGLE, 0);
                  }},
            {{"append-vbox"}, [](ScoreView* cv, const QByteArray&) {
                MeasureBase* mb = cv->appendMeasure(ElementType::VBOX);
                  cv->score()->select(mb, SelectType::SINGLE, 0);
                  }},
            {{"insert-textframe"}, [](ScoreView* cv, const QByteArray&) {
                  cv->cmdInsertMeasure(ElementType::TBOX);
                  }},
            {{"append-textframe"}, [](ScoreView* cv, const QByteArray&) {
                  MeasureBase* mb = cv->appendMeasure(ElementType::TBOX);
                  if (mb) {
                        TBox* tf = static_cast<TBox*>(mb);
                        Text* text = 0;
                        foreach(Element* e, tf->el()) {
                              if (e->type() == ElementType::TEXT) {
                                    text = static_cast<Text*>(e);
                                    break;
                                    }
                              }
                        if (text) {
                              cv->score()->select(text, SelectType::SINGLE, 0);
                              cv->startEditMode(text);
                              }
                        }
                  }},
            {{"insert-fretframe"}, [](ScoreView* cv, const QByteArray&) {
                  if (!enableExperimental)
                        return;
                  cv->cmdInsertMeasure(ElementType::FBOX);
                  }},
            {{"move-left"}, [](ScoreView* cv, const QByteArray&) {
                  cv->cmdMoveCR(true);
                  }},
            {{"move-right"}, [](ScoreView* cv, const QByteArray&) {
                  cv->cmdMoveCR(false);
                  }},
            {{"reset"}, [](ScoreView* cv, const QByteArray&) {
                  if (cv->editMode()) {
                        cv->editData.element->reset();
                        cv->score()->update();
                        }
                  else {
                        cv->score()->startCmd();
                        for (Element* e : cv->score()->selection().elements()) {
                              e->reset();
                              if (e->isSpanner()) {
                                    Spanner* sp = toSpanner(e);
                                    for (SpannerSegment* ss : sp->spannerSegments())
                                          ss->reset();
                                    }
                              }
                        cv->score()->endCmd();
                        }
                  cv->updateGrips();
                  }},
      #ifdef OMR
            {{"show-omr"}, [](ScoreView* cv, const QByteArray&) {
                  if (cv->score()->masterScore()->omr())
                        cv->showOmr(!cv->score()->masterScore()->showOmr());
                  }},
      #endif
            {{"split-measure"}, [](ScoreView* cv, const QByteArray&) {
                  Element* e = cv->score()->selection().element();
                  if (!(e && (e->isNote() || e->isRest())))
                        MScore::setError(NO_CHORD_REST_SELECTED);
                  else {
                        if (e->isNote())
                              e = toNote(e)->chord();
                        ChordRest* cr = toChordRest(e);
                        cv->score()->cmdSplitMeasure(cr);
                        }
                  }},
            {{"join-measures"}, [](ScoreView* cv, const QByteArray&) {
                  Measure* m1;
                  Measure* m2;
                  if (!cv->score()->selection().measureRange(&m1, &m2) || m1 == m2) {
                        QMessageBox::warning(0, "MuseScore",
                           tr("No measures selected:\n"
                           "Please select a range of measures to join and try again"));
                        }
                  else {
                        cv->score()->cmdJoinMeasure(m1, m2);
                        }
                  }},
            {{"next-lyric", "prev-lyric"}, [](ScoreView* cv, const QByteArray& cmd) {
                  cv->editCmd(cmd);
                  }},
            {{"add-remove-breaks"}, [](ScoreView* cv, const QByteArray&) {
                  cv->cmdAddRemoveBreaks();
                  }},
            {{"copy-lyrics-to-clipboard"}, [](ScoreView* cv, const QByteArray&) {
                  cv->cmdCopyLyricsToClipboard();
                  }},

            // STATE_NOTE_ENTRY_REALTIME actions (auto or manual)

            {{"realtime-advance"}, [](ScoreView* cv, const QByteArray&) {
                  cv->realtimeAdvance(true);
                  }},

            // STATE_HARMONY_FIGBASS_EDIT actions

            {{"advance-longa"}, [](ScoreView* cv, const QByteArray&) {
                  cv->ticksTab(Fraction(4,1));
                  }},
            {{"advance-breve"}, [](ScoreView* cv, const QByteArray&) {
                  cv->ticksTab(Fraction(2,1));
                  }},
            {{"advance-1"}, [](ScoreView* cv, const QByteArray&) {
                  cv->ticksTab(Fraction(1,1));
                  }},
            {{"advance-2"}, [](ScoreView* cv, const QByteArray&) {
                  cv->ticksTab(Fraction(1,2));
                  }},
            {{"advance-4"}, [](ScoreView* cv, const QByteArray&) {
                  cv->ticksTab(Fraction(1,4));
                  }},
            {{"advance-8"}, [](ScoreView* cv, const QByteArray&) {
                  cv->ticksTab(Fraction(1,8));
                  }},
            {{"advance-16"}, [](ScoreView* cv, const QByteArray&) {
                  cv->ticksTab(Fraction(1,16));
                  }},
            {{"advance-32"}, [](ScoreView* cv, const QByteArray&) {
                  cv->ticksTab(Fraction(1,32));
                  }},
            {{"advance-64"}, [](ScoreView* cv, const QByteArray&) {
                  cv->ticksTab(Fraction(1,64));
                  }},
            {{"prev-measure-TEXT"}, [](ScoreView* cv, const QByteArray&) {
                  if (cv->editData.element->isHarmony())
                        cv->harmonyTab(true);
                  else if (cv->editData.element->isFiguredBass())
                        cv->figuredBassTab(true, true);
                  }},
            {{"next-measure-TEXT"}, [](ScoreView* cv, const QByteArray&) {
                  if (cv->editData.element->isHarmony())
                        cv->harmonyTab(false);
                  else if (cv->editData.element->isFiguredBass())
                        cv->figuredBassTab(true, false);
                  }},
            {{"prev-beat-TEXT"}, [](ScoreView* cv, const QByteArray&) {
                  if (cv->editData.element->isHarmony())
                        cv->harmonyBeatsTab(false, true);
                  }},
            {{"next-beat-TEXT"}, [](ScoreView* cv, const QByteArray&) {
                  if (cv->editData.element->isHarmony())
                        cv->harmonyBeatsTab(false, false);
                  }},

            // STATE_NOTE_ENTRY_TAB actions

            // move input state string up or down, within the number of strings of the instrument;
            // this may move the input state cursor outside of the tab line range to accommodate
            // instrument strings not represented in the tab (e.g.: lute bass strings):
            // the appropriate visual rendition of the input cursor in those cases will be managed by moveCursor()
            {{"string-above",
              "string-below"}, [](ScoreView* cv, const QByteArray& cmd) {
                  InputState& is          = cv->score()->inputState();
                  Staff*      staff       = cv->score()->staff(is.track() / VOICES);
                  int         instrStrgs  = staff->part()->instrument(is.tick())->stringData()->strings();
                  // assume "string-below": if tab is upside-down, 'below' means toward instrument top (-1)
                  // if not, 'below' means toward instrument bottom (+1)
                  int         delta       = (staff->staffType(is.tick())->upsideDown() ? -1 : +1);
                  if (cmd == "string-above")                      // if "above", reverse delta
                        delta = -delta;
                  int         strg        = is.string() + delta;  // dest. physical string
                  if (strg >= 0 && strg < instrStrgs) {            // if dest. string within instrument limits
                        is.setString(strg);                       // update status
                        cv->moveCursor();
                        }
                  }},
            {{"text-word-left"}, [](ScoreView* cv, const QByteArray&) {
                  toTextBase(cv->editData.element)->movePosition(cv->editData, QTextCursor::WordLeft);
                  }},
            {{"text-word-right"}, [](ScoreView* cv, const QByteArray&) {
                  toTextBase(cv->editData.element)->movePosition(cv->editData, QTextCursor::NextWord);
                  }},
            {{"concert-pitch"}, [](ScoreView* cv, const QByteArray& cmd) {
                  QAction* a = getAction(cmd);
                  if (cv->score()->styleB(Sid::concertPitch) != a->isChecked()) {
                        cv->score()->startCmd();
                        cv->score()->cmdConcertPitchChanged(a->isChecked(), true);
                        cv->score()->endCmd();
                        }
                  }},
            };

      auto c = std::find_if(cmdList.begin(), cmdList.end(), [cmd](const ScoreViewCmd& cc) {
            for (auto& name : cc.commands) {
                  if (cmd == name)
                        return true;
                  }
            return false;
            });

      if (c != cmdList.end()) {
            c->exec(this, cmd);
            }
      else {
            editData.view = this;
            QAction* a = getAction(cmd);
            _score->cmd(a, editData);
            }
      if (_score->processMidiInput())
            mscore->endCmd();
      }

//---------------------------------------------------------
//   textTab
//---------------------------------------------------------

void ScoreView::textTab(bool back)
      {
      if (!editMode())
            return;
      Element* oe = editData.element;
      if (!oe || !oe->isTextBase())
            return;

      if (oe->isHarmony()) {
            harmonyBeatsTab(true, back);
            return;
            }
      else if (oe->isFiguredBass()) {
            figuredBassTab(false, back);
            return;
            }
      else if (oe->isLyrics()) {
            // not actually hit, left/right handled elsewhere
            lyricsTab(back, false, true);
            return;
            }

      Element* op = oe->parent();
      if (!(op->isSegment() || op->isNote()))
            return;

      TextBase* ot = toTextBase(oe);
      Tid defaultTid = Tid(ot->propertyDefault(Pid::SUB_STYLE).toInt());
      Tid tid = ot->tid();
      ElementType type = ot->type();
      int staffIdx = ot->staffIdx();

      // get prev/next element now, as current element may be deleted if empty
      Element* el = back ? score()->prevElement() : score()->nextElement();
      // end edit mode
      changeState(ViewState::NORMAL);

      // find new note to add text to
      bool here = false;      // prevent infinite loop (relevant if navigation is allowed to wrap around end of score)
      while (el) {
            if (el->isNote()) {
                  Note* n = toNote(el);
                  if (op->isNote() && n != op)
                        break;
                  else if (op->isSegment() && n->chord()->segment() != op)
                        break;
                  else if (here)
                        break;
                  here = true;
                  }
            else if (el->isRest() && op->isSegment()) {
                  // skip rests, but still check for infinite loop
                  Rest* r = toRest(el);
                  if (r->segment() != op)
                        ;
                  else if (here)
                        break;
                  here = true;
                  }
            // get prev/next note
            score()->select(el);
            Element* el2 = back ? score()->prevElement() : score()->nextElement();
            // start/end of score reached
            if (el2 == el)
                  break;
            el = el2;
            }
      if (!el || !el->isNote()) {
            // nothing found, exit cleanly
            if (op->selectable())
                  score()->select(op);
            else
                  score()->deselectAll();
            return;
            }
      Note* nn = toNote(el);

      // go to note
      cmdGotoElement(nn);

      // get existing text to edit
      el = nullptr;
      if (op->isNote()) {
            // check element list of new note
            for (Element* e : nn->el()) {
                  if (e->type() != type)
                        continue;
                  TextBase* nt = toTextBase(e);
                  if (nt->tid() == tid) {
                        el = e;
                        break;
                        }
                  }
            }
      else if (op->isSegment()) {
            // check annotation list of new segment
            Segment* ns = nn->chord()->segment();
            for (Element* e : ns->annotations()) {
                  if (e->staffIdx() != staffIdx || e->type() != type)
                        continue;
                  TextBase* nt = toTextBase(e);
                  if (nt->tid() == tid) {
                        el = e;
                        break;
                        }
                  }
            }

      if (el) {
            // edit existing text
            score()->select(el);
            startEditMode(el);
            }
      else {
            // add new text if no existing element to edit
            // TODO: for tempo text, mscore->addTempo() could be called
            // but it pre-fills the text
            // would be better to create empty tempo element
            if (type != ElementType::TEMPO_TEXT) {
                  PropertyFlags pf = oe ? oe->propertyFlags(Pid::PLACEMENT) : PropertyFlags::STYLED;
                  Placement oePlacement = oe ? oe->placement() : Placement::ABOVE;
                  cmdAddText(defaultTid, tid, pf, oePlacement);
                  }
            }
      }

//---------------------------------------------------------
//   editKeySticking
//---------------------------------------------------------

bool ScoreView::editKeySticking()
      {
      Q_ASSERT(editData.element->isSticking());

      switch (editData.key) {
            case Qt::Key_Space:
                  textTab(editData.modifiers & Qt::ShiftModifier);
                  return true;
            case Qt::Key_Left:
                  textTab(true);
                  return true;
            case Qt::Key_Right:
            case Qt::Key_Return:
                  textTab(false);
                  return true;
            default:
                  break;
            }

      return false;
      }

//---------------------------------------------------------
//   showOmr
//---------------------------------------------------------

void ScoreView::showOmr(bool flag)
      {
      _score->masterScore()->setShowOmr(flag);
      ScoreTab* t = mscore->getTab1();
      if (t->view() != this)
            t = mscore->getTab2();
      if (t->view() == this)
            t->setCurrentIndex(t->currentIndex());
      else
            qDebug("view not found");
      }

//---------------------------------------------------------
//   startNoteEntry
//---------------------------------------------------------

void ScoreView::startNoteEntry()
      {
      InputState& is = _score->inputState();

      is.setSegment(0);
      Note* note  = 0;

      if (_score->selection().isNone()) {
            // no selection
            // choose page in current view (favor top left quadrant if possible)
            // select first (top/left) chordrest of that page in current view
            // or, CR at last selected position if that is in view
            Page* p = nullptr;
            QList<QPointF> points;
            points.append(toLogical(QPoint(width() * 0.25, height() * 0.25)));
            points.append(toLogical(QPoint(0.0, 0.0)));
            points.append(toLogical(QPoint(0.0, height())));
            points.append(toLogical(QPoint(width(), 0.0)));
            points.append(toLogical(QPoint(width(), height())));
            int i = 0;
            while (!p && i < points.size()) {
                  p = point2page(points[i]);
                  i++;
                  }
            if (p) {
                  ChordRest* topLeft = nullptr;
                  qreal tlY = 0.0;
                  Fraction tlTick = Fraction(0,1);
                  QRectF viewRect  = toLogical(QRectF(0.0, 0.0, width(), height()));
                  QRectF pageRect  = p->bbox().translated(p->x(), p->y());
                  QRectF intersect = viewRect & pageRect;
                  intersect.translate(-p->x(), -p->y());
                  QList<Element*> el = p->items(intersect);
                  ChordRest* lastSelected = score()->selection().currentCR();
                  if (lastSelected && lastSelected->voice()) {
                        // if last selected CR was not in voice 1,
                        // find CR in voice 1 instead
                        int track = trackZeroVoice(lastSelected->track());
                        Segment* s = lastSelected->segment();
                        if (s)
                              lastSelected = s->nextChordRest(track, true);
                        }
                  for (Element* e : el) {
                        // loop through visible elements
                        // looking for the CR in voice 1 with earliest tick and highest staff position
                        // but stop if we find the last selected CR
                        ElementType et = e->type();
                        if (et == ElementType::NOTE || et == ElementType::REST) {
                              if (e->voice())
                                    continue;
                              ChordRest* cr;
                              if (et == ElementType::NOTE) {
                                    cr = static_cast<ChordRest*>(e->parent());
                                    if (!cr)
                                          continue;
                                    }
                              else {
                                    cr = static_cast<ChordRest*>(e);
                                    }
                              if (cr == lastSelected) {
                                    topLeft = cr;
                                    break;
                                    }
                              // compare ticks rather than x position
                              // to make sure we favor earlier rather than later systems
                              // even though later system might have note farther to left
                              Fraction crTick = Fraction(0,1);
                              if (cr->segment())
                                    crTick = cr->segment()->tick();
                              else
                                    continue;
                              // compare staff Y position rather than note Y position
                              // to be sure we do not reject earliest note
                              // just because it is lower in pitch than subsequent notes
                              qreal crY = 0.0;
                              if (cr->measure() && cr->measure()->system())
                                    crY = cr->measure()->system()->staffYpage(cr->staffIdx());
                              else
                                    continue;
                              if (topLeft) {
                                    if (crTick <= tlTick && crY <= tlY) {
                                          topLeft = cr;
                                          tlTick = crTick;
                                          tlY = crY;
                                          }
                                    }
                              else {
                                    topLeft = cr;
                                    tlTick = crTick;
                                    tlY = crY;
                                    }
                              }
                        }
                  if (topLeft)
                        _score->select(topLeft, SelectType::SINGLE);
                  }
            }

      Element* el = _score->selection().element();
      if (!el)
            el = _score->selection().firstChordRest();
      if (el == 0 || (el->type() != ElementType::CHORD && el->type() != ElementType::REST && el->type() != ElementType::NOTE)) {
            // if no note/rest is selected, start with voice 0
            int track = is.track() == -1 ? 0 : (is.track() / VOICES) * VOICES;
            // try to find an appropriate measure to start in
            Fraction tick = el ? el->tick() : Fraction(0,1);
            el = _score->searchNote(tick, track);
            if (!el)
                  el = _score->searchNote(Fraction(0,1), track);
            }
      if (!el)
            return;
      if (el->type() == ElementType::CHORD) {
            Chord* c = static_cast<Chord*>(el);
            note = c->selectedNote();
            if (note == 0)
                  note = c->upNote();
            el = note;
            }
      TDuration d(is.duration());
      if (!d.isValid() || d.isZero() || d.type() == TDuration::DurationType::V_MEASURE)
            is.setDuration(TDuration(TDuration::DurationType::V_QUARTER));
      is.setAccidentalType(AccidentalType::NONE);

      _score->select(el, SelectType::SINGLE, 0);
      is.setRest(false);
      is.setNoteEntryMode(true);
      adjustCanvasPosition(el, false);
      setEditElement(el);

      getAction("pad-rest")->setChecked(false);
      setMouseTracking(true);
      if (!MScore::disableMouseEntry)
            shadowNote->setVisible(true);
      _score->setUpdateAll();
      _score->update();

      Staff* staff = _score->staff(is.track() / VOICES);

      // if not tab, pitched/unpitched depends on instrument (override StaffGroup to allow pitched/unpitched changes)
      StaffGroup staffGroup = staff->staffType(is.tick())->group();
      if (staffGroup != StaffGroup::TAB)
            staffGroup = staff->part()->instrument(is.tick())->useDrumset() ? StaffGroup::PERCUSSION : StaffGroup::STANDARD;

      switch (staffGroup) {
            case StaffGroup::STANDARD:
                  break;
            case StaffGroup::TAB: {
                  int strg = 0;                 // assume topmost string as current string
                  // if entering note entry with a note selected and the note has a string
                  // set InputState::_string to note physical string
                  if (el->type() == ElementType::NOTE) {
                        strg = (static_cast<Note*>(el))->string();
                        }
                  is.setString(strg);
                  break;
                  }
            case StaffGroup::PERCUSSION:
                  break;
            }
      // set cursor after setting the stafftype-dependent state
      moveCursor();
      mscore->updateInputState(_score);
      shadowNote->setVisible(false);
      setCursorOn(true);
      }

//---------------------------------------------------------
//   endNoteEntry
//---------------------------------------------------------

void ScoreView::endNoteEntry()
      {
      InputState& is = _score->inputState();
      is.setNoteEntryMode(false);
      if (is.slur()) {
            const std::vector<SpannerSegment*>& el = is.slur()->spannerSegments();
            if (!el.empty())
                  el.front()->setSelected(false);
            is.setSlur(0);
            }
      setMouseTracking(false);
      shadowNote->setVisible(false);
      setCursorOn(false);
      _score->setUpdateAll();
      _score->update();
      }

//---------------------------------------------------------
//   dragScoreView
//---------------------------------------------------------

void ScoreView::dragScoreView(QMouseEvent* ev)
      {
      QPoint d = ev->pos() - _matrix.map(editData.startMove).toPoint();
      int dx   = d.x();
      int dy   = d.y();

      if (dx == 0 && dy == 0)
            return;

      constraintCanvas(&dx, &dy);

      TourHandler::startTour("navigate-tour");

      _matrix.setMatrix(_matrix.m11(), _matrix.m12(), _matrix.m13(), _matrix.m21(),
         _matrix.m22(), _matrix.m23(), _matrix.dx()+dx, _matrix.dy()+dy, _matrix.m33());
      imatrix = _matrix.inverted();
      scroll(dx, dy, QRect(0, 0, width(), height()));
      // scroll schedules an update which is probably too small
      // hack around:
      if (dx > 0)
            update(-10, 0, dx + 50, height());
      else if (dx < 0)
            update(width() - 50 + dx, 0, width() + 10, height());
      emit offsetChanged(_matrix.dx(), _matrix.dy());
      emit viewRectChanged();
      }

//---------------------------------------------------------
//   doDragLasso
//---------------------------------------------------------

void ScoreView::doDragLasso(QMouseEvent* ev)
      {
      TourHandler::startTour("select-tour");
      QPointF p = toLogical(ev->pos());
      _score->addRefresh(lasso->canvasBoundingRect());
      QRectF r;
      r.setCoords(editData.startMove.x(), editData.startMove.y(), p.x(), p.y());
      lasso->setbbox(r.normalized());
      QRectF _lassoRect(lasso->bbox());
      r = _matrix.mapRect(_lassoRect);
      QSize sz(r.size().toSize());
      mscore->statusBar()->showMessage(QString("%1 x %2").arg(sz.width()).arg(sz.height()), 3000);
      _score->addRefresh(lasso->canvasBoundingRect());
      _score->lassoSelect(lasso->bbox());
      _score->update();
      }

//---------------------------------------------------------
//   endLasso
//---------------------------------------------------------

void ScoreView::endLasso()
      {
      _score->addRefresh(lasso->canvasBoundingRect());
      lasso->setbbox(QRectF());
      _score->lassoSelectEnd();
      _score->update();
      mscore->endCmd();
      }

//---------------------------------------------------------
//   deselectAll
//---------------------------------------------------------

void ScoreView::deselectAll()
      {
      _score->deselectAll();
      _score->update();
      mscore->endCmd();
      }

//---------------------------------------------------------
//   inputMethodQuery
//---------------------------------------------------------

QVariant ScoreView::inputMethodQuery(Qt::InputMethodQuery query) const
      {
//      qDebug("0x%x  %s", int(query), editData.element ? editData.element->name() : "-no element-");
      if (editData.element && editData.element->isTextBase()) {
            TextBase* text = toTextBase(editData.element);
            switch (query) {
                  case Qt::ImCursorRectangle: {
                        QRectF r;
                        if (editMode()) {
                              TextCursor* cursor = text->cursor(editData);
                              r = toPhysical(cursor->cursorRect().translated(text->canvasPos()));
                              r.setWidth(1); // InputMethod doesn't display properly if width left at 0
                              }
                        else
                              r = toPhysical(text->canvasBoundingRect());
                        r.setHeight(r.height() + 10); // add a little margin under the cursor
                        qDebug("ScoreView::inputMethodQuery() updating cursorRect to: (%3f, %3f) + (%3f, %3f)", r.x(), r.y(), r.width(), r.height());
                        return QVariant(r);
                        }
                  case Qt::ImEnabled:
                        return true; // TextBase will always accept input method input
                  case Qt::ImHints:
                        return Qt::ImhNone; // No hints for now, but maybe in future will give hints
                  case Qt::ImInputItemClipRectangle: // maybe give clip rect hint in future, but for now use default parent clipping rect
                  default:
                        return QWidget::inputMethodQuery(query); // fall back to QWidget's version as default
                  }
            }
      QVariant d = QWidget::inputMethodQuery(query); // fall back to QWidget's version as default
//      qDebug() << "   " << d;
      return d;
      }

//---------------------------------------------------------
//   logicalZoomLevel
//---------------------------------------------------------

qreal ScoreView::logicalZoomLevel() const
      {
      return _matrix.m11() / (mscore->physicalDotsPerInch() / DPI);
      }

//---------------------------------------------------------
//   physicalZoomLevel
//---------------------------------------------------------

qreal ScoreView::physicalZoomLevel() const
      {
      return _matrix.m11();
      }

//---------------------------------------------------------
//   setOffset
//---------------------------------------------------------

void ScoreView::setOffset(qreal x, qreal y)
      {
      _matrix.setMatrix(_matrix.m11(), _matrix.m12(), _matrix.m13(), _matrix.m21(),
         _matrix.m22(), _matrix.m23(), x, y, _matrix.m33());
      imatrix = _matrix.inverted();
      emit viewRectChanged();
      emit offsetChanged(x, y);
      }

//---------------------------------------------------------
//   xoffset
//---------------------------------------------------------

qreal ScoreView::xoffset() const
      {
      return _matrix.dx();
      }

//---------------------------------------------------------
//   yoffset
//---------------------------------------------------------

qreal ScoreView::yoffset() const
      {
      return _matrix.dy();
      }

//---------------------------------------------------------
//   fsize
//---------------------------------------------------------

QSizeF ScoreView::fsize() const
      {
      QSize s = size();
      return QSizeF(s.width() * imatrix.m11(), s.height() * imatrix.m22());
      }

//---------------------------------------------------------
//   keyboard nav constants
//---------------------------------------------------------

constexpr qreal scrollStep   {   .8 };
constexpr qreal thinPadding  { 10.0 };
constexpr qreal thickPadding { 25.0 };

//---------------------------------------------------------
//   pageNext
//---------------------------------------------------------

void ScoreView::pageNext()
      {
      if (score()->pages().empty())
            return;
      if (score()->layoutMode() != LayoutMode::PAGE) {
            screenNext();
            return;
            }
      Page* page = score()->pages().back();
      qreal x, y;
      if (MScore::verticalOrientation()) {
            x        = thinPadding;
            y        = yoffset() - (page->height() + thickPadding) * physicalZoomLevel();
            qreal ly = thinPadding - page->pos().y() * physicalZoomLevel();
            if (y <= ly - height() * scrollStep) {
                  pageEnd();
                  return;
                  }
            }
      else {
            y        = thinPadding;
            x        = xoffset() - (page->width() + thickPadding) * physicalZoomLevel();
            qreal lx = thinPadding - page->pos().x() * physicalZoomLevel();
            if (x <= lx - width() * scrollStep) {
                  pageEnd();
                  return;
                  }
            }
      setOffset(x, y);
      update();
      }

//---------------------------------------------------------
//   screenNext
//---------------------------------------------------------

void ScoreView::screenNext()
      {
      if (score()->pages().empty())
            return;
      qreal x { xoffset() };
      qreal y { yoffset() };
      if (score()->layoutMode() == LayoutMode::LINE) {
            x -= width() * scrollStep;
            MeasureBase* lm = score()->lastMeasureMM();
            // Vertical frames aren't laid out in continuous view
            while (lm->isVBoxBase())
                  lm = lm->prev();
            qreal lx = (lm->pos().x() + lm->width()) * physicalZoomLevel() - width() * scrollStep;
            if (x < -lx)
                  x = -lx;
            }
      else {
            y -= height() * scrollStep;
            MeasureBase* lm { score()->lastMeasureMM() };
            qreal ly { (lm->canvasPos().y() + lm->height()) * physicalZoomLevel() - height() * scrollStep };
            // Special case to jump to top of next page in horizontal view.
            if (score()->layoutMode() == LayoutMode::PAGE && !MScore::verticalOrientation()
               && y <= -ly - height() * scrollStep) {
                  pageNext();
                  return;
                  }
            if (y < -ly)
                  y = -ly;
            }
      setOffset(x, y);
      update();
      }

//---------------------------------------------------------
//   pagePrev
//---------------------------------------------------------

void ScoreView::pagePrev()
      {
      if (score()->pages().empty())
            return;
      if (score()->layoutMode() != LayoutMode::PAGE) {
            screenPrev();
            return;
            }
      Page* page = score()->pages().front();
      qreal x, y;
      if (MScore::verticalOrientation()) {
            x  = thinPadding;
            y  = yoffset() + (page->height() + thickPadding) * physicalZoomLevel();
            if (y > thinPadding)
                  y = thinPadding;
            }
      else {
            y  = thinPadding;
            x  = xoffset() + (page->width() + thickPadding) * physicalZoomLevel();
            if (x > thinPadding)
                  x = thinPadding;
            }
      setOffset(x, y);
      update();
      }

//---------------------------------------------------------
//   screenPrev
//---------------------------------------------------------

void ScoreView::screenPrev()
      {
      if (score()->pages().empty())
            return;
      qreal x { xoffset() };
      qreal y { yoffset() };
      if (score()->layoutMode() == LayoutMode::LINE) {
            x += width() * scrollStep;
            if (x > thinPadding)
                  x = thinPadding;
            }
      else {
            y += height() * scrollStep;
            // Special casing for jumping to bottom of prev page in horizontal view
            if (score()->layoutMode() == LayoutMode::PAGE && !MScore::verticalOrientation()
               && y >= thinPadding + height() * scrollStep) {
                  Page* page { score()->pages().front() };
                  x += (page->width() + thickPadding) * physicalZoomLevel();
                  // The condition prevents jumping to the bottom of the
                  // first page after reaching the top
                  if (x < thinPadding + (page->width() + thickPadding) * physicalZoomLevel()) {
                        MeasureBase* lm { score()->lastMeasureMM() };
                        y = -(lm->canvasPos().y() + lm->height()) * physicalZoomLevel() + height() * scrollStep;
                        }
                  if (x > thinPadding)
                        x = thinPadding;
                  }
            if (y > thinPadding)
                y = thinPadding;
            }
      setOffset(x, y);
      update();
      }

//---------------------------------------------------------
//   pageTop
//---------------------------------------------------------

void ScoreView::pageTop()
      {
      setOffset(thinPadding, score()->layoutMode() == LayoutMode::LINE ? 0.0 : thinPadding);
      update();
      }

//---------------------------------------------------------
//   pageEnd
//---------------------------------------------------------

void ScoreView::pageEnd()
      {
      if (score()->pages().empty())
            return;
      MeasureBase* lm = score()->lastMeasureMM();
      if (score()->layoutMode() == LayoutMode::LINE) {
            // Vertical frames aren't laid out in continuous view
            while (lm->isVBoxBase())
                  lm = lm->prev();
            qreal lx = (lm->pos().x() + lm->width()) * physicalZoomLevel() - scrollStep * width();
            setOffset(-lx, yoffset());
            }
      else {
            qreal lx { -thinPadding };
            if (score()->layoutMode() == LayoutMode::PAGE && !MScore::verticalOrientation()) {
                  for (int i { 0 }; i < score()->npages() - 1; ++i)
                        lx += score()->pages().at(i)->width() * physicalZoomLevel();
                  }
            if (lm->system() && lm->system()->page()->width() * physicalZoomLevel() > width())
                  lx = (lm->canvasPos().x() + lm->width()) * physicalZoomLevel() - width() * scrollStep;

            qreal ly { (lm->canvasPos().y() + lm->height()) * physicalZoomLevel() - height() * scrollStep };
            setOffset(-lx, -ly);
            }
      update();
      }

//---------------------------------------------------------
//   adjustCanvasPosition
//---------------------------------------------------------

void ScoreView::adjustCanvasPosition(const Element* el, bool playBack, int staff )
      {
      if (this != mscore->currentScoreView() && !_moveWhenInactive)
            return;
      // TODO: change icon, or add panning options
      if (!mscore->panDuringPlayback())
            return;

      if (score()->layoutMode() == LayoutMode::LINE) {

            if (!el)
                  return;

            /* Not used, because impossible to get panel width beforehand
            const MeasureBase* m = 0;
            if (el->type() == ElementType::MEASURE)
                  m = static_cast<const MeasureBase*>(el);
            else
                  m = static_cast<const Measure*>(el->parent()->findMeasure());
            */

            qreal xo = 0.0;  // new x offset
            QRectF curPos = playBack ? _cursor->rect() : el->canvasBoundingRect();
            if (playBack && _cursor && seq->isPlaying() && panSettings().enabled)
                  curPos = _controlCursor->rect();
            // keep current note in view as well if applicable (note input mode)
            Element* current = nullptr;
            if (noteEntryMode()) {
                  current = score()->selection().cr();
                  if (current && current != el)
                        curPos |= current->canvasBoundingRect();
                  }

            qreal curPosR = curPos.right();                    // Position on the canvas
            qreal curPosL = curPos.left();                     // Position on the canvas
            qreal curPosScrR = curPosR * physicalZoomLevel() + xoffset(); // Position in the screen
            qreal curPosScrL = curPosL * physicalZoomLevel() + xoffset(); // Position in the screen
            qreal marginLeft = width() * 0.05;
            qreal marginRight = width() * 0.05; // leaves 5% margin to the right

//            if (_continuousPanel->active())                           causes jump
//                  marginLeft += _continuousPanel->width() * physicalZoomLevel();

            // this code implements "continuous" panning
            // it could potentially be enabled via more panning options
            if (playBack && _cursor && seq->isPlaying() && preferences.getBool(PREF_PAN_SMOOTHLY_ENABLED)) {
                  // keep playback cursor pinned at 35% (or at the percent of controlCursorScreenPos + 5%)
                  xo = -curPosL * physicalZoomLevel() + marginLeft + width() * _panSettings.controlCursorScreenPos;
                  }
            else if (round(curPosScrR) > round(width() - marginRight)) {
                  // focus in or beyond right margin
                  // pan to left margin in playback,
                  // most of the way left in note entry,
                  // otherwise just enforce right margin
                  if (playBack)
                        xo = -curPosL * physicalZoomLevel() + marginLeft;
                  else if (noteEntryMode())
                        xo = -curPosL * physicalZoomLevel() + marginLeft + width() * 0.2;
                  else
                        xo = -curPosR * physicalZoomLevel() + width() - marginRight;
                  }
            else if (round(curPosScrL) < round(marginLeft) ) {
                  // focus in or beyond left margin
                  // enforce left margin
                  // (previously we moved canvas all the way right,
                  // but this made sense only when navigating right-to-left)
                  xo = -curPosL * physicalZoomLevel() + marginLeft;
                  }
            else {
                  // focus is within margins, so do nothing
                  return;
                  }
            // avoid empty space on either side of "page"
            qreal scoreEnd = score()->pages().front()->width() * physicalZoomLevel() + xo;
            if (xo > 10)
                  xo = 10;
            else if (scoreEnd < width())
                  xo += width() - scoreEnd;

            setOffset(xo, yoffset());
            update();
            return;
            }

      const MeasureBase* m;
      if (!el)
            return;
      else if (el->type() == ElementType::NOTE)
            m = static_cast<const Note*>(el)->chord()->measure();
      else if (el->type() == ElementType::REST)
            m = static_cast<const Rest*>(el)->measure();
      else if (el->type() == ElementType::CHORD)
            m = static_cast<const Chord*>(el)->measure();
      else if (el->type() == ElementType::SEGMENT)
            m = static_cast<const Segment*>(el)->measure();
      else if (el->type() == ElementType::LYRICS)
            m = static_cast<const Lyrics*>(el)->measure();
      else if ( (el->type() == ElementType::HARMONY || el->type() == ElementType::FIGURED_BASS)
         && el->parent()->type() == ElementType::SEGMENT)
            m = static_cast<const Segment*>(el->parent())->measure();
      else if (el->type() == ElementType::HARMONY && el->parent()->type() == ElementType::FRET_DIAGRAM
         && el->parent()->parent()->type() == ElementType::SEGMENT)
            m = static_cast<const Segment*>(el->parent()->parent())->measure();
      else if (el->isMeasureBase())
            m = static_cast<const MeasureBase*>(el);
      else if (el->isSpannerSegment()) {
            Element* se = static_cast<const SpannerSegment*>(el)->spanner()->startElement();
            m = static_cast<Measure*>(se->findMeasure());
            }
      else if (el->isSpanner()) {
            Element* se = static_cast<const Spanner*>(el)->startElement();
            m = static_cast<Measure*>(se->findMeasure());
            }
      else {
            // attempt to find measure
            Element* e = el->parent();
            while (e && !e->isMeasureBase())
                  e = e->parent();
            if (e)
                  m = toMeasureBase(e);
            else
                  return;
            }
      if (!m)
            return;

      int staffIdx = el->staffIdx();
      System* sys = m->system();
      if (!sys)
            return;

      QPointF p(el->canvasPos());
      QRectF r(canvasViewport());
      QRectF mRect(m->canvasBoundingRect());
      QRectF sysRect;
      if (staffIdx == -1)
            sysRect = sys->canvasBoundingRect();
      else
            sysRect = sys->staff(staffIdx)->bbox();

      // only try to track measure if not during playback
      if (!playBack)
            sysRect = mRect;

      double _spatium    = score()->spatium();
      const qreal border = _spatium * 3;
      QRectF showRect;
      if (staff == -1) {
            showRect = QRectF(mRect.x(), sysRect.y(), mRect.width(), sysRect.height())
                        .adjusted(-border, -border, border, border);
            }
      else {
            // find a box for the individual stave in a system
            QRectF stave = QRectF(sys->canvasBoundingRect().left(),
                                  sys->staffCanvasYpage(staff),
                                  sys->width(),
                                  sys->staff(staff)->bbox().height());
            showRect = mRect.intersected(stave).adjusted(-border, -border, border, border);
            }

/*printf("%f %f %f %f   %f %f %f %f  %d\n",
   showRect.x(), showRect.y(), showRect.width(), showRect.height(),
   r.x(), r.y(), r.width(), r.height(),
   r.contains(showRect)
   );
*/
      // canvas is not as wide as measure, track note instead
      if (r.width() < showRect.width()) {
            QRectF eRect(el->canvasBoundingRect());
            showRect.setX(eRect.x());
            showRect.setWidth(eRect.width());
            }

      // canvas is not as tall as system
      if (r.height() < showRect.height()) {
            if (sys->staves()->size() == 1 || !playBack) {
                  // track note if single staff
                  showRect.setY(p.y());
                  showRect.setHeight(el->height());
                  }
            else if (sys->page()->systems().size() == 1) {
                  // otherwise, just keep current vertical position if possible
                  // see issue #7724
                  showRect.setY(r.y());
                  showRect.setHeight(r.height());
                  }
            }
      if (shadowNote->visible())
            setShadowNote(p);

      if (r.contains(showRect))
            return;

      qreal x  = - xoffset() / physicalZoomLevel();
      qreal y  = - yoffset() / physicalZoomLevel();

      qreal oldX = x, oldY = y;

      if (showRect.left() < r.left())
            x = showRect.left() - border;
      else if (showRect.left() > r.right())
            x = showRect.right() - width() / physicalZoomLevel() + border;
      else if (r.width() >= showRect.width() && showRect.right() > r.right())
            x = showRect.left() - border;
      if (showRect.top() < r.top() && showRect.bottom() < r.bottom())
            y = showRect.top() - border;
      else if (showRect.top() > r.bottom())
            y = showRect.bottom() - height() / physicalZoomLevel() + border;
      else if (r.height() >= showRect.height() && showRect.bottom() > r.bottom())
            y = showRect.top() - border;

      // align to page borders if extends beyond
      Page* page = sys->page();
      if (x < page->x() || r.width() >= page->width())
            x = page->x();
      else if (r.width() < page->width() && r.width() + x > page->width() + page->x())
            x = (page->width() + page->x()) - r.width();
      if (y < page->y() || r.height() >= page->height())
            y = page->y();
      else if (r.height() < page->height() && r.height() + y > page->height() + page->y())
            y = (page->height() + page->y()) - r.height();

      // hack: don't update if we haven't changed the offset
      if (oldX == x && oldY == y)
            return;

      int cx = 0, cy = 0;
      constraintCanvas(&cx, &cy);
      x *= -physicalZoomLevel();
      y *= -physicalZoomLevel();
      cx = (x < 0) ? x : cx + _matrix.dx();

      setOffset(cx, y);
      update();
      }

//---------------------------------------------------------
//   cmdEnterRest
//---------------------------------------------------------

void ScoreView::cmdEnterRest()
      {
      InputState& is = _score->inputState();
      if (is.track() == -1 || is.segment() == 0)          // invalid state
            return;
      if (!is.duration().isValid() || is.duration().isZero() || is.duration().isMeasure())
            is.setDuration(TDuration::DurationType::V_QUARTER);
      cmdEnterRest(is.duration());
      }

//---------------------------------------------------------
//   cmdEnterRest
//---------------------------------------------------------

void ScoreView::cmdEnterRest(const TDuration& d)
      {
      if (!noteEntryMode())
            cmd("note-input");
      if (_score->usingNoteEntryMethod(NoteEntryMethod::RHYTHM))
            _score->cmd(getAction("pad-rest"), editData);
      else
            _score->cmdEnterRest(d);
#if 0
      expandVoice();
      if (_is.cr() == 0) {
            qDebug("cannot enter rest here");
            return;
            }

      int track = _is.track;
      Segment* seg  = setNoteRest(_is.cr(), track, -1, d.fraction(), 0, AUTO);
      ChordRest* cr = static_cast<ChordRest*>(seg->element(track));
      if (cr)
            nextInputPos(cr, false);
      _is.rest = false;  // continue with normal note entry
#endif
      }

//---------------------------------------------------------
//   mscoreState
//---------------------------------------------------------

ScoreState ScoreView::mscoreState() const
      {
      if (fotoMode())
            return STATE_FOTO;
      if (state == ViewState::NOTE_ENTRY) {
            const InputState is = _score->inputState();
            Staff* staff = _score->staff(is.track() / VOICES);

            // pitched/unpitched depending on instrument (override StaffGroup)
            StaffGroup staffGroup = staff->staffType(is.tick())->group();
            if (staffGroup != StaffGroup::TAB)
                  staffGroup = staff->part()->instrument(is.tick())->useDrumset() ? StaffGroup::PERCUSSION : StaffGroup::STANDARD;
            switch( staffGroup ) {
                  case StaffGroup::STANDARD:
                        return STATE_NOTE_ENTRY_STAFF_PITCHED;
                  case StaffGroup::TAB:
                        return STATE_NOTE_ENTRY_STAFF_TAB;
                  case StaffGroup::PERCUSSION:
                        return STATE_NOTE_ENTRY_STAFF_DRUM;
                  }
            }
      if (state == ViewState::EDIT || state == ViewState::DRAG_EDIT) {
            if (editData.element && editData.element->isLyrics())
                  return STATE_LYRICS_EDIT;
            else if (editData.element &&
                  ( (editData.element->isHarmony()) || editData.element->isFiguredBass()) )
                  return STATE_HARMONY_FIGBASS_EDIT;
            else if (editData.element && editData.element->isTextBase())
                  return STATE_TEXT_EDIT;
            return STATE_EDIT;
            }
      if (state == ViewState::PLAY)
            return STATE_PLAY;
      return STATE_NORMAL;
      }

//---------------------------------------------------------
//   startUndoRedo
//---------------------------------------------------------

void ScoreView::startUndoRedo(bool undo)
      {
      _score->undoRedo(undo, state == ViewState::EDIT ? &editData : 0);

      if (_score->inputState().segment())
            mscore->setPos(_score->inputState().tick());
      if (_score->noteEntryMode() && !noteEntryMode())
            cmd("note-input");    // enter note entry mode
      else if (!_score->noteEntryMode() && noteEntryMode())
            cmd("escape");        // leave note entry mode

      updateEditElement();
      }

//---------------------------------------------------------
//   cmdAddSlur
//    command invoked, or icon double clicked
//---------------------------------------------------------

void ScoreView::cmdAddSlur(const Slur* slurTemplate)
      {
      InputState& is = _score->inputState();
      if (noteEntryMode() && is.slur()) {
            const std::vector<SpannerSegment*>& el = is.slur()->spannerSegments();
            if (!el.empty()) {
                  el.front()->setSelected(false);
                  // Now make sure that the slur segment is redrawn so that it does not *look* selected
                  update();
                  }
            is.setSlur(nullptr);
            return;
            }

      _score->startCmd();

      ChordRest* cr1;
      ChordRest* cr2;
      const auto& sel = _score->selection();
      auto el         = sel.uniqueElements();

      if (sel.isRange()) {
            int startTrack = sel.staffStart() * VOICES;
            int endTrack   = sel.staffEnd() * VOICES;
            for (int track = startTrack; track < endTrack; ++track) {
                  cr1 = 0;
                  cr2 = 0;
                  for (Element* e : el) {
                        if (e->track() != track)
                              continue;
                        if (e->isNote())
                              e = toNote(e)->chord();
                        if (!e->isChord())
                              continue;
                        ChordRest* cr = toChordRest(e);
                        if (!cr1 || cr->isBefore(cr1))
                              cr1 = cr;
                        if (!cr2 || cr2->isBefore(cr))
                              cr2 = cr;
                        }
                  if (cr1 && (cr1 != cr2))
                        addSlur(cr1, cr2, slurTemplate);
                  }
            }
      else {
            cr1 = 0;
            cr2 = 0;
            for (Element* e : el) {
                  if (e->isNote())
                        e = toNote(e)->chord();
                  if (!e->isChord())
                        continue;
                  ChordRest* cr = toChordRest(e);
                  if (!cr1 || cr->isBefore(cr1))
                        cr1 = cr;
                  if (!cr2 || cr2->isBefore(cr))
                        cr2 = cr;
                  }
            if (cr1 == cr2)
                  cr2 = 0;
            if (cr1)
                  addSlur(cr1, cr2, slurTemplate);
            }
      _score->endCmd();
      }

//---------------------------------------------------------
//   addSlur
//---------------------------------------------------------

void ScoreView::addSlur(ChordRest* cr1, ChordRest* cr2, const Slur* slurTemplate)
      {
      bool switchToSlur = false;
      if (cr2 == 0) {
            cr2 = nextChordRest(cr1);
            if (cr2 == 0)
                  cr2 = cr1;
            switchToSlur = true; // select slur for editing if last chord is not given
            }

      Slur* slur = slurTemplate ? slurTemplate->clone() : new Slur(cr1->score());
      slur->setScore(cr1->score());
      slur->setTick(cr1->tick());
      slur->setTick2(cr2->tick());
      slur->setTrack(cr1->track());
      if (cr2->staff()->part() == cr1->staff()->part() && !cr2->staff()->isLinked(cr1->staff()))
            slur->setTrack2(cr2->track());
      else
            slur->setTrack2(cr1->track());
      slur->setStartElement(cr1);
      slur->setEndElement(cr2);

      cr1->score()->undoAddElement(slur);
      SlurSegment* ss = new SlurSegment(cr1->score());
      ss->setSpannerSegmentType(SpannerSegmentType::SINGLE);
      if (cr1 == cr2)
            ss->setSlurOffset(Grip::END, QPointF(3.0 * cr1->score()->spatium(), 0.0));
      slur->add(ss);

      if (noteEntryMode()) {
            _score->inputState().setSlur(slur);
            ss->setSelected(true);
            }
      else if (switchToSlur && score()->selection().isSingle()) {
            startEditMode(ss);
            }
      }

//---------------------------------------------------------
//   cmdAddHairpin
//    '<' typed on keyboard
//---------------------------------------------------------

void ScoreView::cmdAddHairpin(HairpinType type)
      {
      const Selection& selection = _score->selection();
      // special case for two selected chordrests on same staff
      bool twoNotesSameStaff = false;
      if (selection.isList() && selection.elements().size() == 2) {
            ChordRest* cr1 = selection.firstChordRest();
            ChordRest* cr2 = selection.lastChordRest();
            if (cr1 && cr2 && cr1 != cr2 && cr1->staffIdx() == cr2->staffIdx())
                  twoNotesSameStaff = true;
            }
      // add hairpin on each staff if possible
      if (selection.isRange() && selection.staffStart() != selection.staffEnd() - 1) {
            _score->startCmd();
            for (int staffIdx = selection.staffStart() ; staffIdx < selection.staffEnd(); ++staffIdx) {
                  ChordRest* cr1 = selection.firstChordRest(staffIdx * VOICES);
                  ChordRest* cr2 = selection.lastChordRest(staffIdx * VOICES);
                  _score->addHairpin(type, cr1, cr2, /* toCr2End */ true);
                  }
            _score->endCmd();
            }
      else if (selection.isRange() || selection.isSingle() || twoNotesSameStaff) {
            // for single staff range selection, or single selection,
            // find start & end elements elements
            ChordRest* cr1;
            ChordRest* cr2;
            _score->getSelectedChordRest2(&cr1, &cr2);
            _score->startCmd();
            Hairpin* pin = _score->addHairpin(type, cr1, cr2, /* toCr2End */ !twoNotesSameStaff);
            _score->endCmd();
            if (!pin)
                  return;

            const std::vector<SpannerSegment*>& el = pin->spannerSegments();
            if (!noteEntryMode()) {
                  if (!el.empty()) {
                        startEditMode(el.front());
                        }
                  }
            }
      else {
            // do not attempt for list selection
            // or we will keep adding hairpins to the same chordrests
            return;
            }
      }

//---------------------------------------------------------
//   cmdAddNoteLine
//---------------------------------------------------------

void ScoreView::cmdAddNoteLine()
      {
      Note* firstNote = 0;
      Note* lastNote  = 0;
      if (_score->selection().isRange()) {
            int startTrack = _score->selection().staffStart() * VOICES;
            int endTrack   = _score->selection().staffEnd() * VOICES;
            for (int track = startTrack; track < endTrack; ++track) {
                  for (Note* n : _score->selection().noteList(track)) {
                        if (firstNote == 0 || firstNote->chord()->tick() > n->chord()->tick())
                              firstNote = n;
                        if (lastNote == 0 || lastNote->chord()->tick() < n->chord()->tick())
                              lastNote = n;
                        }
                  }
            }
      else {
            for (Note* n : _score->selection().noteList()) {
                  if (firstNote == 0 || firstNote->chord()->tick() > n->chord()->tick())
                        firstNote = n;
                  if (lastNote == 0 || lastNote->chord()->tick() < n->chord()->tick())
                        lastNote = n;
                  }
            }
      if (!firstNote || !lastNote) {
            qDebug("addNoteLine: no note %p %p", firstNote, lastNote);
            return;
            }
      if (firstNote == lastNote) {
            qDebug("addNoteLine: no support for note to same note line %p", firstNote);
            return;
            }
      TextLine* tl = new TextLine(_score);
      tl->setParent(firstNote);
      tl->setStartElement(firstNote);
      tl->setDiagonal(true);
      tl->setAnchor(Spanner::Anchor::NOTE);
      tl->setTick(firstNote->chord()->tick());
      tl->setEndElement(lastNote);
      _score->startCmd();
      _score->undoAddElement(tl);
      _score->endCmd();
      tl->setEndElement(lastNote);
      }

//---------------------------------------------------------
//   cmdChangeEnharmonic
//---------------------------------------------------------

void ScoreView::cmdChangeEnharmonic(bool both)
      {
      _score->startCmd();
      std::list<Note*> notes = _score->selection().uniqueNotes();
      for (Note* n : notes) {
            Staff* staff = n->staff();
            if (staff->part()->instrument(n->tick())->useDrumset())
                  continue;
            if (staff->isTabStaff(n->tick())) {
                  int string = n->line() + (both ? 1 : -1);
                  int fret   = staff->part()->instrument(n->tick())->stringData()->fret(n->pitch(), string, staff, n->chord()->tick());
                  if (fret != -1) {
                        n->undoChangeProperty(Pid::FRET, fret);
                        n->undoChangeProperty(Pid::STRING, string);
                        }
                  }
            else {
                  static const int tab[36] = {
                        26, 14,  2,  // 60  B#   C   Dbb
                        21, 21,  9,  // 61  C#   C#  Db
                        28, 16,  4,  // 62  C##  D   Ebb
                        23, 23, 11,  // 63  D#   D#  Eb
                        30, 18,  6,  // 64  D##  E   Fb
                        25, 13,  1,  // 65  E#   F   Gbb
                        20, 20,  8,  // 66  F#   F#  Gb
                        27, 15,  3,  // 67  F##  G   Abb
                        22, 22, 10,  // 68  G#   G#  Ab
                        29, 17,  5,  // 69  G##  A   Bbb
                        24, 24, 12,  // 70  A#   A#  Bb
                        31, 19,  7,  // 71  A##  B   Cb
                        };
                  int tpc = n->tpc();
                  int i;
                  for (i = 0; i < 36; ++i) {
                        if (tab[i] == tpc) {
                              if ((i % 3) < 2) {
                                    if (tab[i] == tab[i + 1])
                                          tpc = tab[i + 2];
                                    else
                                          tpc = tab[i + 1];
                                    }
                              else {
                                    tpc = tab[i - 2];
                                    }
                              break;
                              }
                        }
                  if (i == 36) {
                        qDebug("tpc %d not found", tpc);
                        }
                  else {
                        n->undoSetTpc(tpc);
                        if (both || staff->part()->instrument(n->chord()->tick())->transpose().isZero()) {
                              // change both spellings
                              int t = n->transposeTpc(tpc);
                              if (n->concertPitch())
                                    n->undoChangeProperty(Pid::TPC2, t);
                              else
                                    n->undoChangeProperty(Pid::TPC1, t);
                              }
                        }
                  }
            }

      Selection& selection = _score->selection();
      selection.clear();
      for (Note* n : notes)
            selection.add(n);
      _score->endCmd();
      }

//---------------------------------------------------------
//   cloneElement
//---------------------------------------------------------

void ScoreView::cloneElement(Element* e)
      {
      if (e->isMeasure() || e->isNote() || e->isVBox())
            return;
      QDrag* drag = new QDrag(this);
      QMimeData* mimeData = new QMimeData;
      if (e->isSpannerSegment())
            e = toSpannerSegment(e)->spanner();
      mimeData->setData(mimeSymbolFormat, e->mimeData(QPointF()));
      drag->setMimeData(mimeData);
      static QPixmap pixmap = QPixmap(2, 2);    // null or 1x1 crashes on Linux under ChromeOS?!
      pixmap.fill(Qt::white);
      drag->setPixmap(pixmap);
      drag->exec(Qt::CopyAction);
      }

//---------------------------------------------------------
//   changeEditElement
//---------------------------------------------------------

void ScoreView::changeEditElement(Element* e)
      {
      if (editData.element == e)
            return;

      const Grip grip = editData.curGrip;
      const bool dragEdit = (state == ViewState::DRAG_EDIT);

      if (editData.element && dragEdit)
            editData.element->endEditDrag(editData);

      endEdit();
      startEdit(e, grip);

      if (editData.element && dragEdit)
            editData.element->startEditDrag(editData);
      }

//---------------------------------------------------------
//   setCursorVisible
//---------------------------------------------------------

void ScoreView::setCursorVisible(bool v)
      {
      _cursor->setVisible(v);
      }

//---------------------------------------------------------
//   setContinuousCursorVisible
//---------------------------------------------------------

void ScoreView::setControlCursorVisible(bool v)
      {
      _controlCursor->setVisible(v);
      }

//---------------------------------------------------------
//   cmdTuplet
//---------------------------------------------------------

void ScoreView::cmdTuplet(int n, ChordRest* cr)
      {
      if ((cr->durationType() < TDuration(TDuration::DurationType::V_512TH) && cr->durationType() != TDuration(TDuration::DurationType::V_MEASURE))
          || (cr->durationType() < TDuration(TDuration::DurationType::V_256TH) && n > 3)
          || (cr->durationType() < TDuration(TDuration::DurationType::V_128TH) && n > 7)
          ) {
            mscore->noteTooShortForTupletDialog();
            return;
            }
      Measure* measure = cr->measure();
      if (measure && measure->isMMRest())
            return;

      Fraction f(cr->ticks());
      Tuplet* ot  = cr->tuplet();

      f.reduce();       //measure duration might not be reduced
      Fraction ratio(n, f.numerator());
      Fraction fr(1, f.denominator());
      while (ratio.numerator() >= ratio.denominator() * 2) {
            ratio.setDenominator(ratio.denominator() * 2);  // operator*= reduces, we don't want that here
            fr    *= Fraction(1,2);
            }

      Tuplet* tuplet = new Tuplet(_score);
      tuplet->setRatio(ratio);

      //
      // "fr" is the fraction value of one tuple element
      //
      // "tuplet time" is "normal time" / tuplet->ratio()
      //    Example: an 1/8 has 240 midi ticks, in an 1/8 triplet the note
      //             has a tick duration of 240 / (3/2) = 160 ticks
      //

      tuplet->setTicks(f);
      TDuration baseLen(fr);
      tuplet->setBaseLen(baseLen);

      tuplet->setTrack(cr->track());
      tuplet->setTick(cr->tick());
      tuplet->setParent(measure);

      if (ot)
            tuplet->setTuplet(ot);

      cmdCreateTuplet(cr, tuplet);
      }

//---------------------------------------------------------
//   cmdCreateTuplet
//---------------------------------------------------------

void ScoreView::cmdCreateTuplet(ChordRest* cr, Tuplet* tuplet)
      {
      _score->cmdCreateTuplet(cr, tuplet);

      const std::vector<DurationElement*>& cl = tuplet->elements();
      size_t ne = cl.size();
      DurationElement* el = 0;
      if (ne && cl[0]->type() == ElementType::REST)
            el  = cl[0];
      else if (ne > 1)
            el = cl[1];
      if (el) {
            _score->select(el, SelectType::SINGLE, 0);
            _score->inputState().setDuration(tuplet->baseLen());
            changeState(ViewState::NOTE_ENTRY);
            }
      }

//---------------------------------------------------------
//   changeVoice
//---------------------------------------------------------

void ScoreView::changeVoice(int voice)
      {
      InputState* is = &score()->inputState();
      int track = (is->track() / VOICES) * VOICES + voice;

      if (is->noteEntryMode()) {
            is->setTrack(track);
            if (is->segment()) { // can be null for eg repeatMeasure
                  is->setSegment(is->segment()->measure()->first(SegmentType::ChordRest));
                  moveCursor();
                  score()->setUpdateAll();
                  score()->update();
                  mscore->setPos(is->segment()->tick());
                  }
            }
      else {
            // treat as command to move notes to another voice
            score()->changeVoice(voice);
            // modify the input state only if the command was successful
            for (ChordRest* cr : score()->getSelectedChordRests())
                  if (cr->voice() == voice) {
                        is->setTrack(track);
                        break;
                        }
            }
      }

//---------------------------------------------------------
//   cmdTuplet
//---------------------------------------------------------

void ScoreView::cmdTuplet(int n)
      {
      _score->startCmd();
      if (noteEntryMode()) {
            _score->expandVoice();
            ChordRest* cr = _score->inputState().cr();
            if (cr) {
                  _score->changeCRlen(cr, _score->inputState().duration());
                  cmdTuplet(n, cr);
                  }
            }
      else {
            for (ChordRest* cr : _score->getSelectedChordRests()) {
                  if (!cr->isGrace()) {
                        cmdTuplet(n, cr);
                        }
                  }
            }
      _score->endCmd();
      moveCursor();     // do this after endCmd to make sure segment has been laid out
      }

//---------------------------------------------------------
//   midiNoteReceived
//---------------------------------------------------------

void ScoreView::midiNoteReceived(int pitch, bool chord, int velocity)
      {
      qDebug("midiNoteReceived: pitch %d, chord %d, velocity %d", pitch, chord, velocity);

      MidiInputEvent ev;
      ev.pitch = pitch;
      ev.chord = chord;
      ev.velocity = velocity;

      score()->masterScore()->enqueueMidiEvent(ev);

      if (!score()->undoStack()->active())
            cmd((const char*)0);

      if (!chord && velocity && !realtimeTimer->isActive() && score()->usingNoteEntryMethod(NoteEntryMethod::REALTIME_AUTO)) {
            // First note pressed in automatic real-time mode.
            extendNoteTimer->start(preferences.getInt(PREF_IO_MIDI_REALTIMEDELAY)); // set timer to trigger repeatedly
            triggerCmdRealtimeAdvance(); // also trigger once immediately
            }

      }

//---------------------------------------------------------
//   extendCurrentNote
//    Called after user has held down a midi key for a while.
//    TODO: adapt to allow calling from StepTime mode.
//---------------------------------------------------------

void ScoreView::extendCurrentNote()
      {
      if (!noteEntryMode() || realtimeTimer->isActive())
            return;

      allowRealtimeRests = false;
      realtimeTimer->start(preferences.getInt(PREF_IO_MIDI_REALTIMEDELAY)); // set timer to trigger repeatedly
      triggerCmdRealtimeAdvance(); // also trigger once immediately
      }

//---------------------------------------------------------
//   realtimeAdvance
//---------------------------------------------------------

void ScoreView::realtimeAdvance(bool allowRests)
      {
      if (!noteEntryMode())
            return;
      InputState& is = score()->inputState();
      switch (is.noteEntryMethod()) {
            case NoteEntryMethod::REALTIME_MANUAL:
                  allowRealtimeRests = allowRests;
                  triggerCmdRealtimeAdvance();
                  break;
            case NoteEntryMethod::REALTIME_AUTO:
                  if (realtimeTimer->isActive())
                        realtimeTimer->stop();
                  else {
                        allowRealtimeRests = allowRests;
                        realtimeTimer->start(preferences.getInt(PREF_IO_MIDI_REALTIMEDELAY));
                        }
                  break;
            default:
                  break;
            }
      }

//---------------------------------------------------------
//   triggerCmdRealtimeAdvance
//---------------------------------------------------------

void ScoreView::triggerCmdRealtimeAdvance()
      {
      InputState& is = score()->inputState();
      bool realtime = is.usingNoteEntryMethod(NoteEntryMethod::REALTIME_AUTO) || is.usingNoteEntryMethod(NoteEntryMethod::REALTIME_MANUAL);
      if (!realtime || !noteEntryMode() || (!allowRealtimeRests && score()->activeMidiPitches()->empty())) {
            if (realtimeTimer->isActive())
                  realtimeTimer->stop();
            allowRealtimeRests = true;
            return;
            }
      // give audible feedback immediately to indicate a beat, but don’t advance just yet.
      seq->playMetronomeBeat(_score->tick2beatType(is.tick()));
      // The user will want to press notes "on the beat" and not before the beat, so wait a
      // little in case midi input event is received just after realtime-advance was called.
      QTimer::singleShot(100, Qt::PreciseTimer, this, SLOT(cmdRealtimeAdvance()));
      }

//---------------------------------------------------------
//   cmdRealtimeAdvance
//    move input forwards and extend current chord/rest.
//---------------------------------------------------------

void ScoreView::cmdRealtimeAdvance()
      {
      InputState& is = _score->inputState();
      if (!is.noteEntryMode())
            return;
      _score->startCmd();
      Fraction ticks2measureEnd = is.segment()->measure()->ticks() - is.segment()->rtick();
      if (!is.cr() || (is.cr()->ticks() != is.duration().fraction() && is.duration() < ticks2measureEnd))
            _score->setNoteRest(is.segment(), is.track(), NoteVal(), is.duration().fraction(), Direction::AUTO);
      ChordRest* prevCR = toChordRest(is.cr());
      if (_score->inputState().endOfScore())
            _score->appendMeasures(1);
      is.moveToNextInputPos();
      if (_score->activeMidiPitches()->empty())
            _score->setNoteRest(is.segment(), is.track(), NoteVal(), is.duration().fraction(), Direction::AUTO);
      else {
            Chord* prevChord = prevCR->isChord() ? toChord(prevCR) : 0;
            bool partOfChord = false;
            for (const MidiInputEvent &ev : *_score->activeMidiPitches()) {
                  _score->addTiedMidiPitch(ev.pitch, partOfChord, prevChord);
                  partOfChord = true;
                  }
            }
      if (prevCR->measure() != is.segment()->measure()) {
            // just advanced across barline. Now simplify tied notes.
            score()->regroupNotesAndRests(prevCR->measure()->tick(), is.segment()->measure()->tick(), is.track());
            }
      _score->endCmd();
      }

//---------------------------------------------------------
//   cmdAddChordName
//---------------------------------------------------------

void ScoreView::cmdAddChordName(HarmonyType ht)
      {
      if (!_score->checkHasMeasures())
            return;

      int track = -1;
      Element* newParent = nullptr;
      Element* el = _score->selection().element();
      if (el && el->isFretDiagram()) {
            FretDiagram* fd = toFretDiagram(el);
            track = fd->track();
            newParent = fd;
            }
      else {
            ChordRest* cr = _score->getSelectedChordRest();
            if (cr) {
                  track = cr->track();
                  newParent = toElement(cr->segment());
                  }
            }
      if (track == -1 || !newParent)
            return;

      _score->startCmd();
      Harmony* harmony = new Harmony(_score);
      harmony->setTrack(track);
      harmony->setParent(newParent);
      harmony->setHarmonyType(ht);
      _score->undoAddElement(harmony);
      _score->endCmd();

      _score->select(harmony, SelectType::SINGLE, 0);
      startEditMode(harmony);
      _score->update();
      }

//---------------------------------------------------------
//   cmdAddText
//---------------------------------------------------------

void ScoreView::cmdAddText(Tid tid, Tid customTid, PropertyFlags pf, Placement p)
      {
      if (!_score->checkHasMeasures())
            return;
      if (noteEntryMode())          // force out of entry mode
            changeState(ViewState::NORMAL);

      TextBase* s = 0;
      _score->startCmd();
      if (tid == Tid::STAFF && customTid == Tid::EXPRESSION)
            tid = customTid;  // expression is not first class element, but treat as such
      switch (tid) {
            case Tid::FRAME:
            case Tid::TITLE:
            case Tid::SUBTITLE:
            case Tid::COMPOSER:
            case Tid::POET:
            case Tid::INSTRUMENT_EXCERPT:
                  {
                  MeasureBase* measure = _score->first();
                  if (!measure->isVBox()) {
                        _score->insertMeasure(ElementType::VBOX, measure);
                        measure = measure->prev();
                        }
                  s = new Text(_score, tid);
                  s->setParent(measure);
                  _score->undoAddElement(s);
                  }
                  break;

            case Tid::REHEARSAL_MARK:
                  {
                  ChordRest* cr = _score->getSelectedChordRest();
                  if (!cr)
                        break;
                  s = new RehearsalMark(_score);
                  cr->undoAddAnnotation(s);
                  }
                  break;
            case Tid::STAFF:
                  {
                  ChordRest* cr = _score->getSelectedChordRest();
                  if (!cr)
                        break;
                  s = new StaffText(_score, Tid::STAFF);
                  // if the previous text is UNSTYLED or NOSTYLE
                  // and the next text is created using text tab command
                  // (only those created in this way can make pf not STYLED, see definition of ScoreView::textTab())
                  // it inherits the same PropertyFlags and Placement
                  // making it much easier to put all texts above/below staves
                  if (pf != PropertyFlags::STYLED) {
                        s->setPlacement(p);
                        s->setPropertyFlags(Pid::PLACEMENT, pf);
                        }
                  cr->undoAddAnnotation(s);
                  }
                  break;
            case Tid::SYSTEM:
                  {
                  ChordRest* cr = _score->getSelectedChordRest();
                  if (!cr)
                        break;
                  s = new SystemText(_score, Tid::SYSTEM);
                  if (pf != PropertyFlags::STYLED) {
                        s->setPlacement(p);
                        s->setPropertyFlags(Pid::PLACEMENT, pf);
                        }
                  cr->undoAddAnnotation(s);
                  }
                  break;
            case Tid::EXPRESSION:
                  {
                  ChordRest* cr = _score->getSelectedChordRest();
                  if (!cr)
                        break;
                  s = new StaffText(_score, Tid::EXPRESSION);
                  s->setPlacement(Placement::BELOW);
                  s->setPropertyFlags(Pid::PLACEMENT, PropertyFlags::UNSTYLED);
                  cr->undoAddAnnotation(s);
                  }
                  break;
            case Tid::INSTRUMENT_CHANGE:
                  {
                  ChordRest* cr = _score->getSelectedChordRest();
                  if (!cr)
                        break;
                  s = new InstrumentChange(_score);
                  cr->undoAddAnnotation(s);
                  }
                  break;
            case Tid::STICKING:
                  {
                  ChordRest* cr = _score->getSelectedChordRest();
                  if (!cr)
                        break;
                  s = new Sticking(_score);
                  cr->undoAddAnnotation(s);
                  }
                  break;
            case Tid::FINGERING:
            case Tid::LH_GUITAR_FINGERING:
            case Tid::RH_GUITAR_FINGERING:
            case Tid::STRING_NUMBER:
                  {
                  Element* e = _score->getSelectedElement();
                  if (!e || !e->isNote())
                        break;
                  bool isTablature = e->staff()->isTabStaff(e->tick());
                  bool tabFingering = e->staff()->staffType(e->tick())->showTabFingering();
                  if (isTablature && !tabFingering)
                        break;
                  s = new Fingering(_score, tid);
                  s->setTrack(e->track());
                  s->setParent(e);
                  if (pf != PropertyFlags::STYLED) {
                        s->setPlacement(p);
                        s->setPropertyFlags(Pid::PLACEMENT, pf);
                        }
                  _score->undoAddElement(s);
                  }
                  break;
            default:
                  break;
            }

      if (s) {
            if (customTid != Tid::DEFAULT && customTid != tid)
                  s->initTid(customTid);
            _score->select(s, SelectType::SINGLE, 0);
            _score->endCmd();
            Measure* m = s->findMeasure();
            if (m && m->hasMMRest() && s->links()) {
                  Measure* mmRest = m->mmRest();
                  for (ScoreElement* se : *s->links()) {
                        TextBase* s1 = toTextBase(se);
                        if (s != s1 && s1->findMeasure() == mmRest) {
                              s = s1;
                              break;
                              }
                        }
                  }
            adjustCanvasPosition(s, false);
            startEditMode(s);
            }
      else
            _score->endCmd();
      }

//---------------------------------------------------------
//   cmdAppendMeasures
///   Append \a n measures.
///
///   Keyboard callback, called from pulldown menu.
//
//    - called from pulldown menu
//---------------------------------------------------------

void ScoreView::cmdAppendMeasures(int n, ElementType type)
      {
      _score->startCmd();
      appendMeasures(n, type);
      _score->endCmd();
      }

//---------------------------------------------------------
//   appendMeasure
//---------------------------------------------------------

MeasureBase* ScoreView::appendMeasure(ElementType type)
      {
      _score->startCmd();
      _score->insertMeasure(type, 0);
      MeasureBase* mb = _score->lastMeasureMM();
      _score->endCmd();
      return mb;
      }

//---------------------------------------------------------
//   appendMeasures
//---------------------------------------------------------

void ScoreView::appendMeasures(int n, ElementType type)
      {
      if (_score->noStaves()) {
            QMessageBox::warning(0, "MuseScore",
               tr("No staves found:\n"
                  "Please use the instruments dialog to\n"
                  "first create some staves"));
            return;
            }
      for (int i = 0; i < n; ++i)
            _score->insertMeasure(type, 0);
      }

//---------------------------------------------------------
//   cmdInsertMeasures
//---------------------------------------------------------

void ScoreView::cmdInsertMeasures(int n, ElementType type)
      {
      MeasureBase* mb = checkSelectionStateForInsertMeasure();
      if (!mb)
            return;
      _score->startCmd();
      for (int i = 0; i < n; ++i)
            _score->insertMeasure(type, mb);
      _score->endCmd();

      if (mb->type() == ElementType::MEASURE) {
            // re-select the original measure (which may now be covered by an mmrest)
            // do this after the layout so mmrests are updated
            Measure* m = _score->tick2measureMM(mb->tick());
            _score->select(m, SelectType::SINGLE, 0);
            }
      else {
            // original selection was not a measure, just re-select it
            _score->select(mb, SelectType::SINGLE, 0);
            }
      }

//---------------------------------------------------------
//   cmdInsertMeasure
//---------------------------------------------------------

void ScoreView::cmdInsertMeasure(ElementType type)
      {
      MeasureBase* mb = checkSelectionStateForInsertMeasure();
      if (!mb)
            return;
      _score->startCmd();
      _score->insertMeasure(type, mb);
      mb = mb->prev();
      if (mb->type() == ElementType::TBOX) {
            TBox* tbox = static_cast<TBox*>(mb);
            Text* s = tbox->text();
            _score->select(s, SelectType::SINGLE, 0);
            _score->endCmd();
            startEditMode(s);
            return;
            }
      if (mb)
           _score->select(mb, SelectType::SINGLE, 0);
      _score->endCmd();
      }

//---------------------------------------------------------
//   checkSelectionStateForInsertMeasure
//---------------------------------------------------------

MeasureBase* ScoreView::checkSelectionStateForInsertMeasure()
      {
      MeasureBase* mb = 0;
      if (_score->selection().isRange()) {
            mb = _score->selection().startSegment()->measure();
            return mb;
            }

      mb = _score->selection().findMeasure();
      if (mb)
            return mb;

      Element* e = _score->selection().element();
      if (e) {
            if (e->type() == ElementType::VBOX || e->type() == ElementType::TBOX || e->type() == ElementType::HBOX)
                  return static_cast<MeasureBase*>(e);
            }
      QMessageBox::warning(0, "MuseScore",
            tr("No measure selected:\n" "Please select a measure and try again"));
      return 0;
      }

//---------------------------------------------------------
//   cmdRepeatSelection
//---------------------------------------------------------

void ScoreView::cmdRepeatSelection()
      {
      const Selection& selection = _score->selection();

      if (noteEntryMode() && selection.isSingle()) {
            Element* el = _score->selection().element();
            if (el && el->type() == ElementType::NOTE) {
                  if (!_score->inputState().endOfScore()) {
                        _score->startCmd();
                        bool addTo = false;
                        Chord* c = static_cast<Note*>(el)->chord();
                        for (Note* note : c->notes()) {
                              NoteVal nval = note->noteVal();
                              _score->addPitch(nval, addTo);
                              addTo = true;
                              }
                        _score->endCmd();
                        }
                  }
            return;
            }
      if (!selection.isRange()) {
            ChordRest* cr = _score->getSelectedChordRest();
            if (!cr)
                  return;
            _score->select(cr, SelectType::RANGE);
            }

      if (!checkCopyOrCut())
            return;

      QString mimeType = selection.mimeType();
      if (mimeType.isEmpty()) {
            qDebug("mime type is empty");
            return;
            }
      QMimeData* mimeData = new QMimeData;
      mimeData->setData(mimeType, selection.mimeData());
      if (MScore::debugMode)
            qDebug("cmdRepeatSelection: <%s>", mimeData->data(mimeType).data());
      QApplication::clipboard()->setMimeData(mimeData);

      QByteArray d(mimeData->data(mimeType));
      XmlReader xml(d);
      xml.setPasteMode(true);

      int dStaff = selection.staffStart();
      Segment* endSegment = selection.endSegment();

      if (endSegment && endSegment->segmentType() != SegmentType::ChordRest)
            endSegment = endSegment->next1(SegmentType::ChordRest);
      if (endSegment && endSegment->element(dStaff * VOICES)) {
            Element* e = endSegment->element(dStaff * VOICES);
            if (e) {
                  ChordRest* cr = toChordRest(e);
                  _score->startCmd();
                  _score->pasteStaff(xml, cr->segment(), cr->staffIdx());
                  _score->endCmd();
                  }
            else
                  qDebug("ScoreView::cmdRepeatSelection: cannot paste: %p <%s>", e, e ? e->name() : "");
            }
      else {
            qDebug("cmdRepeatSelection: cannot paste: endSegment: %p dStaff %d", endSegment, dStaff);
            }
      }

//---------------------------------------------------------
//   searchPage
//---------------------------------------------------------

bool ScoreView::searchPage(int n)
      {
      bool result = true;
      n -= score()->pageNumberOffset();
      if (n <= 0) {
            n = 1;
            result = false;
            }
      n--;
      if (n >= _score->npages()) {
            result = false;
            n = _score->npages() - 1;
            }
      const Page* page = _score->pages()[n];
      foreach (System* s, page->systems()) {
            if (s->firstMeasure()) {
                  gotoMeasure(s->firstMeasure());
                  break;
                  }
            }
      return result;
      }

//---------------------------------------------------------
//   searchMeasure
//---------------------------------------------------------

bool ScoreView::searchMeasure(int n)
      {
      if (n <= 0)
            return false;
      bool result = true;
      --n;
      int i = 0;
      Measure* measure;
      for (measure = _score->firstMeasureMM(); measure; measure = measure->nextMeasureMM()) {
            int nn = _score->styleB(Sid::createMultiMeasureRests) && measure->isMMRest()
               ? measure->mmRestCount() : 1;
            if (n >= i && n < (i+nn))
                  break;
            i += nn;
            }
      if (!measure) {
            measure = score()->lastMeasureMM();
            result = false;
            }
      gotoMeasure(measure);
      return result;
      }

//---------------------------------------------------------
//   searchRehearsalMark
//---------------------------------------------------------

bool ScoreView::searchRehearsalMark(const QString& s)
      {
      //search rehearsal marks
      QString ss = s.toLower();
      bool found = false;
      for (Segment* seg = score()->firstSegment(SegmentType::ChordRest); seg; seg = seg->next1(SegmentType::ChordRest)) {
            for (Element* e : seg->annotations()){
                  if (e->type() == ElementType::REHEARSAL_MARK) {
                        RehearsalMark* rm = static_cast<RehearsalMark*>(e);
                        QString rms = rm->plainText().toLower();
                        if (rms.startsWith(ss)) {
                              gotoMeasure(seg->measure());
                              found = true;
                              break;
                              }
                        }
                  }
            if (found)
                  break;
            }
      return found;
      }

//---------------------------------------------------------
//   gotoMeasure
//---------------------------------------------------------

void ScoreView::gotoMeasure(Measure* measure)
      {
      adjustCanvasPosition(measure, state != ViewState::NORMAL);
      int tracks = _score->nstaves() * VOICES;
      for (Segment* segment = measure->first(); segment; segment = segment->next()) {
            if (segment->segmentType() != SegmentType::ChordRest)
                  continue;
            int track;
            for (track = 0; track < tracks; ++track) {
                  ChordRest* cr = static_cast<ChordRest*>(segment->element(track));
                  if (cr) {
                        Element* e;
                        if (cr->type() == ElementType::CHORD)
                              e =  static_cast<Chord*>(cr)->upNote();
                        else //REST
                              e = cr;

                        _score->select(e, SelectType::SINGLE, 0);
                        break;
                        }
                  }
            if (track != tracks)
                  break;
            }
      _score->setUpdateAll();
      _score->update();
      }

//---------------------------------------------------------
//   layoutChanged
//---------------------------------------------------------

void ScoreView::layoutChanged()
      {
      if (mscore->navigator())
            mscore->navigator()->layoutChanged();
      _curLoopIn->move(_score->pos(POS::LEFT));
      Measure* lm = _score->lastMeasure();
      if (lm && _score->pos(POS::RIGHT) > lm->endTick())
            _score->setPos(POS::RIGHT, lm->endTick());
      _curLoopOut->move(_score->pos(POS::RIGHT));
      }

//---------------------------------------------------------
//   elementLower
//---------------------------------------------------------

static bool elementLower(const Element* e1, const Element* e2)
      {
      if (!e1->selectable())
            return false;
      if (!e2->selectable())
            return true;
      if (e1->isNote() && e2->isStem())
            return true;
      if (e2->isNote() && e1->isStem())
            return false;
      if (e1->z() == e2->z()) {
            // same stacking order, prefer non-hidden elements
            if (e1->type() == e2->type()) {
                  if (e1->type() == ElementType::NOTEDOT) {
                        const NoteDot* n1 = static_cast<const NoteDot*>(e1);
                        const NoteDot* n2 = static_cast<const NoteDot*>(e2);
                        if (n1->note() && n1->note()->hidden())
                              return false;
                        else if (n2->note() && n2->note()->hidden())
                              return true;
                        }
                  else if (e1->type() == ElementType::NOTE) {
                        const Note* n1 = static_cast<const Note*>(e1);
                        const Note* n2 = static_cast<const Note*>(e2);
                        if (n1->hidden())
                              return false;
                        else if (n2->hidden())
                              return true;
                        }
                  }
            // different types, or same type but nothing hidden - use track
            return e1->track() <= e2->track();
            }

      // default case, use stacking order
      return e1->z() <= e2->z();
      }

//---------------------------------------------------------
//   elementsNear
//---------------------------------------------------------

QList<Element*> ScoreView::elementsNear(QPointF p)
      {
      QList<Element*> ll;
      Page* page = point2page(p);
      if (!page)
            return ll;

      p       -= page->pos();
      double w = (preferences.getInt(PREF_UI_CANVAS_MISC_SELECTIONPROXIMITY) * .5) / matrix().m11();
      QRectF r(p.x() - w, p.y() - w, 3.0 * w, 3.0 * w);

      QList<Element*> el = page->items(r);
      for (int i = 0; i < MAX_HEADERS; i++)
            if (score()->headerText(i) != nullptr)      // gives the ability to select the header
                  el.push_back(score()->headerText(i));
      for (int i = 0; i < MAX_FOOTERS; i++)
            if (score()->footerText(i) != nullptr)      // gives the ability to select the footer
                  el.push_back(score()->footerText(i));
      for (Element* e : el) {
            e->itemDiscovered = 0;
            if (!e->selectable() || e->isPage())
                  continue;
            if (e->contains(p))
                  ll.append(e);
            }
      int n = ll.size();
      if ((n == 0) || ((n == 1) && (ll[0]->isMeasure()))) {
            //
            // if no relevant element hit, look nearby
            //
            for (Element* e : el) {
                  if (e->isPage() || !e->selectable())
                        continue;
                  if (e->intersects(r))
                        ll.append(e);
                  }
            }
      if (!ll.empty())
            std::sort(ll.begin(), ll.end(), elementLower);
      return ll;
      }

//---------------------------------------------------------
//   elementNear
//---------------------------------------------------------

Element* ScoreView::elementNear(QPointF p)
      {
      QList<Element*> ll = elementsNear(p);
      if (ll.empty()) {
            // qDebug("  nothing found");
            return 0;
            }
#if 0
      qDebug("==");
      for (const Element* e : ll)
            qDebug("  %s selected %d z %d", e->name(), e->selected(), e->z());
#endif
      Element* e = ll.at(0);
      return e;
      }

//---------------------------------------------------------
//   posChanged
//---------------------------------------------------------

void ScoreView::posChanged(POS pos, unsigned tick)
      {
      switch (pos) {
            case POS::CURRENT:
                  // draw playback cursor only in the currently active view
                  if (this != mscore->currentScoreView() && !_moveWhenInactive)
                        return;
                  if (noteEntryMode())
                        moveCursor();     // update input cursor position
                  else
                        moveCursor(Fraction::fromTicks(tick)); // update play position
                  break;
            case POS::LEFT:
                  _curLoopIn->move(_score->pos(POS::LEFT));
                  break;
            case POS::RIGHT:
                  _curLoopOut->move(_score->pos(POS::RIGHT));
                  break;
            }
      }

//---------------------------------------------------------
//   loopToggled
//---------------------------------------------------------

void ScoreView::loopToggled(bool val)
      {
      if (_score->lastMeasure() == 0)
            return;
      if (_score->pos(POS::LEFT).isZero() && _score->pos(POS::RIGHT).isZero())
            _score->setPos(POS::RIGHT, _score->lastMeasure()->endTick());
      _curLoopIn->move(_score->loopInTick());
      _curLoopOut->move(_score->loopOutTick());
      _curLoopIn->setVisible(val);
      _curLoopOut->setVisible(val);
      update();
      }

//---------------------------------------------------------
//   cmdMoveCR
//    swap selected cr with cr to the left or right
//      - not across measure boundaries
//---------------------------------------------------------

void ScoreView::cmdMoveCR(bool left)
      {
      Element* e = _score->getSelectedElement();
      if (e && (e->type() == ElementType::NOTE || e->type() == ElementType::REST)) {
            if (e->type() == ElementType::NOTE)
                  e = e->parent();
            QList<ChordRest*> crl;
            if (e->links()) {
                  for (ScoreElement* cr : *e->links())
                        crl.append(static_cast<ChordRest*>(cr));
                  }
            else
                  crl.append(static_cast<ChordRest*>(e));

            bool cmdActive = false;
            for (ChordRest* cr1 : crl) {
                  if (cr1->type() == ElementType::REST) {
                        Rest* r = static_cast<Rest*>(cr1);
                        if (r->measure() && r->measure()->isMMRest())
                              break;
                        }
                  ChordRest* cr2 = left ? prevChordRest(cr1) : nextChordRest(cr1);
                  // ensures cr1 is the left chord, useful in SwapCR::flip()
                  if (left)
                        std::swap(cr1, cr2);
                  if (cr1 && cr2 && cr1->measure() == cr2->measure() && !cr1->tuplet() && !cr2->tuplet()
                     && cr1->durationType() == cr2->durationType() && cr1->ticks() == cr2->ticks()
                     // if two chords belong to different two-note tremolos, abort
                     && !(cr1->isChord() && toChord(cr1)->tremolo() && toChord(cr1)->tremolo()->twoNotes()
                        && cr2->isChord() && toChord(cr2)->tremolo() && toChord(cr2)->tremolo()->twoNotes()
                        && toChord(cr1)->tremolo() != toChord(cr2)->tremolo())) {
                        if (!cmdActive) {
                              _score->startCmd();
                              cmdActive = true;
                              }
                        _score->undo(new SwapCR(cr1, cr2));
                        }
                  }
            if (cmdActive)
                  _score->endCmd();
            }
      }

//---------------------------------------------------------
//   cmdAddRemoveBreaks
///   add or remove line breaks within a range selection
///   or, if nothing is selected, the entire score
//---------------------------------------------------------

void ScoreView::cmdAddRemoveBreaks()
      {
      bool noSelection = !_score->selection().isRange();

      if (noSelection)
            _score->cmdSelectAll();
      else if (!_score->selection().isRange())
            return;

      BreaksDialog bd;
      if (!bd.exec())
            return;

      int interval = bd.remove || bd.lock ? 0 : bd.interval;

      _score->addRemoveBreaks(interval, bd.lock);

      if (noSelection)
             _score->deselectAll();
      }

//---------------------------------------------------------
//   cmdCopyLyricsToClipboard
///   Copy the score lyrics into clipboard
//---------------------------------------------------------

void ScoreView::cmdCopyLyricsToClipboard()
      {
      QApplication::clipboard()->setText(_score->extractLyrics());
      }

//---------------------------------------------------------
//   updateContinuousPanel
//   slot triggered when moving around the score to keep the panel visible
//---------------------------------------------------------

void ScoreView::updateContinuousPanel()
      {
      if (_score->layoutMode() == LayoutMode::LINE)
            update();
      }

//---------------------------------------------------------
//   updateShadowNotes
//---------------------------------------------------------

void ScoreView::updateShadowNotes()
      {
      if (shadowNote->visible())
            setShadowNote(shadowNote->pos());
      }

//---------------------------------------------------------
//   getEditElement
//---------------------------------------------------------

Element* ScoreView::getEditElement()
      {
      return editData.element;
      }

//---------------------------------------------------------
//   onElementDestruction
//---------------------------------------------------------

void ScoreView::onElementDestruction(Element* e)
      {
      if (editData.element == e) {
            editData.element = nullptr;
            if (editMode())
                  changeState(ViewState::NORMAL);
            }
      }

//---------------------------------------------------------
//   startNoteEntryMode
//---------------------------------------------------------

void ScoreView::startNoteEntryMode()
      {
      changeState(ViewState::NOTE_ENTRY);
      }

//---------------------------------------------------------
//   fotoMode
//---------------------------------------------------------

bool ScoreView::fotoMode() const
      {
      switch (state) {
            case ViewState::NORMAL:
            case ViewState::DRAG:
            case ViewState::DRAG_OBJECT:
            case ViewState::EDIT:
            case ViewState::DRAG_EDIT:
            case ViewState::LASSO:
            case ViewState::NOTE_ENTRY:
            case ViewState::PLAY:
            case ViewState::ENTRY_PLAY:
                  break;

            case ViewState::FOTO:
            case ViewState::FOTO_DRAG:
            case ViewState::FOTO_DRAG_EDIT:
            case ViewState::FOTO_DRAG_OBJECT:
            case ViewState::FOTO_LASSO:
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   setEditElement
//---------------------------------------------------------

void ScoreView::setEditElement(Element* e)
      {
      if (editData.element == e)
            return;

      const bool normalState = (state == ViewState::NORMAL);

      if (normalState && editData.element && editData.element->normalModeEditBehavior() == Element::EditBehavior::Edit)
            endEdit();

      editData.element = e;

      if (normalState && e && e->normalModeEditBehavior() == Element::EditBehavior::Edit)
            startEdit(/* editMode */ false);
      else if (editData.grips) {
            editData.grips = 0;
            editData.curGrip = Grip::NO_GRIP;
            }
      }

//---------------------------------------------------------
//   updateEditElement
//---------------------------------------------------------

void ScoreView::updateEditElement()
      {
      if (state != ViewState::NORMAL)
            return;

      if (!_score) {
            setEditElement(nullptr);
            return;
            }

      const Selection& sel = _score->selection();

      switch (sel.state()) {
            case SelState::NONE:
            case SelState::RANGE:
                  setEditElement(nullptr);
                  break;
            case SelState::LIST:
                  if (Element* e = sel.element()) {
                        if (editData.element == e)
                              updateGrips();
                        else
                              setEditElement(e);
                        }
                  else {
                        setEditElement(nullptr);
                        }
                  break;
            }
      }

//---------------------------------------------------------
//   visibleElementInScore
//---------------------------------------------------------

static const Element* visibleElementInScore(const Element* orig, const Score* s)
      {
      if (!orig)
            return nullptr;
      if (orig->score() == s && orig->bbox().isValid())
            return orig;

      for (const ScoreElement* se : orig->linkList()) {
            const Element* e = toElement(se);
            if (e->score() == s && e->bbox().isValid()) // bbox check to ensure the element is indeed visible
                  return e;
            }

      return nullptr;
      }

//---------------------------------------------------------
//   needViewportMove
//---------------------------------------------------------

static bool needViewportMove(Score* cs, ScoreView* cv)
      {
      if (!cs || !cv)
            return false;

      const CmdState& state = cs->cmdState();

      if (state.startTick() < Fraction(0, 1))
            return false;

      const QRectF viewport = cv->canvasViewport(); // TODO: margins for intersection check?

      const Element* editElement = visibleElementInScore(state.element(), cs);
      if (editElement && editElement->bbox().isValid() && !editElement->isSpanner())
            return !viewport.intersects(editElement->canvasBoundingRect());

      if (state.startTick().isZero() && state.endTick() == cs->endTick())
            return false; // TODO: adjust staff perhaps?

      // Is any part of the range visible?
      Measure* mStart = cs->tick2measureMM(state.startTick());
      Measure* mEnd = cs->tick2measureMM(state.endTick());
      mEnd = mEnd ? mEnd->nextMeasureMM() : nullptr;

      const bool isExcerpt = !cs->isMaster();
      const bool csStaves = (isExcerpt || (state.endStaff() < 0) || (state.endStaff() >= cs->nstaves()));
      const int startStaff = csStaves ? 0 : state.startStaff();
      const int endStaff = csStaves ? (cs->nstaves() - 1) : state.endStaff();

      for (Measure* m = mStart; m && m != mEnd; m = m->nextMeasureMM()) {
            for (int st = startStaff; st <= endStaff; ++st) {
                  const StaffLines* l = m->staffLines(st);
                  if (l) {
                        const QRectF r = l->bbox().translated(l->canvasPos());
                        if (viewport.intersects(r))
                              return false;
                        }
                  }
            }

      // maybe viewport is at least near the desired position?
      const QPointF p = viewport.center();
      int staffIdx = 0;
      const Measure* m = cs->pos2measure(p, &staffIdx, nullptr, nullptr, nullptr);
      if (m && m->tick() >= state.startTick() && m->tick() <= state.endTick())
            return false;

      return true;
      }

//---------------------------------------------------------
//   moveViewportToLastEdit
//---------------------------------------------------------

void ScoreView::moveViewportToLastEdit()
      {
      if (_blockShowEdit)
            return;

      Score* sc = score();

      if (!needViewportMove(sc, this))
            return;

      const CmdState& st = sc->cmdState();
      const Element* editElement = visibleElementInScore(st.element(), sc);

      const MeasureBase* mb = nullptr;
      if (editElement) {
            if (editElement->isSpannerSegment()) {
                  const SpannerSegment* s = toSpannerSegment(editElement);
                  Fraction tick = s->tick();
                  if (System* sys = s->system()) {
                        Measure* fm = sys->firstMeasure();
                        if (fm)
                              tick = std::max(tick, fm->tick());
                        }
                  mb = sc->tick2measureMM(tick);
                  }
            else {
                  mb = editElement->findMeasureBase();
                  }
            }
      if (!mb)
            mb = sc->tick2measureMM(st.startTick());
      if (!mb)
            return;

      const Element* viewportElement = (editElement && editElement->bbox().isValid() && !mb->isMeasure()) ? editElement : mb;

      const int staff = sc->isMaster() && mb->isMeasure() ? st.startStaff() : -1; // TODO: choose the closest staff to the current viewport?
      adjustCanvasPosition(viewportElement, /* playback */ false, staff);
      }

//---------------------------------------------------------
//   loadFromPreferences
//---------------------------------------------------------

void SmoothPanSettings::loadFromPreferences()
      {
      enabled = preferences.getBool(PREF_PAN_SMOOTHLY_ENABLED);
      controlModifierBase = preferences.getDouble(PREF_PAN_MODIFIER_BASE);
      if(mscore->currentScoreView() != nullptr)
            mscore->currentScoreView()->_controlModifier = controlModifierBase;
      controlModifierSteps = preferences.getDouble(PREF_PAN_MODIFIER_STEP);
      minContinuousModifier = preferences.getDouble(PREF_PAN_MODIFIER_MIN);
      maxContinuousModifier = preferences.getDouble(PREF_PAN_MODIFIER_MAX);
//      leftDistance = preferences.getDouble(PREF_PAN_DISTANCE_LEFT);
//      leftDistance1 = preferences.getDouble(PREF_PAN_DISTANCE_LEFT1);
//      leftDistance2 = preferences.getDouble(PREF_PAN_DISTANCE_LEFT2);
//      leftDistance3 = preferences.getDouble(PREF_PAN_DISTANCE_LEFT3);
//      leftMod1 = preferences.getDouble(PREF_PAN_MODIFIER_LEFT1);
//      leftMod2 = preferences.getDouble(PREF_PAN_MODIFIER_LEFT2);
//      leftMod3 = preferences.getDouble(PREF_PAN_MODIFIER_LEFT3);
//      rightDistance = preferences.getDouble(PREF_PAN_DISTANCE_RIGHT);
//      rightDistance1 = preferences.getDouble(PREF_PAN_DISTANCE_RIGHT1);
//      rightDistance2 = preferences.getDouble(PREF_PAN_DISTANCE_RIGHT2);
//      rightDistance3 = preferences.getDouble(PREF_PAN_DISTANCE_RIGHT3);
//      rightMod1 = preferences.getDouble(PREF_PAN_MODIFIER_RIGHT1);
//      rightMod2 = preferences.getDouble(PREF_PAN_MODIFIER_RIGHT2);
//      rightMod3 = preferences.getDouble(PREF_PAN_MODIFIER_RIGHT3);
//      normalWeight = preferences.getDouble(PREF_PAN_WEIGHT_NORMAL);
//      smartWeight = preferences.getDouble(PREF_PAN_WEIGHT_SMART);
//      advancedWeighting = preferences.getBool(PREF_PAN_WEIGHT_ADVANCED);
//      cursorTimerDuration = preferences.getInt(PREF_PAN_SMART_TIMER_DURATION);
      controlCursorScreenPos = preferences.getDouble(PREF_PAN_CURSOR_POS);
      teleportLeftEnabled = preferences.getBool(PREF_PAN_TELEPORT_LEFT);
      teleportRightEnabled = preferences.getBool(PREF_PAN_TELEPORT_RIGHT);
      }
} // namespace Ms
