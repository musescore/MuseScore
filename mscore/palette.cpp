//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "palette.h"
#include "musescore.h"
#include "libmscore/element.h"
#include "libmscore/style.h"
#include "globals.h"
#include "libmscore/sym.h"
#include "libmscore/symbol.h"
#include "libmscore/score.h"
#include "libmscore/image.h"
#include "libmscore/xml.h"
#include "scoreview.h"
#include "libmscore/note.h"
#include "libmscore/chord.h"
#include "libmscore/clef.h"
#include "libmscore/segment.h"
#include "libmscore/measure.h"
#include "libmscore/staff.h"
#include "libmscore/system.h"
#include "libmscore/page.h"
#include "libmscore/keysig.h"
#include "libmscore/timesig.h"
#include "preferences.h"
#include "seq.h"
#include "libmscore/part.h"
#include "libmscore/textline.h"
#include "libmscore/measure.h"
#include "libmscore/icon.h"
#include "libmscore/mscore.h"
#include "libmscore/imageStore.h"
#include "thirdparty/qzip/qzipreader_p.h"
#include "thirdparty/qzip/qzipwriter_p.h"
#include "libmscore/slur.h"
#include "paletteBoxButton.h"
#include "palettebox.h"
#include "shortcut.h"

namespace Ms {

PaletteCell::~PaletteCell()
      {
      delete element;
      }

//---------------------------------------------------------
//   needsStaff
//    should a staff been drawn if e is used as icon in
//    a palette
//---------------------------------------------------------

static bool needsStaff(Element* e)
      {
      if (e == 0)
            return false;
      switch(e->type()) {
            case ElementType::CHORD:
            case ElementType::BAR_LINE:
            case ElementType::CLEF:
            case ElementType::KEYSIG:
            case ElementType::TIMESIG:
            case ElementType::REST:
            case ElementType::BAGPIPE_EMBELLISHMENT:
                  return true;
            default:
                  return false;
            }
      }

//---------------------------------------------------------
//   Palette
//---------------------------------------------------------

Palette::Palette(QWidget* parent)
   : QWidget(parent)
      {
      extraMag      = 1.0;
      currentIdx    = -1;
      dragIdx       = -1;
      selectedIdx   = -1;
      _yOffset      = 0.0;
      setGrid(50, 60);
      _drawGrid     = false;
      _selectable   = false;
      setMouseTracking(true);
      setReadOnly(false);
      setSystemPalette(false);
      _moreElements = false;
      setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Ignored);
      setObjectName("palette-cells");
      }

Palette::~Palette()
      {
      for (PaletteCell* cell : cells)
            delete cell;
      }

//---------------------------------------------------------
//   resizeEvent
//---------------------------------------------------------

void Palette::resizeEvent(QResizeEvent* e)
      {
      setFixedHeight(heightForWidth(e->size().width()));
      }

//---------------------------------------------------------
//   filter
///  return true if all filtered
//---------------------------------------------------------

bool Palette::filter(const QString& text)
      {
      filterActive = false;
      setMouseTracking(true);
      QString t = text.toLower();
      if (t == "") {
            mscore->getPaletteBox()->setKeyboardNavigation(false);
            for (Palette* p : mscore->getPaletteBox()->palettes()) {
                  if (p->getCurrentIdx() != -1) {
                        p->setCurrentIdx(-1);
                        p->update();
                        }
                  }
            }
      bool res = true;
      dragCells.clear();
      // if palette name is searched for, display all elements in the palette
      if (_name.startsWith(t, Qt::CaseInsensitive)) {
            PaletteCell* c  = cells.first();
            for (PaletteCell* cell : cells)
                  dragCells.append(cell);

            bool contains = t.isEmpty() || c;
            if (!contains)
                  filterActive = true;
            if (contains && res)
                  res = false;
            }

      for (PaletteCell* cell : cells) {
            QStringList h = cell->name.toLower().split(" ");
            bool c        = false;
            QStringList n = t.split(" ");
            for (QString hs : h) {
                  for (QString ns : n) {
                        if (!ns.trimmed().isEmpty())
                              c = hs.trimmed().startsWith(ns.trimmed());
                        }
                  if (c)
                        break;
                  }
            if (t.isEmpty() || c)
                  dragCells.append(cell);
            bool contains = t.isEmpty() || c;
            if (!contains)
                  filterActive = true;
            if (contains && res)
                  res = false;
            }
      setFixedHeight(heightForWidth(width()));
      updateGeometry();
      return res;
      }

//---------------------------------------------------------
//   setMoreElements
//---------------------------------------------------------

void Palette::setMoreElements(bool val)
      {
      _moreElements = val;
      if (val && (cells.isEmpty() || cells.back()->tag != "ShowMore")) {
            PaletteCell* cell = new PaletteCell;
            cell->name      = "Show More";
            cell->tag       = "ShowMore";
            cells.append(cell);
            }
      else if (!val && !cells.isEmpty() && (cells.last()->tag == "ShowMore") ) {
            PaletteCell* cell = cells.takeLast();
            delete cell;
            }
      }

//---------------------------------------------------------
//   setSystemPalette
//---------------------------------------------------------

void Palette::setSystemPalette(bool val)
      {
      _systemPalette = val;
      if (val)
            setReadOnly(true);
      }

//---------------------------------------------------------
//   setReadOnly
//---------------------------------------------------------

void Palette::setReadOnly(bool val)
      {
      _readOnly = val;
      setAcceptDrops(!val);
      }

//---------------------------------------------------------
//   setMag
//---------------------------------------------------------

void Palette::setMag(qreal val)
      {
      extraMag = val;
      }

//---------------------------------------------------------
//   contextMenuEvent
//---------------------------------------------------------

void Palette::contextMenuEvent(QContextMenuEvent* event)
      {
      if (!_showContextMenu)
            return;
      int i = idx(event->pos());
      if (i == -1) {
            // palette context menu
            if (!_moreElements)
                  return;
            QMenu menu;
            QAction* moreAction = menu.addAction(tr("More Elements…"));
            moreAction->setEnabled(_moreElements);
            QAction* action = menu.exec(mapToGlobal(event->pos()));
            if (action == moreAction)
                  emit displayMore(_name);
            return;
            }

      QMenu menu;
      QAction* deleteCellAction   = menu.addAction(tr("Delete"));
      QAction* contextAction = menu.addAction(tr("Properties…"));
      deleteCellAction->setEnabled(!_readOnly);
      contextAction->setEnabled(!_readOnly);
      QAction* moreAction    = menu.addAction(tr("More Elements…"));
      moreAction->setEnabled(_moreElements);

      if (filterActive || (cellAt(i) && cellAt(i)->readOnly))
            deleteCellAction->setEnabled(false);

      if (!deleteCellAction->isEnabled() && !contextAction->isEnabled() && !moreAction->isEnabled())
            return;

      const QAction* action = menu.exec(mapToGlobal(event->pos()));

      if (action == deleteCellAction) {
            PaletteCell* cell = cellAt(i);
            if (cell) {
                  int ret = QMessageBox::warning(this, QWidget::tr("Delete palette cell"),
                                                 QWidget::tr("Are you sure you want to delete palette cell \"%1\"?")
                                                 .arg(cell->name), QMessageBox::Yes | QMessageBox::No,
                                                 QMessageBox::Yes);
                  if (ret != QMessageBox::Yes)
                        return;
                  if(cell->tag == "ShowMore")
                        _moreElements = false;
                  delete cell;
                  }
            cells[i] = nullptr;
            emit changed();
            }
      else if (action == contextAction) {
            PaletteCell* c = cellAt(i);
            if (c == 0)
                  return;
            PaletteCellProperties props(c);
            if (props.exec())
                  emit changed();
            }
      else if (moreAction && (action == moreAction))
            emit displayMore(_name);

      bool sizeChanged = false;
      for (int i = 0; i < cells.size(); ++i) {
            if (!cellAt(i)) {
                  cells.removeAt(i);
                  sizeChanged = true;
                  }
            }
      if (sizeChanged) {
            setFixedHeight(heightForWidth(width()));
            updateGeometry();
            }
      update();
      }

//---------------------------------------------------------
//   setGrid
//---------------------------------------------------------

void Palette::setGrid(int hh, int vv)
      {
      hgrid = hh;
      vgrid = vv;
      QSize s(hgrid, vgrid);
      setSizeIncrement(s);
      setBaseSize(s);
      setMinimumSize(s);
      updateGeometry();
      }

//---------------------------------------------------------
//   element
//---------------------------------------------------------

Element* Palette::element(int idx)
      {
      if (idx < size() && cellAt(idx))
            return cellAt(idx)->element;
      else
            return 0;
      }

//---------------------------------------------------------
//   contentsMousePressEvent
//---------------------------------------------------------

void Palette::mousePressEvent(QMouseEvent* ev)
      {
      dragStartPosition = ev->pos();
      dragIdx           = idx(dragStartPosition);

/*
      // Take out of edit mode to prevent crashes when adding
      // elements from palette

      ScoreView* cv = mscore->currentScoreView();
      if (cv && cv->editMode())
            cv->changeState(ViewState::NORMAL);
*/
      if (dragIdx == -1)
            return;
      if (_selectable) {
            if (dragIdx != selectedIdx) {
                  update(idxRect(dragIdx) | idxRect(selectedIdx));
                  selectedIdx = dragIdx;
                  }
            emit boxClicked(dragIdx);
            }
      PaletteCell* cell = cellAt(dragIdx);
      if (cell && (cell->tag == "ShowMore"))
            emit displayMore(_name);
      if (mscore->getPaletteBox()->getKeyboardNavigation()) {
            PaletteBox* pb = mscore->getPaletteBox();
            pb->mousePressEvent(ev, this);
            update();
            }
      }

//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void Palette::mouseMoveEvent(QMouseEvent* ev)
      {
      PaletteBox* pb = mscore->getPaletteBox();
      if (pb->getKeyboardNavigation()) {
            ev->ignore();
            return;
            }
      if ((currentIdx != -1) && (dragIdx == currentIdx) && (ev->buttons() & Qt::LeftButton)
         && (ev->pos() - dragStartPosition).manhattanLength() > QApplication::startDragDistance())
            {
            PaletteCell* cell = cellAt(currentIdx);
            if (cell && cell->element) {
                  QDrag* drag         = new QDrag(this);
                  QMimeData* mimeData = new QMimeData;
                  Element* el         = cell->element;

                  mimeData->setData(mimeSymbolFormat, el->mimeData(QPointF()));
                  drag->setMimeData(mimeData);

                  drag->setPixmap(pixmap(currentIdx));
                  QRect r = idxRect(currentIdx);

                  QPoint o(dragStartPosition - r.topLeft());
                  drag->setHotSpot(o);

                  Qt::DropActions da;
                  if (!(_readOnly || filterActive) && (ev->modifiers() & Qt::ShiftModifier)) {
                        dragCells = cells;      // backup
                        da = Qt::MoveAction;
                        }
                  else
                        da = Qt::CopyAction;
                  Qt::DropAction a = drag->exec(da);
                  if (da == Qt::MoveAction && a != da)
                        cells = dragCells;      // restore on a failed move action
                  update();
                  }
            }
      else {
            currentIdx = idx(ev->pos());
            if (currentIdx != -1 && cellAt(currentIdx) == 0)
                  currentIdx = -1;
            update();
            }
      }

//---------------------------------------------------------
//   applyDrop
//---------------------------------------------------------

static void applyDrop(Score* score, ScoreView* viewer, Element* target, Element* e, Qt::KeyboardModifiers modifiers, QPointF pt = QPointF())
      {
      EditData& dropData = viewer->getEditData();
      dropData.pos         = pt.isNull() ? target->pagePos() : pt;
      dropData.dragOffset  = QPointF();
      dropData.modifiers   = modifiers;
      dropData.dropElement = e;

      if (target->acceptDrop(dropData)) {
            // use same code path as drag&drop

            QByteArray a = e->mimeData(QPointF());
//printf("<<%s>>\n", a.data());

            XmlReader n(a);
            Fraction duration;  // dummy
            QPointF dragOffset;
            ElementType type = Element::readType(n, &dragOffset, &duration);
            dropData.dropElement = Element::create(type, score);

            dropData.dropElement->read(n);
            dropData.dropElement->styleChanged();   // update to local style

            Element* el = target->drop(dropData);
            if (el)
                  score->select(el, SelectType::SINGLE, 0);
            dropData.dropElement = 0;
            }
      }

//---------------------------------------------------------
//   applyPaletteElement
//---------------------------------------------------------

void Palette::applyPaletteElement(PaletteCell* cell, Qt::KeyboardModifiers modifiers)
      {
      Score* score = mscore->currentScore();
      if (score == 0)
            return;
      const Selection sel = score->selection(); // make a copy of selection state before applying the operation.
      if (sel.isNone())
            return;

      Element* element = 0;
      if (cell)
            element = cell->element;
      if (element == 0)
            return;

      ScoreView* viewer = mscore->currentScoreView();
      if (viewer->mscoreState() != STATE_EDIT
         && viewer->mscoreState() != STATE_LYRICS_EDIT
         && viewer->mscoreState() != STATE_HARMONY_FIGBASS_EDIT) { // Already in startCmd in this case
            score->startCmd();
            }
      if (sel.isList()) {
            ChordRest* cr1 = sel.firstChordRest();
            ChordRest* cr2 = sel.lastChordRest();
            bool addSingle = false;       // add a single line only
            if (cr1 && cr2 == cr1) {
                  // one chordrest selected, ok to add line
                  addSingle = true;
                  }
            else if (sel.elements().size() == 2 && cr1 && cr2 && cr1 != cr2) {
                  // two chordrests selected
                  // must be on same staff in order to add line, except for slur
                  if (element->isSlur() || cr1->staffIdx() == cr2->staffIdx())
                        addSingle = true;
                  }
            if (viewer->mscoreState() == STATE_NOTE_ENTRY_STAFF_DRUM && element->isChord()) {
                  // use input position rather than selection if possible
                  Element* e = score->inputState().cr();
                  if (!e)
                        e = sel.elements().first();
                  if (e) {
                        // get note if selection was full chord
                        if (e->isChord())
                              e = toChord(e)->upNote();
                        // use voice of element being added to (otherwise we can might corrupt the measure)
                        element->setTrack(e->voice());
                        applyDrop(score, viewer, e, element, modifiers);
                        // continue in same track
                        score->inputState().setTrack(e->track());
                        }
                  else
                        qDebug("nowhere to place drum note");
                  }
            else if (element->isLayoutBreak()) {
                  LayoutBreak* breakElement = toLayoutBreak(element);
                  score->cmdToggleLayoutBreak(breakElement->layoutBreakType());
                  }
            else if (element->isSlur() && addSingle) {
                  viewer->addSlur();
                  }
            else if (element->isSLine() && !element->isGlissando() && addSingle) {
                  Segment* startSegment = cr1->segment();
                  Segment* endSegment = cr2->segment();
                  if (element->type() == ElementType::PEDAL && cr2 != cr1)
                        endSegment = endSegment->nextCR(cr2->track());
                  // TODO - handle cross-voice selections
                  int idx = cr1->staffIdx();

                  QByteArray a = element->mimeData(QPointF());
//printf("<<%s>>\n", a.data());
                  XmlReader e(a);
                  Fraction duration;  // dummy
                  QPointF dragOffset;
                  ElementType type = Element::readType(e, &dragOffset, &duration);
                  Spanner* spanner = static_cast<Spanner*>(Element::create(type, score));
                  spanner->read(e);
                  spanner->styleChanged();
                  score->cmdAddSpanner(spanner, idx, startSegment, endSegment);
                  }
            else {
                  for (Element* e : sel.elements())
                        applyDrop(score, viewer, e, element, modifiers);
                  }
            }
      else if (sel.isRange()) {
            if (element->type() == ElementType::BAR_LINE
                || element->type() == ElementType::MARKER
                || element->type() == ElementType::JUMP
                || element->type() == ElementType::SPACER
                || element->type() == ElementType::VBOX
                || element->type() == ElementType::HBOX
                || element->type() == ElementType::TBOX
                || element->type() == ElementType::MEASURE
                || element->type() == ElementType::BRACKET
                || (element->type() == ElementType::ICON
                    && (toIcon(element)->iconType() == IconType::VFRAME
                        || toIcon(element)->iconType() == IconType::HFRAME
                        || toIcon(element)->iconType() == IconType::TFRAME
                        || toIcon(element)->iconType() == IconType::MEASURE
                        || toIcon(element)->iconType() == IconType::BRACKETS))) {
                  Measure* last = sel.endSegment() ? sel.endSegment()->measure() : nullptr;
                  for (Measure* m = sel.startSegment()->measure(); m; m = m->nextMeasureMM()) {
                        QRectF r = m->staffabbox(sel.staffStart());
                        QPointF pt(r.x() + r.width() * .5, r.y() + r.height() * .5);
                        pt += m->system()->page()->pos();
                        applyDrop(score, viewer, m, element, modifiers, pt);
                        if (m == last)
                              break;
                        }
                  }
            else if (element->type() == ElementType::LAYOUT_BREAK) {
                  LayoutBreak* breakElement = static_cast<LayoutBreak*>(element);
                  score->cmdToggleLayoutBreak(breakElement->layoutBreakType());
                  }
            else if (element->isClef() || element->isKeySig() || element->isTimeSig()) {
                  Measure* m1 = sel.startSegment()->measure();
                  Measure* m2 = sel.endSegment() ? sel.endSegment()->measure() : nullptr;
                  if (m2 == m1 && sel.startSegment()->rtick() == 0)
                        m2 = nullptr;     // don't restore original if one full measure selected
                  else if (m2)
                        m2 = m2->nextMeasureMM();
                  // for clefs, apply to each staff separately
                  // otherwise just apply to top staff
                  int staffIdx1 = sel.staffStart();
                  int staffIdx2 = element->type() == ElementType::CLEF ? sel.staffEnd() : staffIdx1 + 1;
                  for (int i = staffIdx1; i < staffIdx2; ++i) {
                        // for clefs, use mid-measure changes if appropriate
                        Element* e1 = nullptr;
                        Element* e2 = nullptr;
                        // use mid-measure clef changes as appropriate
                        if (element->type() == ElementType::CLEF) {
                              if (sel.startSegment()->segmentType() == SegmentType::ChordRest && sel.startSegment()->rtick() != 0) {
                                    ChordRest* cr = static_cast<ChordRest*>(sel.startSegment()->nextChordRest(i * VOICES));
                                    if (cr && cr->isChord())
                                          e1 = static_cast<Chord*>(cr)->upNote();
                                    else
                                          e1 = cr;
                                    }
                              if (sel.endSegment() && sel.endSegment()->segmentType() == SegmentType::ChordRest) {
                                    ChordRest* cr = static_cast<ChordRest*>(sel.endSegment()->nextChordRest(i * VOICES));
                                    if (cr && cr->isChord())
                                          e2 = static_cast<Chord*>(cr)->upNote();
                                    else
                                          e2 = cr;
                                    }
                              }
                        if (m2 || e2) {
                              // restore original clef/keysig/timesig
                              Staff* staff = score->staff(i);
                              int tick1 = sel.startSegment()->tick();
                              Element* oelement = nullptr;
                              switch (element->type()) {
                                    case ElementType::CLEF:
                                          {
                                          Clef* oclef = new Clef(score);
                                          oclef->setClefType(staff->clef(tick1));
                                          oelement = oclef;
                                          break;
                                          }
                                    case ElementType::KEYSIG:
                                          {
                                          KeySig* okeysig = new KeySig(score);
                                          okeysig->setKeySigEvent(staff->keySigEvent(tick1));
                                          if (!score->styleB(Sid::concertPitch) && !okeysig->isCustom() && !okeysig->isAtonal()) {
                                                Interval v = staff->part()->instrument(tick1)->transpose();
                                                if (!v.isZero()) {
                                                      Key k = okeysig->key();
                                                      okeysig->setKey(transposeKey(k, v));
                                                      }
                                                }
                                          oelement = okeysig;
                                          break;
                                          }
                                    case ElementType::TIMESIG:
                                          {
                                          TimeSig* otimesig = new TimeSig(score);
                                          otimesig->setFrom(staff->timeSig(tick1));
                                          oelement = otimesig;
                                          break;
                                          }
                                    default:
                                          break;
                                    }
                              if (oelement) {
                                    if (e2) {
                                          applyDrop(score, viewer, e2, oelement, modifiers);
                                          }
                                    else {
                                          QRectF r = m2->staffabbox(i);
                                          QPointF pt(r.x() + r.width() * .5, r.y() + r.height() * .5);
                                          pt += m2->system()->page()->pos();
                                          applyDrop(score, viewer, m2, oelement, modifiers, pt);
                                          }
                                    delete oelement;
                                    }
                              }
                        // apply new clef/keysig/timesig
                        if (e1) {
                              applyDrop(score, viewer, e1, element, modifiers);
                              }
                        else {
                              QRectF r = m1->staffabbox(i);
                              QPointF pt(r.x() + r.width() * .5, r.y() + r.height() * .5);
                              pt += m1->system()->page()->pos();
                              applyDrop(score, viewer, m1, element, modifiers, pt);
                              }
                        }
                  }
            else if (element->isSlur()) {
                  viewer->addSlur();
                  }
            else if (element->isSLine() && element->type() != ElementType::GLISSANDO) {
                  Segment* startSegment = sel.startSegment();
                  Segment* endSegment = sel.endSegment();
                  int endStaff = sel.staffEnd();
                  for (int i = sel.staffStart(); i < endStaff; ++i) {
                        Spanner* spanner = static_cast<Spanner*>(element->clone());
                        spanner->setScore(score);
                        score->cmdAddSpanner(spanner, i, startSegment, endSegment);
                        }
                  }
            else {
                  int track1 = sel.staffStart() * VOICES;
                  int track2 = sel.staffEnd() * VOICES;
                  Segment* startSegment = sel.startSegment();
                  Segment* endSegment = sel.endSegment(); //keep it, it could change during the loop
                  for (Segment* s = startSegment; s && s != endSegment; s = s->next1()) {
                        for (int track = track1; track < track2; ++track) {
                              Element* e = s->element(track);
                              if (e == 0 || !score->selectionFilter().canSelect(e) || !score->selectionFilter().canSelectVoice(track))
                                    continue;
                              if (e->isChord()) {
                                    Chord* chord = toChord(e);
                                    for (Note* n : chord->notes())
                                          applyDrop(score, viewer, n, element, modifiers);
                                    }
                              else {
                                    // do not apply articulation to barline in a range selection
                                    if (!e->isBarLine() || !element->isArticulation())
                                          applyDrop(score, viewer, e, element, modifiers);
                                    }
                              }
                        }
                  }
            }
      else
            qDebug("unknown selection state");

      if (viewer->mscoreState() != STATE_EDIT
         && viewer->mscoreState() != STATE_LYRICS_EDIT
         && viewer->mscoreState() != STATE_HARMONY_FIGBASS_EDIT) { //Already in startCmd mode in this case
            score->endCmd();
            if (viewer->mscoreState() == STATE_NOTE_ENTRY_STAFF_DRUM)
                  viewer->moveCursor();
            }
      viewer->setDropTarget(0);
//      mscore->endCmd();
      }

//---------------------------------------------------------
//   keyPressEvent
//---------------------------------------------------------

void PaletteScrollArea::keyPressEvent(QKeyEvent* event)
      {
      QWidget* w = this->widget();
      Palette* p = static_cast<Palette*>(w);
      int pressedKey = event->key();
      switch (pressedKey) {
            case Qt::Key_Right:
            case Qt::Key_Left:
            case Qt::Key_Up:
            case Qt::Key_Down:
                  {
                  int idx = p->getSelectedIdx();
                  if (pressedKey == Qt::Key_Left || pressedKey == Qt::Key_Up)
                        idx--;
                  else
                        idx++;
                  if (idx < 0)
                        idx = p->size() - 1;
                  else if (idx >= p->size())
                        idx = 0;
                  p->setSelected(idx);
                  // Set widget name to name of selected key signature. We could
                  // set the description, but some screen readers ignore it.
                  setAccessibleName(qApp->translate("Palette", p->cellAt(idx)->name.toUtf8()));
                  QAccessibleEvent event(this, QAccessible::NameChanged);
                  QAccessible::updateAccessibility(&event);
                  p->update();
                  break;
                  }
            default:
                  break;
            }
      QScrollArea::keyPressEvent(event);
      }

//---------------------------------------------------------
//   mouseDoubleClickEvent
//---------------------------------------------------------

void Palette::mouseDoubleClickEvent(QMouseEvent* ev)
      {
      if (_disableDoubleClick)
            return;
      int i = idx(ev->pos());
      if (i == -1)
            return;
      Score* score = mscore->currentScore();
      if (score == 0)
            return;
      if (score->selection().isNone())
            return;
      
      // exit edit mode, to allow for palette element to be applied properly
      ScoreView* viewer = mscore->currentScoreView();
      if (viewer && viewer->editMode() && !(viewer->mscoreState() & STATE_ALLTEXTUAL_EDIT))
            viewer->changeState(ViewState::NORMAL);

      applyPaletteElement(cellAt(i), ev->modifiers());
      }

//---------------------------------------------------------
//   idx
//---------------------------------------------------------

int Palette::idx(const QPoint& p) const
      {
      if (columns() == 0)
            return -1;
      int rightBorder = width() % hgrid;
      int hhgrid      = hgrid + (rightBorder / columns());

      int x = p.x();
      int y = p.y();

      int row = y / vgrid;
      int col = x / hhgrid;

      int nc = columns();
      if (col > nc)
            return -1;

      int idx = row * nc + col;
      if (idx < 0 || idx >= size())
            return -1;
      return idx;
      }

//---------------------------------------------------------
//   idx2
//   returns indexes outside of cells.size()
//---------------------------------------------------------

int Palette::idx2(const QPoint& p) const
      {
      if (columns() == 0)
            return -1;
      int rightBorder = width() % hgrid;
      int hhgrid      = hgrid + (rightBorder / columns());

      int x = p.x();
      int y = p.y();

      int row = y / vgrid;
      int col = x / hhgrid;

      int nc = columns();
      if (col > nc)
            return -1;

      int idx = row * nc + col;
      if (idx < 0 || idx > columns()*rows())
            return -1;
      return idx;
      }

//---------------------------------------------------------
//   idxRect
//---------------------------------------------------------

QRect Palette::idxRect(int i) const
      {
      if (i == -1)
            return QRect();
     if (columns() == 0)
            return QRect();

      int rightBorder = width() % hgrid;
      int hhgrid = hgrid + (rightBorder / columns());

      int cc = i % columns();
      int cr = i / columns();
      return QRect(cc * hhgrid, cr * vgrid, hhgrid, vgrid);
      }

//---------------------------------------------------------
//   leaveEvent
//---------------------------------------------------------

void Palette::leaveEvent(QEvent*)
      {
      if (currentIdx != -1) {
            QRect r = idxRect(currentIdx);
            PaletteBox* pb = mscore->getPaletteBox();
            if (!pb->getKeyboardNavigation())
                  currentIdx = -1;
            update(r);
            }
      }

//---------------------------------------------------------
//   nextPaletteElement
//---------------------------------------------------------
void Palette::nextPaletteElement()
      {
      PaletteBox* pb = mscore->getPaletteBox();
      if (!pb->getKeyboardNavigation())
            return;
      int i = currentIdx;
      if (i == -1)
            return;
      i++;
      if (i < size() && cellAt(i))
            currentIdx = i;

      update();
      }

//---------------------------------------------------------
//   prevPaletteElement
//---------------------------------------------------------

void Palette::prevPaletteElement()
      {
      PaletteBox* pb = mscore->getPaletteBox();
      if (!pb->getKeyboardNavigation())
            return;
      int i = currentIdx;
      if (i == -1)
            return;
      i--;
      if (i >= 0 && cellAt(i))
            currentIdx = i;

      update();
      }

//---------------------------------------------------------
//   applyPaletteElement
//---------------------------------------------------------

void Palette::applyPaletteElement()
      {
      Score* score = mscore->currentScore();
      if (score == 0)
            return;
      const Selection& sel = score->selection();
      if (sel.isNone())
            return;
      // apply currently selected palette symbol to selected score elements
      int i = currentIdx;
      if (i < size() && cellAt(i))
            applyPaletteElement(cellAt(i));
      else
            return;
      }

//---------------------------------------------------------
//   append
//    append element to palette
//---------------------------------------------------------

PaletteCell* Palette::append(Element* s, const QString& name, QString tag, qreal mag)
      {
      if (s == 0) {
            cells.append(0);
            return 0;
            }
      PaletteCell* cell = new PaletteCell;
      int idx;
      if (_moreElements) {
            cells.insert(cells.size() - 1, cell);
            idx = cells.size() - 2;
            }
      else {
            cells.append(cell);
            idx = cells.size() - 1;
            }
      PaletteCell* pc = add(idx, s, name, tag, mag);
      setFixedHeight(heightForWidth(width()));
      updateGeometry();
      return pc;
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

PaletteCell* Palette::add(int idx, Element* s, const QString& name, QString tag, qreal mag)
      {
      if (s) {
            s->setPos(0.0, 0.0);
            s->setOffset(QPointF());
            }

      PaletteCell* cell = new PaletteCell;
      if (idx < cells.size()) {
            delete cells[idx];
            }
      else {
            for (int i = cells.size(); i <= idx; ++i)
                  cells.append(0);
            }
      cells[idx]      = cell;
      cell->element   = s;
      cell->name      = name;
      cell->tag       = tag;
      cell->drawStaff = needsStaff(s);
      cell->xoffset   = 0;
      cell->yoffset   = 0;
      cell->mag       = mag;
      cell->readOnly  = false;
      update();
      if (s && s->isIcon()) {
            Icon* icon = toIcon(s);
            connect(getAction(icon->action()), SIGNAL(toggled(bool)), SLOT(actionToggled(bool)));
            }
      updateGeometry();
      return cell;
      }

//---------------------------------------------------------
//   paintPaletteElement
//---------------------------------------------------------

static void paintPaletteElement(void* data, Element* e)
      {
      QPainter* p = static_cast<QPainter*>(data);
      p->save();
      p->translate(e->pos());
      e->draw(p);
      p->restore();
      }

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void Palette::paintEvent(QPaintEvent* /*event*/)
      {
      qreal _spatium = gscore->spatium();
//      qreal mag      = PALETTE_SPATIUM * extraMag * guiScaling / _spatium;
      qreal mag      = PALETTE_SPATIUM * extraMag / _spatium;
      gscore->setSpatium(SPATIUM20);

      QPainter p(this);
      p.setRenderHint(QPainter::Antialiasing, true);

      QColor bgColor(0xf6, 0xf0, 0xda);
      if (preferences.getBool(PREF_UI_CANVAS_FG_USECOLOR))
            bgColor = preferences.getColor(PREF_UI_CANVAS_FG_COLOR);
#if 1
      p.setBrush(bgColor);
      p.drawRoundedRect(0, 0, width(), height(), 2, 2);
#else
      p.fillRect(event->rect(), QColor(0xf6, 0xf0, 0xda));
#endif
      //
      // draw grid
      //
      if (columns() == 0)
            return;
      int rightBorder = width() % hgrid;
      int hhgrid = hgrid + (rightBorder / columns());

      if (_drawGrid) {
            p.setPen(Qt::gray);
            for (int row = 1; row < rows(); ++row) {
                  int x2 = row < rows()-1 ? columns() * hhgrid : width();
                  int y  = row * vgrid;
                  p.drawLine(0, y, x2, y);
                  }
            for (int column = 1; column < columns(); ++column) {
                  int x = hhgrid * column;
                  p.drawLine(x, 0, x, rows() * vgrid);
                  }
            }

      qreal dy = lrint(2 * PALETTE_SPATIUM * extraMag);

      //
      // draw symbols
      //

      // QPen pen(palette().color(QPalette::Normal, QPalette::Text));
      QPen pen(Qt::black);
      pen.setWidthF(MScore::defaultStyle().value(Sid::staffLineWidth).toDouble() * PALETTE_SPATIUM * extraMag);

      for (int idx = 0; idx < ccp()->size(); ++idx) {
            int yoffset  = gscore->spatium() * _yOffset;
            QRect r      = idxRect(idx);
            QRect rShift = r.translated(0, yoffset);
            p.setPen(pen);
            QColor c(MScore::selectColor[0]);
            if (idx == selectedIdx) {
                  c.setAlpha(100);
                  p.fillRect(r, c);
                  }
            else if (idx == currentIdx) {
                  c.setAlpha(50);
                  p.fillRect(r, c);
                  }
            if (ccp()->at(idx) == 0)
                  continue;
            PaletteCell* cc = ccp()->at(idx);      // current cell

            QString tag = cc->tag;
            if (!tag.isEmpty()) {
                  p.setPen(Qt::darkGray);
                  QFont f(p.font());
                  f.setPointSize(12);
                  p.setFont(f);
                  if (tag == "ShowMore")
                        p.drawText(idxRect(idx), Qt::AlignCenter, "???");
                  else
                        p.drawText(rShift, Qt::AlignLeft | Qt::AlignTop, tag);
                  }

            p.setPen(pen);

            Element* el = cc->element;
            if (el == 0)
                  continue;
            bool drawStaff = cc->drawStaff;
            int row    = idx / columns();
            int column = idx % columns();

            qreal cellMag = cc->mag * mag;
            if (el->isIcon()) {
                  toIcon(el)->setExtent((hhgrid < vgrid ? hhgrid : vgrid) - 4);
                  cellMag = 1.0;
                  }
            el->layout();

            if (drawStaff) {
                  qreal y = r.y() + vgrid * .5 - dy + _yOffset * _spatium * cellMag;
                  qreal x = r.x() + 3;
                  qreal w = hhgrid - 6;
                  for (int i = 0; i < 5; ++i) {
                        qreal yy = y + PALETTE_SPATIUM * i * extraMag;
                        p.drawLine(QLineF(x, yy, x + w, yy));
                        }
                  }
            p.save();
            p.scale(cellMag, cellMag);

            double gw = hhgrid / cellMag;
            double gh = vgrid / cellMag;
            double gx = column * gw + cc->xoffset * _spatium;
            double gy = row    * gh + cc->yoffset * _spatium;

            double sw = el->width();
            double sh = el->height();
            double sy;

            if (drawStaff)
                  sy = gy + gh * .5 - 2.0 * _spatium;
            else
                  sy  = gy + (gh - sh) * .5 - el->bbox().y();
            double sx  = gx + (gw - sw) * .5 - el->bbox().x();

            sy += _yOffset * _spatium;

            p.translate(sx, sy);
            cc->x = sx;
            cc->y = sy;

            QColor color;
            if (idx != selectedIdx) {
                  // show voice colors for notes
                  if (el->isChord())
                        color = el->curColor();
                  else
                        color = palette().color(QPalette::Normal, QPalette::Text);
                  }
            else
                  color = palette().color(QPalette::Normal, QPalette::HighlightedText);

            p.setPen(QPen(color));
            el->scanElements(&p, paintPaletteElement);
            p.restore();
            }
      }

//---------------------------------------------------------
//   pixmap
//---------------------------------------------------------

QPixmap Palette::pixmap(int paletteIdx) const
      {
      qreal _spatium = gscore->spatium();
      qreal mag      = PALETTE_SPATIUM * extraMag / _spatium;
      PaletteCell* c = cellAt(paletteIdx);
      if (!c || !c->element)
            return QPixmap();
      qreal cellMag = c->mag * mag;
      Element* e = c->element;
      e->layout();
      QRectF r = e->bbox();
      int w    = lrint(r.width()  * cellMag);
      int h    = lrint(r.height() * cellMag);

      if (w * h == 0) {
            qDebug("zero pixmap %d %d %s", w, h, e->name());
            return QPixmap();
            }

      QPixmap pm(w, h);
      pm.fill(Qt::transparent);
      QPainter p(&pm);
      p.setRenderHint(QPainter::Antialiasing, true);

      if (e->isIcon())
            toIcon(e)->setExtent(w < h ? w : h);
      p.scale(cellMag, cellMag);

      QPointF pos = e->ipos();
      e->setPos(-r.topLeft());

      QColor color;
       // show voice colors for notes
      if (e->isChord()) {
             Chord* chord = toChord(e);
             for (Note* n : chord->notes())
                   n->setSelected(true);
             color = e->curColor();
             }
       else
             color = palette().color(QPalette::Normal, QPalette::Text);

      p.setPen(QPen(color));
      e->scanElements(&p, paintPaletteElement);

      e->setPos(pos);
      return pm;
      }

//---------------------------------------------------------
//   event
//---------------------------------------------------------

bool Palette::event(QEvent* ev)
      {
      PaletteBox* pb = mscore->getPaletteBox();
      // disable mouse hover when keyboard navigation is enabled
      if (filterActive && pb->getKeyboardNavigation() && (ev->type() == QEvent::MouseMove || ev->type() == QEvent::ToolTip
          || ev->type() == QEvent::WindowDeactivate)) {
            return true;
            }
      else if (columns() && (ev->type() == QEvent::ToolTip)) {
            int rightBorder = width() % hgrid;
            int hhgrid = hgrid + (rightBorder / columns());
            QHelpEvent* he = (QHelpEvent*)ev;
            int x = he->pos().x();
            int y = he->pos().y();

            int row = y / vgrid;
            int col = x / hhgrid;

            if (row < 0 || row >= rows())
                  return false;
            if (col < 0 || col >= columns())
                  return false;
            int idx = row * columns() + col;
            if (idx >= cells.size())
                  return false;
            if (cellAt(idx) == 0)
                  return false;
            QToolTip::showText(he->globalPos(),
               qApp->translate("Palette", cellAt(idx)->name.toUtf8()), this);
            return false;
            }
      return QWidget::event(ev);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Palette::write(XmlWriter& xml) const
      {
      xml.stag(QString("Palette name=\"%1\"").arg(XmlWriter::xmlString(_name)));
      xml.tag("gridWidth", hgrid);
      xml.tag("gridHeight", vgrid);
      xml.tag("mag", extraMag);
      if (_drawGrid)
            xml.tag("grid", _drawGrid);

      xml.tag("moreElements", _moreElements);
      if (_yOffset != 0.0)
            xml.tag("yoffset", _yOffset);

      int n = cells.size();
      for (int i = 0; i < n; ++i) {
            if (cells[i] && cells[i]->tag == "ShowMore")
                  continue;
            if (cells[i] == 0 || cells[i]->element == 0) {
                  xml.tagE("Cell");
                  continue;
                  }
            if (!cells[i]->name.isEmpty())
                  xml.stag(QString("Cell name=\"%1\"").arg(XmlWriter::xmlString(cells[i]->name)));
            else
                  xml.stag("Cell");
            if (cells[i]->drawStaff)
                  xml.tag("staff", cells[i]->drawStaff);
            if (cells[i]->xoffset)
                  xml.tag("xoffset", cells[i]->xoffset);
            if (cells[i]->yoffset)
                  xml.tag("yoffset", cells[i]->yoffset);
            if (!cells[i]->tag.isEmpty())
                  xml.tag("tag", cells[i]->tag);
            if (cells[i]->mag != 1.0)
                  xml.tag("mag", cells[i]->mag);
            cells[i]->element->write(xml);
            xml.etag();
            }
      xml.etag();
      }

//---------------------------------------------------------
//   read
//    return false on error
//---------------------------------------------------------

bool Palette::read(QFile* qf)
      {
      XmlReader e(qf);
      while (e.readNextStartElement()) {
            if (e.name() == "museScore") {
                  QString version = e.attribute("version");
                  QStringList sl = version.split('.');
                  int versionId = sl[0].toInt() * 100 + sl[1].toInt();
                  gscore->setMscVersion(versionId);

                  while (e.readNextStartElement()) {
                        const QStringRef& tag(e.name());
                        if (tag == "Palette") {
                              QString name = e.attribute("name");
                              setName(name);
                              read(e);
                              }
                        else
                              e.unknown();
                        }
                  }
            }
      return true;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

bool Palette::read(const QString& p)
      {
      QString path(p);
      if (!path.endsWith(".mpal"))
            path += ".mpal";

      MQZipReader f(path);
      if (!f.exists()) {
            qDebug("palette <%s> not found", qPrintable(path));
            return false;
            }
      clear();

      QByteArray ba = f.fileData("META-INF/container.xml");

      XmlReader e(ba);
      // extract first rootfile
      QString rootfile = "";
      QList<QString> images;
      while (e.readNextStartElement()) {
            if (e.name() != "container") {
                  e.unknown();
                  break;;
                  }
            while (e.readNextStartElement()) {
                  if (e.name() != "rootfiles") {
                        e.unknown();
                        break;
                        }
                  while (e.readNextStartElement()) {
                        const QStringRef& tag(e.name());

                        if (tag == "rootfile") {
                              if (rootfile.isEmpty())
                                    rootfile = e.attribute("full-path");
                              e.readNext();
                              }
                        else if (tag == "file")
                              images.append(e.readElementText());
                        else
                              e.unknown();
                        }
                  }
            }
      //
      // load images
      //
      foreach(const QString& s, images)
            imageStore.add(s, f.fileData(s));

      if (rootfile.isEmpty()) {
            qDebug("can't find rootfile in: %s", qPrintable(path));
            return false;
            }

      ba = f.fileData(rootfile);
      e.clear();
      e.addData(ba);
      while (e.readNextStartElement()) {
            if (e.name() == "museScore") {
                  QString version = e.attribute("version");
                  QStringList sl = version.split('.');
                  int versionId = sl[0].toInt() * 100 + sl[1].toInt();
                  gscore->setMscVersion(versionId);

                  while (e.readNextStartElement()) {
                        if (e.name() == "Palette") {
                              QString name = e.attribute("name");
                              setName(name);
                              read(e);
                              }
                        else
                              e.unknown();
                        }
                  }
            else
                  e.unknown();
            }
      return true;
      }

//---------------------------------------------------------
//   writeFailed
//---------------------------------------------------------

static void writeFailed(const QString& path)
      {
      QString s = qApp->translate("Palette", "Writing Palette File\n%1\nfailed: ").arg(path); // reason?
      QMessageBox::critical(mscore, qApp->translate("Palette", "Writing Palette File"), s);
      }

//---------------------------------------------------------
//   write
//    write as compressed zip file and include
//    images as needed
//---------------------------------------------------------

void Palette::write(const QString& p)
      {
      QSet<ImageStoreItem*> images;
      int n = cells.size();
      for (int i = 0; i < n; ++i) {
            if (cells[i] == 0 || cells[i]->element == 0 || cells[i]->element->type() != ElementType::IMAGE)
                  continue;
            images.insert(static_cast<Image*>(cells[i]->element)->storeItem());
            }

      QString path(p);
      if (!path.endsWith(".mpal"))
            path += ".mpal";

      MQZipWriter f(path);
      // f.setCompressionPolicy(QZipWriter::NeverCompress);
      f.setCreationPermissions(
         QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner
         | QFile::ReadUser | QFile::WriteUser | QFile::ExeUser
         | QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup
         | QFile::ReadOther | QFile::WriteOther | QFile::ExeOther);

      if (f.status() != MQZipWriter::NoError) {
            writeFailed(path);
            return;
            }
      QBuffer cbuf;
      cbuf.open(QIODevice::ReadWrite);
      XmlWriter xml(gscore, &cbuf);
      xml.header();
      xml.stag("container");
      xml.stag("rootfiles");
      xml.stag(QString("rootfile full-path=\"%1\"").arg(XmlWriter::xmlString("palette.xml")));
      xml.etag();
      foreach (ImageStoreItem* ip, images) {
            QString ipath = QString("Pictures/") + ip->hashName();
            xml.tag("file", ipath);
            }
      xml.etag();
      xml.etag();
      cbuf.seek(0);
      //f.addDirectory("META-INF");
      //f.addDirectory("Pictures");
      f.addFile("META-INF/container.xml", cbuf.data());

      // save images
      foreach(ImageStoreItem* ip, images) {
            QString ipath = QString("Pictures/") + ip->hashName();
            f.addFile(ipath, ip->buffer());
            }
      {
      QBuffer cbuf1;
      cbuf1.open(QIODevice::ReadWrite);
      XmlWriter xml1(gscore, &cbuf1);
      xml1.header();
      xml1.stag("museScore version=\"" MSC_VERSION "\"");
      write(xml1);
      xml1.etag();
      cbuf1.close();
      f.addFile("palette.xml", cbuf1.data());
      }
      f.close();
      if (f.status() != MQZipWriter::NoError)
            writeFailed(path);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Palette::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& t(e.name());
            if (t == "gridWidth")
                  hgrid = e.readDouble();
            else if (t == "gridHeight")
                  vgrid = e.readDouble();
            else if (t == "mag")
                  extraMag = e.readDouble();
            else if (t == "grid")
                  _drawGrid = e.readInt();
            else if (t == "moreElements")
                  setMoreElements(e.readInt());
            else if (t == "yoffset")
                  _yOffset = e.readDouble();
            else if (t == "drumPalette")      // obsolete
                  e.skipCurrentElement();
            else if (t == "Cell") {
                  PaletteCell* cell = new PaletteCell;
                  cell->name = e.attribute("name");
                  bool add = true;
                  while (e.readNextStartElement()) {
                        const QStringRef& t1(e.name());
                        if (t1 == "staff")
                              cell->drawStaff = e.readInt();
                        else if (t1 == "xoffset")
                              cell->xoffset = e.readDouble();
                        else if (t1 == "yoffset")
                              cell->yoffset = e.readDouble();
                        else if (t1 == "mag")
                              cell->mag = e.readDouble();
                        else if (t1 == "tag")
                              cell->tag = e.readElementText();
                        else {
                              cell->element = Element::name2Element(t1, gscore);
                              if (cell->element == 0) {
                                    e.unknown();
                                    delete cell;
                                    return;
                                    }
                              else {
                                    cell->element->read(e);
                                    cell->element->styleChanged();
                                    if (cell->element->type() == ElementType::ICON) {
                                          Icon* icon = static_cast<Icon*>(cell->element);
                                          QAction* ac = getAction(icon->action());
                                          if (ac) {
                                                QIcon qicon(ac->icon());
                                                icon->setAction(icon->action(), qicon);
                                                }
                                          else {
                                                add = false; // action is not valid, don't add it to the palette.
                                                }
                                          }
                                    }
                              }
                        }
                  if (add) {
                        int idx = _moreElements ? cells.size() - 1 : cells.size();
                        cells.insert(idx, cell);
                        }
                  }
            else
                  e.unknown();
            }
            // make sure hgrid and vgrid are not 0, we divide by them later
            if (hgrid <= 0)
                  hgrid = 28;
            if (vgrid <= 0)
                  vgrid = 28;
      }

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void Palette::clear()
      {
      qDeleteAll(cells);
      cells.clear();
      }

//---------------------------------------------------------
//   rows
//---------------------------------------------------------

int Palette::rows() const
      {
      int c = columns();
      if (c == 0)
            return 0;
      return (size() + c - 1) / c;
      }

//---------------------------------------------------------
//   heightForWidth
//---------------------------------------------------------

int Palette::heightForWidth(int w) const
      {
      int c = w / hgrid;
      if (c <= 0)
            c = 1;
      int s = size();
      if (_moreElements)
            s += 1;
      int rows = (s + c - 1) / c;
      if (rows <= 0)
            rows = 1;
      qreal mag = PALETTE_SPATIUM * extraMag;
      int h = lrint(_yOffset * 2 * mag);
      return rows * vgrid + h;
      }

//---------------------------------------------------------
//   sizeHint
//---------------------------------------------------------

QSize Palette::sizeHint() const
      {
      int h = heightForWidth(width());
      return QSize((width() / hgrid) * hgrid, h);
      }

//---------------------------------------------------------
//   actionToggled
//---------------------------------------------------------

void Palette::actionToggled(bool /*val*/)
      {
      selectedIdx = -1;
      int nn = ccp()->size();
      for (int n = 0; n < nn; ++n) {
            Element* e = cellAt(n)->element;
            if (e && e->type() == ElementType::ICON) {
                  QAction* a = getAction(static_cast<Icon*>(e)->action());
                  if (a->isChecked()) {
                        selectedIdx = n;
                        break;
                        }
                  }
            }
      update();
      }

//---------------------------------------------------------
//   PaletteProperties
//---------------------------------------------------------

PaletteProperties::PaletteProperties(Palette* p, QWidget* parent)
   : QDialog(parent)
      {
      setObjectName("PaletteProperties");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

      palette = p;

      name->setText(palette->name());
      cellWidth->setValue(palette->gridWidth());
      cellHeight->setValue(palette->gridHeight());
      showGrid->setChecked(palette->drawGrid());
      moreElements->setChecked(palette->moreElements());
      elementOffset->setValue(palette->yOffset());
      mag->setValue(palette->mag());

      MuseScore::restoreGeometry(this);
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void PaletteProperties::accept()
      {
      palette->setName(name->text());
      palette->setGrid(cellWidth->value(), cellHeight->value());
      palette->setDrawGrid(showGrid->isChecked());
      palette->setMoreElements(moreElements->isChecked());
      palette->setYOffset(elementOffset->value());
      palette->setMag(mag->value());
      palette->emitChanged();
      QDialog::accept();
      }

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void PaletteProperties::hideEvent(QHideEvent* event)
      {
      MuseScore::saveGeometry(this);
      QWidget::hideEvent(event);
      }

//---------------------------------------------------------
//   PaletteCellProperties
//---------------------------------------------------------

PaletteCellProperties::PaletteCellProperties(PaletteCell* p, QWidget* parent)
   : QDialog(parent)
      {
      setObjectName("PaletteCellProperties");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

      cell = p;

      xoffset->setValue(cell->xoffset);
      yoffset->setValue(cell->yoffset);
      scale->setValue(cell->mag);
      drawStaff->setChecked(cell->drawStaff);
      name->setText(p->name);

      MuseScore::restoreGeometry(this);
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void PaletteCellProperties::accept()
      {
      cell->xoffset = xoffset->value();
      cell->yoffset = yoffset->value();
      cell->mag     = scale->value();
      cell->name    = name->text();
      cell->drawStaff = drawStaff->isChecked();
      QDialog::accept();
      }

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void PaletteCellProperties::hideEvent(QHideEvent* event)
      {
      MuseScore::saveGeometry(this);
      QWidget::hideEvent(event);
      }

//---------------------------------------------------------
// PaletteScrollArea
//---------------------------------------------------------

PaletteScrollArea::PaletteScrollArea(Palette* w, QWidget* parent)
   : QScrollArea(parent)
      {
      setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
      setWidget(w);
      setWidgetResizable(false);
      _restrictHeight = true;
      setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
      }

//---------------------------------------------------------
// resizeEvent
//---------------------------------------------------------

void PaletteScrollArea::resizeEvent(QResizeEvent* re)
      {
      QScrollArea::resizeEvent(re); // necessary?

      Palette* palette = static_cast<Palette*>(widget());
      int h = palette->heightForWidth(width());
      palette->resize(QSize(width() - 6, h));
      if (_restrictHeight) {
            setMaximumHeight(h+6);
            }
      }

//---------------------------------------------------------
//   dragEnterEvent
//---------------------------------------------------------

void Palette::dragEnterEvent(QDragEnterEvent* event)
      {
      const QMimeData* dta = event->mimeData();
      if (dta->hasUrls()) {
            QList<QUrl>ul = event->mimeData()->urls();
            QUrl u = ul.front();
            if (MScore::debugMode) {
                  qDebug("dragEnterEvent: Url: %s", qPrintable(u.toString()));
                  qDebug("   scheme <%s> path <%s>", qPrintable(u.scheme()), qPrintable(u.path()));
                  }
            if (u.scheme() == "file") {
                  QFileInfo fi(u.path());
                  QString suffix(fi.suffix().toLower());
                  if (suffix == "svg"
                     || suffix == "jpg"
                     || suffix == "jpeg"
                     || suffix == "png"
                     ) {
                        event->acceptProposedAction();
                        }
                  }
            }
      else if (dta->hasFormat(mimeSymbolFormat)) {
            event->accept();
            update();
            }
      else {
            event->ignore();
#ifndef NDEBUG
            qDebug("dragEnterEvent: formats:");
            for (const QString& s : event->mimeData()->formats())
                  qDebug("   %s", qPrintable(s));
#endif
            }
      }

//---------------------------------------------------------
//   dragMoveEvent
//---------------------------------------------------------

void Palette::dragMoveEvent(QDragMoveEvent* event)
      {
      int i = idx(event->pos());
      if (event->source() == this) {
            if (i != -1) {
                  if (currentIdx != -1 && event->proposedAction() == Qt::MoveAction) {
                        if (i != currentIdx) {
                              PaletteCell* c = cells.takeAt(currentIdx);
                              cells.insert(i, c);
                              currentIdx = i;
                              update();
                              }
                        event->accept();
                        return;
                        }
                  }
            event->ignore();
            }
      else
            event->accept();
      }

//---------------------------------------------------------
//   dropEvent
//---------------------------------------------------------

void Palette::dropEvent(QDropEvent* event)
      {
      Element* e = 0;
      QString name;

      const QMimeData* datap = event->mimeData();
      if (datap->hasUrls()) {
            QList<QUrl>ul = event->mimeData()->urls();
            QUrl u = ul.front();
            if (u.scheme() == "file") {
                  QFileInfo fi(u.path());
                  Image* s = new Image(gscore);
                  QString filePath(u.toLocalFile());
                  s->load(filePath);
                  e = s;
                  QFileInfo f(filePath);
                  name = f.completeBaseName();
                  }
            }
      else if (datap->hasFormat(mimeSymbolFormat)) {
            QByteArray dta(event->mimeData()->data(mimeSymbolFormat));
            XmlReader xml(dta);
            QPointF dragOffset;
            Fraction duration;
            ElementType type = Element::readType(xml, &dragOffset, &duration);

            if (type == ElementType::SYMBOL) {
                  Symbol* symbol = new Symbol(gscore);
                  symbol->read(xml);
                  e = symbol;
                  }
            else {
                  e = Element::create(type, gscore);
                  if (e) {
                        e->read(xml);
                        e->setTrack(0);
                        if (e->isIcon()) {
                              Icon* i = toIcon(e);
                              const QByteArray& action = i->action();
                              if (!action.isEmpty()) {
                                    const Shortcut* s = Shortcut::getShortcut(action);
                                    if (s) {
                                          QAction* a = s->action();
                                          QIcon icon(a->icon());
                                          i->setAction(action, icon);
                                          }
                                    }
                              }
                        }
                  }
            }
      if (e == 0) {
            event->ignore();
            return;
            }
      if (event->source() == this) {
            delete e;
            if (event->proposedAction() == Qt::MoveAction) {
                  event->accept();
                  emit changed();
                  return;
                  }
            event->ignore();
            return;
            }
      e->setSelected(false);
      int i = idx(event->pos());
      if (i == -1 || cells[i])
            append(e, name);
      else
            add(i, e, name);
      event->accept();
      while (!cells.isEmpty() && cells.back() == 0)
            cells.removeLast();
      setFixedHeight(heightForWidth(width()));
      updateGeometry();
      update();
      emit changed();
      }

}

