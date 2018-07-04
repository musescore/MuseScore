//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2009-2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================


#include "pianoview.h"
#include "pianoruler.h"
#include "libmscore/staff.h"
#include "pianokeyboard.h"
#include "libmscore/measure.h"
#include "libmscore/chord.h"
#include "libmscore/score.h"
#include "libmscore/note.h"
#include "libmscore/slur.h"
#include "libmscore/segment.h"
#include "libmscore/noteevent.h"

namespace Ms {

static const QColor colSelectionBox = QColor(255, 128, 0);
static const QColor colSelectionBoxFill = QColor(255, 128, 0, 128);

const QColor noteDeselected = QColor(29, 204, 160);
const QColor noteSelected = Qt::yellow;

const QColor colPianoBg(60, 60, 60);

const QColor noteDeselectedBlack = noteDeselected.darker(150);
const QColor noteSelectedBlack = noteSelected.darker(150);


static const qreal MIN_DRAG_DIST_SQ = 9;
      
//---------------------------------------------------------
//   PianoItem
//---------------------------------------------------------

PianoItem::PianoItem(Note* n, NoteEvent* e, PianoView* pianoView)
   : _note(n), _event(e), _pianoView(pianoView), isBlack(false)
      {
      updateValues();
      }

//---------------------------------------------------------
//   updateValues
//---------------------------------------------------------

void PianoItem::updateValues()
      {
      Chord* chord = _note->chord();
      int ticks    = chord->duration().ticks();
      int tieLen   = _note->playTicks() - ticks;
      int pitch    = _note->pitch() + _event->pitch();
      int len      = ticks * _event->len() / 1000 + tieLen;

      int degree = pitch % 12;
      isBlack = degree == 1 || degree == 3 || degree == 6 || degree == 8 || degree == 10;

      qreal tix2pix = _pianoView->xZoom();
      int noteHeight = _pianoView->noteHeight();
      
      qreal x1 = _pianoView->tickToPixelX(_note->chord()->tick() + _event->ontime() * ticks / 1000);
      qreal y1 = (127 - pitch) * noteHeight;
      
      rect.setRect(x1, y1, len * tix2pix, noteHeight);
      }


//---------------------------------------------------------
//   startTick
//---------------------------------------------------------


int PianoItem::startTick()
      {
      Chord* chord = _note->chord();
      int ticks    = chord->duration().ticks();
      return _note->chord()->tick() + _event->ontime() * ticks / 1000;
      }

//---------------------------------------------------------
//   tick length
//---------------------------------------------------------


int PianoItem::tickLength()
      {
      Chord* chord = _note->chord();
      int ticks    = chord->duration().ticks();
      int tieLen   = _note->playTicks() - ticks;
      return ticks * _event->len() / 1000 + tieLen;
      }

//---------------------------------------------------------
//   pitch
//---------------------------------------------------------


int PianoItem::pitch()
      {
      return _note->pitch() + _event->pitch();
      }

//---------------------------------------------------------
//   paint
//---------------------------------------------------------

void PianoItem::paint(QPainter* painter)
      {
      painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);
      
      int pitch    = _note->pitch() + _event->pitch();
      int degree = pitch % 12;
      bool isBlack = degree == 1 || degree == 3 || degree == 6 || degree == 8 || degree == 10;
      int roundRadius = 3;

      QColor noteColor = _note->selected() ? noteSelected : noteDeselected;
      painter->setBrush(noteColor);
      
      painter->setPen(Qt::NoPen);
      QRectF bounds = rect;
      painter->drawRoundedRect(bounds, roundRadius, roundRadius);

//      const QString pitchNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
      const QString pitchNames[] = {"c", "c#", "d", "d#", "e", "f", "f#", "g", "g#", "a", "a#", "b"};

      //Pitch name
      if (bounds.width() >= 20 && bounds.height() >= 11)
            {
            QRectF textRect(bounds.x() + 2, bounds.y() - 1, bounds.width() - 6, bounds.height() + 1);
            QRectF textHiliteRect(bounds.x() + 3, bounds.y(), bounds.width() - 6, bounds.height());
            
            QFont f("FreeSans",  8);
            painter->setFont(f);

            //Note name
            painter->setPen(QPen(noteColor.lighter(130)));
            painter->drawText(textHiliteRect,
                    Qt::AlignLeft | Qt::AlignTop, pitchNames[pitch % 12]);

            painter->setPen(QPen(noteColor.darker(180)));
            painter->drawText(textRect,
                    Qt::AlignLeft | Qt::AlignTop, pitchNames[pitch % 12]);
            
            
            //Black key hint
            if (isBlack && bounds.width() >= 30)
                  {
                  painter->setPen(QPen(noteColor.lighter(130)));
                  painter->drawText(textHiliteRect,
                          Qt::AlignRight | Qt::AlignTop, QString("#"));
                  
                  painter->setPen(QPen(noteColor.darker(180)));
                  painter->drawText(textRect,
                          Qt::AlignRight | Qt::AlignTop, QString("#"));
                  }
            }
      
//      if (isBlack) {
//            painter->setBrush(isSelected() ? noteSelected : noteDeselected);
//            painter->drawRoundedRect(bounds.x() + 2, 
//                    bounds.y() + 2, 
//                    bounds.width() - 4, 
//                    bounds.height() - 4, 
//                    roundRadius, roundRadius);
//            
//            }
      }


//---------------------------------------------------------
//   PianoView
//---------------------------------------------------------

PianoView::PianoView()
   : QGraphicsView()
      {
      setFrameStyle(QFrame::NoFrame);
      setLineWidth(0);
      setMidLineWidth(0);
      setScene(new QGraphicsScene);
      setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
      setResizeAnchor(QGraphicsView::AnchorUnderMouse);
      setMouseTracking(true);
      _timeType = TType::TICKS;
      _staff     = 0;
      chord     = 0;
      _noteHeight = DEFAULT_KEY_HEIGHT;
      _xZoom = 0.1;
      dragStarted = false;
      mouseDown = false;
      dragStyle = DragStyle::NONE;
      tempUndoEvent = false;


      }

//---------------------------------------------------------
//   ~PianoView
//---------------------------------------------------------

PianoView::~PianoView()
      {
      clearNoteData();
      }


//---------------------------------------------------------
//   drawBackground
//---------------------------------------------------------

void PianoView::drawBackground(QPainter* p, const QRectF& r)
      {
      if (_staff == 0)
            return;
      Score* _score = _staff->score();
      setFrameShape(QFrame::NoFrame);

      QColor colGutter = colPianoBg.darker(150);
      QColor colBlackKeyBg = colPianoBg.darker(130);
      QColor colGridLineMajor = colPianoBg.darker(180);
      QColor colGridLineMinor = colPianoBg.darker(160);
      QPen penLineMajor = QPen(colGridLineMajor, 2.0, Qt::SolidLine);
      QPen penLineMinor = QPen(colGridLineMinor, 1.0, Qt::SolidLine);

      QRectF r1;
      r1.setCoords(-1000000.0, 0.0, tickToPixelX(0), 1000000.0);
      QRectF r2;
      r2.setCoords(tickToPixelX(ticks), 0.0, 1000000.0, 1000000.0);
      
      p->fillRect(r, colPianoBg);
      if (r.intersects(r1))
            p->fillRect(r.intersected(r1), colGutter);
      if (r.intersects(r2))
            p->fillRect(r.intersected(r2), colGutter);

      //
      // draw horizontal grid lines
      //
      qreal y1 = r.y();
      qreal y2 = y1 + r.height();
      qreal x1 = r.x();
      qreal x2 = x1 + r.width();

      int topPitch = ceil((_noteHeight * 128 - y1) / _noteHeight);
      int bmPitch = floor((_noteHeight * 128 - y2) / _noteHeight);
      
      //MIDI notes span [0, 127] and map to pitches starting at C-1
      for (int pitch = bmPitch; pitch <= topPitch; ++pitch) {
            int y = (127 - pitch) * _noteHeight;

            //int octave = pitch / 12;
            int degree = pitch % 12;
            if (degree == 1 || degree == 3 || degree == 6 || degree == 8 || degree == 10)
            {
                  qreal px0 = qMax(r.x(), (qreal)tickToPixelX(0));
                  qreal px1 = qMin(r.x() + r.width(), (qreal)tickToPixelX(ticks));
                  QRectF hbar;
                  
                  hbar.setCoords(px0, y, px1, y + _noteHeight);
                  p->fillRect(hbar, colBlackKeyBg);
            }
            

            //Lines between rows
            p->setPen(degree == 0 ? penLineMajor : penLineMinor);
            p->drawLine(QLineF(x1, y + _noteHeight, x2, y + _noteHeight));
      
            }

      //
      // draw vertical grid lines
      //
      Pos pos1(_score->tempomap(), _score->sigmap(), qMax(pixelXToTick(x1), 0), TType::TICKS);
      Pos pos2(_score->tempomap(), _score->sigmap(), qMax(pixelXToTick(x2), 0), TType::TICKS);
      
      int bar1, bar2, beat, tick;
      pos1.mbt(&bar1, &beat, &tick);
      pos2.mbt(&bar2, &beat, &tick);
      
      //Draw bar lines
      const int minBeatGap = 20;
      for (int bar = bar1; bar <= bar2; ++bar) {
            Pos barPos(_score->tempomap(), _score->sigmap(), bar, 0, 0);

            //Beat lines
            int beatsInBar = barPos.timesig().timesig().numerator();
            int ticksPerBeat = barPos.timesig().timesig().beatTicks();
            int beatSkip = ceil(minBeatGap / (ticksPerBeat * _xZoom));
            //Round up to next power of 2
            beatSkip = (int)pow(2, ceil(log(beatSkip)/log(2)));
            
            for (int beat = 0; beat < beatsInBar; beat += beatSkip)
                  {
                  Pos beatPos(_score->tempomap(), _score->sigmap(), bar, beat, 0);
                  double x = tickToPixelX(beatPos.time(TType::TICKS));
                  p->setPen(penLineMinor);
                  p->drawLine(x, y1, x, y2);
                  }
            
            
            //Bar line
            double x = tickToPixelX(barPos.time(TType::TICKS));
            p->setPen(x > 0 ? penLineMajor : QPen(Qt::black, 2.0));
            p->drawLine(x, y1, x, y2);
            }
      
      
      //Draw notes
      for (int i = 0; i < noteList.size(); ++i)
      {
            noteList[i]->paint(p);
      }

      
      //Draw drag selection box
      if (dragStarted && dragStyle == DragStyle::SELECTION_RECT)
            {
            int minX = qMin(mouseDownPos.x(), lastMousePos.x());
            int minY = qMin(mouseDownPos.y(), lastMousePos.y());
            int maxX = qMax(mouseDownPos.x(), lastMousePos.x());
            int maxY = qMax(mouseDownPos.y(), lastMousePos.y());
            QRectF rect(minX, minY, maxX - minX + 1, maxY - minY + 1);
            
            p->setPen(QPen(colSelectionBox, 2));
            p->setBrush(QBrush(colSelectionBoxFill, Qt::SolidPattern));
            p->drawRect(rect);
            }
      }

//---------------------------------------------------------
//   createLocators
//---------------------------------------------------------

void PianoView::createLocators()
      {
      static const QColor lcColors[3] = { Qt::red, Qt::blue, Qt::blue };
      for (int i = 0; i < 3; ++i) {
            locatorLines[i] = new QGraphicsLineItem(QLineF(0.0, 0.0, 0.0, _noteHeight * 128));
            QPen pen(lcColors[i]);
            pen.setWidth(2);
            locatorLines[i]->setPen(pen);
            locatorLines[i]->setZValue(1000+i);       // set stacking order
            locatorLines[i]->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
            scene()->addItem(locatorLines[i]);
            }
      }

//---------------------------------------------------------
//   moveLocator
//---------------------------------------------------------

void PianoView::moveLocator(int i)
      {
      
      if (_locator[i].valid()) {
            locatorLines[i]->setVisible(true);
            qreal x = tickToPixelX(_locator[i].time(TType::TICKS));
            locatorLines[i]->setPos(QPointF(x, 0.0));
            }
      else
            locatorLines[i]->setVisible(false);
      }



//---------------------------------------------------------
//   pixelXToTick
//---------------------------------------------------------

int PianoView::pixelXToTick(int pixX) {
      return (int)(pixX / _xZoom) - MAP_OFFSET; 
      }


//---------------------------------------------------------
//   tickToPixelX
//---------------------------------------------------------

int PianoView::tickToPixelX(int tick) { 
      return (int)(tick + MAP_OFFSET) *  _xZoom; 
      }

//---------------------------------------------------------
//   wheelEvent
//---------------------------------------------------------

void PianoView::wheelEvent(QWheelEvent* event)
      {
      int step = event->delta() / 120;

      if (event->modifiers() == Qt::ControlModifier) 
            {
            //Horizontal zoom
            
            //QPointF mouseScene = mapToScene(event->pos());
            QRectF viewRect = mapToScene(viewport()->geometry()).boundingRect();
            
            int mouseXTick = pixelXToTick(event->x() + (int)viewRect.x());
            
            _xZoom *= pow(X_ZOOM_RATIO, step);
            emit xZoomChanged(_xZoom);
            
            updateBoundingSize();
            updateNotes();
            
            int mousePixX = tickToPixelX(mouseXTick);
            horizontalScrollBar()->setValue(mousePixX - event->x());

            scene()->update();
            }
      else if (event->modifiers() == Qt::ShiftModifier) 
            {
            //Horizontal scroll
            QWheelEvent we(event->pos(), event->delta(), event->buttons(), 0, Qt::Horizontal);
            QGraphicsView::wheelEvent(&we);
            }
      else if (event->modifiers() == 0)
            {
            //Vertical scroll
            QGraphicsView::wheelEvent(event);
            }
      else if (event->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier))
            {
            //Vertical zoom
            QRectF viewRect = mapToScene(viewport()->geometry()).boundingRect();
            qreal mouseYNote = (event->y() + (int)viewRect.y()) / (qreal)_noteHeight;
            
            _noteHeight = qMax(qMin(_noteHeight + step, MAX_KEY_HEIGHT), MIN_KEY_HEIGHT);
            emit noteHeightChanged(_noteHeight);
            
            updateBoundingSize();
            updateNotes();
            
            int mousePixY = (int)(mouseYNote * _noteHeight);
            verticalScrollBar()->setValue(mousePixY - event->y());
            
            scene()->update();
            }
      }


//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void PianoView::mousePressEvent(QMouseEvent* event)
      {
      mouseDown = true;
      mouseDownPos = mapToScene(event->pos());
      lastMousePos = mouseDownPos;
      }

//---------------------------------------------------------
//   mouseReleaseEvent
//---------------------------------------------------------

void PianoView::mouseReleaseEvent(QMouseEvent* event)
      {
      QPointF mouseUpPos = mapToScene(event->pos());
      
//      bool bnLeft = event->buttons() & Qt::LeftButton;
//      bool bnMid = event->buttons() & Qt::MidButton;
//      bool bnRight = event->buttons() & Qt::RightButton;

      int modifiers = QGuiApplication::keyboardModifiers();
      bool bnShift = modifiers & Qt::ShiftModifier;
      bool bnCtrl = modifiers & Qt::ControlModifier;
//      bool bnAlt = modifiers & Qt::AltModifier;
      
      NoteSelectType selType = bnShift ? (bnCtrl ? NoteSelectType::SUBTRACT : NoteSelectType::XOR)
              : (bnCtrl ? NoteSelectType::ADD : NoteSelectType::REPLACE);

      if (dragStarted)
            {
            if (dragStyle == DragStyle::SELECTION_RECT)
                  {
                  //Update selection
                  qreal minX = qMin(mouseDownPos.x(), lastMousePos.x());
                  qreal minY = qMin(mouseDownPos.y(), lastMousePos.y());
                  qreal maxX = qMax(mouseDownPos.x(), lastMousePos.x());
                  qreal maxY = qMax(mouseDownPos.y(), lastMousePos.y());

                  int startTick = pixelXToTick((int)minX);
                  int endTick = pixelXToTick((int)maxX);
                  int lowPitch = (int)floor(128 - maxY / noteHeight());
                  int highPitch = (int)ceil(128 - minY / noteHeight());

                  selectNotes(startTick, endTick, lowPitch, highPitch, selType);
                  }
            else if (dragStyle == DragStyle::MOVE_NOTES)
                  {
                  Score* score = _staff->score();
                  if (tempUndoEvent)
                        {
                        score->undoRedo(true, 0, false);
                        tempUndoEvent = false;
                        }
                  
                  //Final note move action
                  int mouseDownPitch = pixelYToPitch(mouseDownPos.y());
                  int curPitch = pixelYToPitch(mouseUpPos.y());
                  int pitchDelta = curPitch - mouseDownPitch;
                  score->startCmd();
                  score->upDownDelta(pitchDelta, false);
                  score->endCmd();
                  }

            dragStarted = false;
            }
      else
            {
            int startTick = pixelXToTick((int)mouseDownPos.x());
            int lowPitch = pixelYToPitch(mouseDownPos.y());
            
            selectNotes(startTick, startTick + 1, lowPitch, lowPitch, selType);
            }
      
      
      dragStyle = DragStyle::NONE;
      mouseDown = false;
      scene()->update();
      }


//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void PianoView::mouseMoveEvent(QMouseEvent* event)
      {
      lastMousePos = mapToScene(event->pos());

      if (mouseDown && !dragStarted)
            {
            qreal dx = lastMousePos.x() - mouseDownPos.x();
            qreal dy = lastMousePos.y() - mouseDownPos.y();

            if (dx * dx + dy * dy >= MIN_DRAG_DIST_SQ)
                  {
                  //Start dragging
                  dragStarted = true;
                  
                  //Check for move note
                  int tick = pixelXToTick(mouseDownPos.x());
                  int mouseDownPitch = pixelYToPitch(mouseDownPos.y());
                  PianoItem* pi = pickNote(tick, mouseDownPitch);
                  if (pi)
                        {
                        if (!pi->note()->selected())
                              {
                              selectNotes(tick, tick, mouseDownPitch, mouseDownPitch, NoteSelectType::REPLACE);
                              }
                        dragStyle = DragStyle::MOVE_NOTES;
                        lastDragPitch = mouseDownPitch;
                        }
                  else
                        {
                        dragStyle = DragStyle::SELECTION_RECT;
                        }
                  }
            }

      if (dragStarted)
            {
            if (dragStyle == DragStyle::MOVE_NOTES)
                  {
                  int mouseDownPitch = pixelYToPitch(mouseDownPos.y());
                  int curPitch = pixelYToPitch(lastMousePos.y());
                  if (curPitch != lastDragPitch)
                        {
                        int pitchDelta = curPitch - mouseDownPitch;
                        
                        Score* score = _staff->score();
                        if (tempUndoEvent)
                              {
                              score->undoRedo(true, 0, false);
                              tempUndoEvent = false;
                              }
                        
                        score->startCmd();
                        score->upDownDelta(pitchDelta, false);
                        score->endCmd();
                        
                        tempUndoEvent = true;
                        lastDragPitch = curPitch;
                        }
                  }
            
            scene()->update();
            //update();
            }
      

      //Update mouse tracker      
      QPointF p(mapToScene(event->pos()));
      int pitch = (int)((_noteHeight * 128 - p.y()) / _noteHeight);
      emit pitchChanged(pitch);

      int tick = pixelXToTick(p.x());
      if (tick < 0) {
            tick = 0;
            trackingPos.setTick(tick);
            trackingPos.setInvalid();
            }
      else
            trackingPos.setTick(tick);
      emit trackingPosChanged(trackingPos);
      }
      
//---------------------------------------------------------
//   selectNotes
//---------------------------------------------------------

//PianoItem* PianoView::pickNote(int pixX, int pixY)
PianoItem* PianoView::pickNote(int tick, int pitch)
      {
      for (int i = 0; i < noteList.size(); ++i)
            {
            PianoItem* pi = noteList[i];
            
            if (tick >= pi->startTick() && tick <= pi->startTick() + pi->tickLength()
                    && pitch == pi->pitch())
                  {
                  return pi;
                  }
            }
      
      return 0;
      }

//---------------------------------------------------------
//   selectNotes
//---------------------------------------------------------

void PianoView::selectNotes(int startTick, int endTick, int lowPitch, int highPitch, NoteSelectType selType)
      {

      Score* score = _staff->score();
      Selection selection(score);

      for (int i = 0; i < noteList.size(); ++i)
            {
            PianoItem* pi = noteList[i];
            int ts = pi->startTick();
            int tLen = pi->tickLength();

            int pitch = pi->note()->pitch();


            bool inBounds = ts + tLen >= startTick && ts <= endTick 
                  && pitch >= lowPitch && pitch <= highPitch;

            bool sel;
            switch (selType)
                  {
                  default:
                  case NoteSelectType::REPLACE:
                        sel = inBounds;
                        break;
                  case NoteSelectType::XOR:
                        sel = inBounds != pi->note()->selected();
                        break;
                  case NoteSelectType::ADD:
                        sel = inBounds || pi->note()->selected();
                        break;
                  case NoteSelectType::SUBTRACT:
                        sel = !inBounds && pi->note()->selected();
                        break;
                  }
            
            if (sel)
                  {
                  selection.add(pi->note());
                  }
            }

      score->setSelection(selection);
      for (MuseScoreView* view : score->getViewer())
            view->updateAll();
      
      scene()->update();
      }

//---------------------------------------------------------
//   leaveEvent
//---------------------------------------------------------

void PianoView::leaveEvent(QEvent* event)
      {
      emit pitchChanged(-1);
      trackingPos.setInvalid();
      emit trackingPosChanged(trackingPos);
      QGraphicsView::leaveEvent(event);
      }

//---------------------------------------------------------
//   ensureVisible
//---------------------------------------------------------

void PianoView::ensureVisible(int tick)
      {
      QRectF rect = mapToScene(viewport()->geometry()).boundingRect();

      qreal xpos = tickToPixelX(tick);
      qreal margin = rect.width() / 2;
      if (xpos < rect.x() + margin)
            horizontalScrollBar()->setValue(qMax(xpos - margin, 0.0));
      else if (xpos >= rect.x() + rect.width() - margin)
            horizontalScrollBar()->setValue(qMax(xpos - rect.width() + margin, 0.0));
      }

//---------------------------------------------------------
//   updateBoundingSize
//---------------------------------------------------------
void PianoView::updateBoundingSize()
      {
      Measure* lm = _staff->score()->lastMeasure();
      ticks       = lm->tick() + lm->ticks();
      scene()->setSceneRect(0.0, 0.0, 
              double((ticks + MAP_OFFSET * 2) * _xZoom),
              _noteHeight * 128);
      
      }

//---------------------------------------------------------
//   setStaff
//---------------------------------------------------------

void PianoView::setStaff(Staff* s, Pos* l)
      {
      _locator = l;
      
      if (_staff == s)
            {
            return;
            }
      
      _staff    = s;
      setEnabled(_staff != nullptr);
      if (!_staff) {
            scene()->blockSignals(true);  // block changeSelection()
            scene()->clear();
            clearNoteData();
            scene()->blockSignals(false);
            return;
            }

      trackingPos.setContext(_staff->score()->tempomap(), _staff->score()->sigmap());
      updateBoundingSize();

      updateNotes();
      
      QRectF boundingRect;
      bool brInit = false;
      QRectF boundingRectSel;
      bool brsInit = false;
      
      foreach (PianoItem* item, noteList) {
            if (!brInit)
                  {
                  boundingRect = item->boundingRect();
                  brInit = true;
                  }
            else
                  {
                  boundingRect |= item->boundingRect();
                  }

            if (item->note()->selected())
                  {
                  if (!brsInit)
                        {
                        boundingRectSel = item->boundingRect();
                        brsInit = true;
                        }
                  else
                        {
                        boundingRectSel |= item->boundingRect();
                        }
                  }
                  
            }

      QRectF viewRect = mapToScene(viewport()->geometry()).boundingRect();
      
      if (brsInit)
            {
            horizontalScrollBar()->setValue(boundingRectSel.x());
            verticalScrollBar()->setValue(qMax(boundingRectSel.y() + (boundingRectSel.height() - viewRect.height()) / 2, 0.0));
            }
      else if (brInit)
            {
            horizontalScrollBar()->setValue(boundingRect.x());
            verticalScrollBar()->setValue(qMax(boundingRect.y() - (boundingRectSel.height() - viewRect.height()) / 2, 0.0));
            }
      else
            {
            
            horizontalScrollBar()->setValue(0);
            verticalScrollBar()->setValue(qMax(viewRect.y() - viewRect.height() / 2, 0.0));
            }
      }

//---------------------------------------------------------
//   addChord
//---------------------------------------------------------

void PianoView::addChord(Chord* chord)
      {
      for (Chord* c : chord->graceNotes())
            addChord(c);
      for (Note* note : chord->notes()) {
            if (note->tieBack())
                  continue;
            for (NoteEvent& e : note->playEvents())
                  {
                  noteList.append(new PianoItem(note, &e, this));
                  }
            }
      }

//---------------------------------------------------------
//   updateNotes
//---------------------------------------------------------

void PianoView::updateNotes()
      {
      scene()->blockSignals(true);  // block changeSelection()
      scene()->clearFocus();
      scene()->clear();
      clearNoteData();
      createLocators();

      int staffIdx   = _staff->idx();
      int startTrack = staffIdx * VOICES;
      int endTrack   = startTrack + VOICES;

      SegmentType st = SegmentType::ChordRest;
      for (Segment* s = _staff->score()->firstSegment(st); s; s = s->next1(st)) {
            for (int track = startTrack; track < endTrack; ++track) {
                  Element* e = s->element(track);
                  if (e && e->isChord())
                        addChord(toChord(e));
                  }
            }
      for (int i = 0; i < 3; ++i)
            moveLocator(i);
      scene()->blockSignals(false);
      
      scene()->update(sceneRect());
      }

//---------------------------------------------------------
//   updateNotes
//---------------------------------------------------------

void PianoView::clearNoteData()
      {
      for (int i = 0; i < noteList.size(); ++i)
            {
            delete noteList[i];
            }
      
      noteList.clear();
      }


//---------------------------------------------------------
//   getSelectedItems
//---------------------------------------------------------

QList<PianoItem*> PianoView::getSelectedItems()
      {
      QList<PianoItem*> list;
      for (int i = 0; i < noteList.size(); ++i)
            {
            if (noteList[i]->note()->selected())
                  {
                  list.append(noteList[i]);
                  }
            }
      return list;
      }

//---------------------------------------------------------
//   getItems
//---------------------------------------------------------

QList<PianoItem*> PianoView::getItems()
      {
      QList<PianoItem*> list;
      for (int i = 0; i < noteList.size(); ++i)
            {
            list.append(noteList[i]);
            }
      return list;
      }

}

