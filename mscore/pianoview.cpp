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

//static const int MAP_OFFSET = 480;

//---------------------------------------------------------
//   pitch2y
//---------------------------------------------------------

//static int pitch2y(int pitch)
//      {
//      static int tt[] = {
//            12, 19, 25, 32, 38, 51, 58, 64, 71, 77, 84, 90
//            };
//      int y = (75 * keyHeight) - (tt[pitch % 12] + (7 * keyHeight) * (pitch / 12));
//      if (y < 0)
//            y = 0;
//      return y;
//      }

//---------------------------------------------------------
//   PianoItem
//---------------------------------------------------------

PianoItem::PianoItem(Note* n, NoteEvent* e, PianoView* pianoView)
   : QGraphicsRectItem(0), _note(n), _event(e), _pianoView(pianoView), isBlack(false)
      {
      setFlags(flags() | QGraphicsItem::ItemIsSelectable);
      setBrush(QBrush());
      updateValues();
      }

//---------------------------------------------------------
//   updateValues
//---------------------------------------------------------

void PianoItem::updateValues()
      {
//      QRectF r(rect().translated(pos()));
      
      Chord* chord = _note->chord();
      int ticks    = chord->duration().ticks();
      int tieLen   = _note->playTicks() - ticks;
      int pitch    = _note->pitch() + _event->pitch();
      int len      = ticks * _event->len() / 1000 + tieLen;

      int degree = pitch % 12;
      isBlack = degree == 1 || degree == 3 || degree == 6 || degree == 8 || degree == 10;
      
      setSelected(_note->selected());
      
      qreal tix2pix = _pianoView->xZoom();
      int noteHeight = _pianoView->noteHeight();
      
      qreal x1 = (_note->chord()->tick() + _event->ontime() * ticks / 1000 + MAP_OFFSET) * tix2pix;
      qreal y1 = (127 - pitch) * noteHeight;
      
      setRect(x1, y1, len * tix2pix, noteHeight);
            
//      if (isBlack)
//            setRect(x1, y1 + 1, len * tix2pix, noteHeight - 2);
//      else
//            setRect(x1, y1, len * tix2pix, noteHeight);
      
//      setRect(0, 0, len, keyHeight/2);
//
//      setPos(_note->chord()->tick() + _event->ontime() * ticks / 1000 + MAP_OFFSET,
//         pitch2y(pitch) + keyHeight / 4);
//
//      return r | rect().translated(pos());
      }

//---------------------------------------------------------
//   paint
//---------------------------------------------------------

void PianoItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
      {
      painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);
//      painter->setRenderHint(QPainter::Antialiasing);
      
      int pitch    = _note->pitch() + _event->pitch();
      int degree = pitch % 12;
      bool isBlack = degree == 1 || degree == 3 || degree == 6 || degree == 8 || degree == 10;
      int roundRadius = 3;
      
//      painter->setPen(pen());
//      painter->setBrush(isSelected() ? Qt::yellow : Qt::blue);
//      QColor outlineColor = isSelected() ? noteSelectedBlack : noteDeselectedBlack;
      
//      if (isBlack)
////            painter->setPen(QPen(outlineColor, 1));
//            painter->setBrush(isSelected() ? noteSelectedBlack : noteDeselectedBlack);
//      else
////            painter->setPen(QPen(outlineColor, 1));
//            painter->setBrush(isSelected() ? noteSelected : noteDeselected);
      

      QColor noteColor = isSelected() ? noteSelected : noteDeselected;
      painter->setBrush(noteColor);
      
      
      
//      painter->drawRect(boundingRect());
//      painter->setPen(QPen(outlineColor, 1));
//      painter->setPen(isBlack ? QPen(Qt::black, 1, Qt::DotLine) : Qt::NoPen);
      
//      painter->setPen(isBlack ? QPen(Qt::black, 1, Qt::SolidLine) : Qt::NoPen);
      painter->setPen(Qt::NoPen);
      QRectF bounds = boundingRect();
      painter->drawRoundedRect(bounds, roundRadius, roundRadius);
      
//      const QString pitchNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
      const QString pitchNames[] = {"c", "c#", "d", "d#", "e", "f", "f#", "g", "g#", "a", "a#", "b"};

      //Pitch name
      if (bounds.width() >= 20 && bounds.height() >= 11)
            {
            QFont f("FreeSans",  8);
            painter->setFont(f);
            
            painter->setPen(QPen(noteColor.lighter(130)));
            painter->drawText(bounds.x() + 3, bounds.y()+1, bounds.width(), bounds.height(),
                    Qt::AlignLeft | Qt::AlignTop, pitchNames[pitch % 12]);
            
            painter->setPen(QPen(noteColor.darker(180)));
            painter->drawText(bounds.x() + 2, bounds.y(), bounds.width(), bounds.height(),
                    Qt::AlignLeft | Qt::AlignTop, pitchNames[pitch % 12]);
            

//            QRectF rectText(bounds.x() + 2, bounds.y(), bounds.width(), bounds.height());
//            painter->drawText(rectText, Qt::AlignLeft | Qt::AlignTop, pitchNames[pitch % 12]);
            
            //Black key hint
            if (isBlack && bounds.width() >= 30)
                  painter->drawText(bounds.x() + 2, bounds.y(), bounds.width(), bounds.height(),
                          Qt::AlignRight | Qt::AlignVCenter, QString("#  "));
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
      setRubberBandSelectionMode(Qt::IntersectsItemBoundingRect);
      setDragMode(QGraphicsView::RubberBandDrag);
      _timeType = TType::TICKS;
      magStep   = 0;
      staff     = 0;
      chord     = 0;
      _noteHeight = DEFAULT_KEY_HEIGHT;
      //Initialize to something that will give us a zoom of around .1
//      _xZoom = (int)(log2(0.1) / log2(X_ZOOM_RATIO));  
      _xZoom = 0.1;
      }

//---------------------------------------------------------
//   pix2pos
//---------------------------------------------------------

//Pos PianoView::pix2pos(int x) const
//      {
//      int ticks = (int)(x / ticksToPixelsRatio()) - MAP_OFFSET;
//      if (ticks < 0)
//            ticks = 0;
//      return Pos(staff->score()->tempomap(), staff->score()->sigmap(), ticks, TType::TICKS);
//      
//      
////      x -= MAP_OFFSET;
////      if (x < 0)
////            x = 0;
////      return Pos(staff->score()->tempomap(), staff->score()->sigmap(), x, _timeType);
//      }

//---------------------------------------------------------
//   pos2pix
//---------------------------------------------------------

//int PianoView::pos2pix(const Pos& p) const
//      {
//      return (int)((p.time(TType::TICKS) + MAP_OFFSET) * ticksToPixelsRatio());
//      
////      return p.time(_timeType) + MAP_OFFSET;
//      }

//---------------------------------------------------------
//   drawBackground
//---------------------------------------------------------

void PianoView::drawBackground(QPainter* p, const QRectF& r)
      {
      if (staff == 0)
            return;
      Score* _score = staff->score();
      setFrameShape(QFrame::NoFrame);

      QColor colGutter = colPianoBg.darker(150);
      QColor colBlackKeyBg = colPianoBg.darker(130);
      QColor colGridLineMajor = colPianoBg.darker(180);
      QColor colGridLineMinor = colPianoBg.darker(160);
      QPen penLineMajor = QPen(colGridLineMajor, 2.0, Qt::SolidLine);
      QPen penLineMinor = QPen(colGridLineMinor, 1.0, Qt::SolidLine);

      //Ticks to pixels
      //qreal ticks2Pix = _xZoom;
      
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
      
      /*
      int key = floor(y1 / kh);
      qreal y = key * kh;

      for (; key < 75; ++key, y += kh) {
            if (y < y1)
                  continue;
            if (y > y2)
                  break;
            p->setPen(QPen((key % 7) == 5 ? Qt::lightGray : Qt::gray));
            p->drawLine(QLineF(x1, y, x2, y));
            }
       */

      //
      // draw vertical grid lines
      //
      

//      printf("xScale: %f, xZoom: %d, zoomRatio:%f\n", ticks2Pix, xZoom, X_ZOOM_RATIO);
      
//      xScale = .1;
      
//      qreal xScaleInv = 1 / xScale;
//      qreal beatWidth = BEAT_WIDTH_IN_PIXELS * xScale;
      
//      int tick1 = (int)(x1 * xScaleInv - MAP_OFFSET);
//      int tick2 = (int)(x2 * xScaleInv - MAP_OFFSET);
      
//      Pos pos1(_score->tempomap(), _score->sigmap(), qMax((int)(x1 / ticks2Pix) - MAP_OFFSET, 0), TType::TICKS);
//      Pos pos2(_score->tempomap(), _score->sigmap(), qMax((int)(x2 / ticks2Pix) - MAP_OFFSET, 0), TType::TICKS);
      Pos pos1(_score->tempomap(), _score->sigmap(), qMax(pixelXToTick(x1), 0), TType::TICKS);
      Pos pos2(_score->tempomap(), _score->sigmap(), qMax(pixelXToTick(x2), 0), TType::TICKS);
      
//      Pos pos1 = pix2pos(x1 / xScale);
//      Pos pos2 = pix2pos(x2 / xScale);
//      Pos pos1 = pix2pos(x1);
//      Pos pos2 = pix2pos(x2);

      
      int bar1, bar2, beat, tick;
      pos1.mbt(&bar1, &beat, &tick);
//      printf("bar1: %d  beat:%d  tick:%d\n", bar1, beat, tick);
      pos2.mbt(&bar2, &beat, &tick);
//      printf("bar2: %d  beat:%d  tick:%d\n", bar2, beat, tick);
      
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
      
      
      
      //---------------------------------------------------
      //    draw raster
      //---------------------------------------------------
/*
      static const int mag[7] = {
            1, 1, 2, 5, 10, 20, 50
            };
      
      int n = mag[magStep < 0 ? 0 : magStep];

      bar1 = (bar1 / n) * n;           // round down
      if (bar1 && n >= 2)
            bar1 -= 1;
      bar2 = ((bar2 + n - 1) / n) * n; // round up

      for (int bar = bar1; bar <= bar2;) {
            Pos stick(_score->tempomap(), _score->sigmap(), bar, 0, 0);
            if (magStep > 0) {
                  double x = double(pos2pix(stick));
                  if (x > 0) {
                        p->setPen(QPen(colGridLineMajor, 0.0));
                        p->drawLine(x, y1, x, y2);
                        }
                  else {
                        p->setPen(QPen(Qt::black, 0.0));
                        p->drawLine(x, y1, x, y1);
                        }
                  }
            else {
                  int z = stick.timesig().timesig().numerator();
                  for (int beat = 0; beat < z; beat++) {
                        if (magStep == 0) {
                              Pos xx(_score->tempomap(), _score->sigmap(), bar, beat, 0);
                              int xp = pos2pix(xx);
                              if (xp < 0)
                                    continue;
                              if (xp > 0) {
//                                    p->setPen(QPen(beat == 0 ? colGridLineMajor : colGridLineMinor, 0.0));
//                                    p->setPen(QPen(beat == 0 ? colGridLineMajor : colGridLineMinor, 5.0));
                                    p->setPen(beat == 0 ? penLineMajor : penLineMinor);
                                    p->drawLine(xp, y1, xp, y2);
//                                    p->drawLine(xp, y1, xp + 100, y1 + 100);
                                    }
                              else {
                                    p->setPen(QPen(Qt::black, 0.0));
                                    p->drawLine(xp, y1, xp, y2);
                                    }
                              }
                        else {
                              int k;
                              if (magStep == -1)
                                    k = 2;
                              else if (magStep == -2)
                                    k = 4;
                              else if (magStep == -3)
                                    k = 8;
                              else if (magStep == -4)
                                    k = 16;
                              else
                                    k = 32;

                              int n = (MScore::division * 4) / stick.timesig().timesig().denominator();
                              for (int i = 0; i < k; ++i) {
                                    Pos xx(_score->tempomap(), _score->sigmap(), bar, beat, (n * i)/ k);
                                    int xp = pos2pix(xx);
                                    if (xp < 0)
                                          continue;
                                    if (xp > 0) {
                                          p->setPen(QPen(i == 0 && beat == 0 ? colGridLineMajor : colGridLineMinor, 0.0));
                                          p->drawLine(xp, y1, xp, y2);
                                          }
                                    else {
                                          p->setPen(QPen(Qt::black, 0.0));
                                          p->drawLine(xp, y1, xp, y2);
                                          }
                                    }
                              }
                        }
                  }
            if (bar == 0 && n >= 2)
                  bar += (n-1);
            else
                  bar += n;
            }
      */
      
//      colGutter = Qt::green;
//      if (r.intersects(r1))
//            p->fillRect(r.intersected(r1), colGutter);
//      if (r.intersects(r2))
//            p->fillRect(r.intersected(r2), colGutter);
      
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
//            qreal x = (_locator[i].time(TType::TICKS) + MAP_OFFSET) * _xZoom;
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
//            printf("wheelEvent viewRect %f %f %f %f\n", viewRect.x(), viewRect.y(), viewRect.width(), viewRect.height());
//            printf("mousePos:%d %d\n", event->x(), event->y());
            
//            qreal mouseXTick = event->x() / _xZoom - MAP_OFFSET;
            int mouseXTick = pixelXToTick(event->x() + (int)viewRect.x());
            //int viewportX = event->x() - viewRect.x();
//            printf("mouseTick:%d viewportX:%d\n", mouseXTick, viewportX);
//            printf("mouseTick:%d\n", mouseXTick);
            
            _xZoom *= pow(X_ZOOM_RATIO, step);
            emit xZoomChanged(_xZoom);
            
            updateBoundingSize();
            updateNotes();
            
            int mousePixX = tickToPixelX(mouseXTick);
  //          printf("mousePixX:%d \n", mousePixX);
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
            //QPointF mouseScenePos = mapToScene(event->pos());
            qreal mouseYNote = (event->y() + (int)viewRect.y()) / (qreal)_noteHeight;
            
            _noteHeight = qMax(qMin(_noteHeight + step, MAX_KEY_HEIGHT), MIN_KEY_HEIGHT);
            emit noteHeightChanged(_noteHeight);
            
            updateBoundingSize();
            updateNotes();
            
            int mousePixY = (int)(mouseYNote * _noteHeight);
  //          printf("mousePixX:%d \n", mousePixX);
            verticalScrollBar()->setValue(mousePixY - event->y());
            
            
            scene()->update();
            }
      }

//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void PianoView::mouseMoveEvent(QMouseEvent* event)
      {
      QPointF p(mapToScene(event->pos()));
      int pitch = (int)((_noteHeight * 128 - p.y()) / _noteHeight);
//      int pitch = (_noteHeight * 128 - event->y()) / _noteHeight;
      emit pitchChanged(pitch);
      
      //int tick = event->x() / _xZoom - MAP_OFFSET;
      int tick = pixelXToTick(p.x());
//      int tick = int(p.x()) -480;
      if (tick < 0) {
            tick = 0;
            pos.setTick(tick);
            pos.setInvalid();
            }
      else
            pos.setTick(tick);
      emit posChanged(pos);
      QGraphicsView::mouseMoveEvent(event);
      }

//---------------------------------------------------------
//   leaveEvent
//---------------------------------------------------------

void PianoView::leaveEvent(QEvent* event)
      {
      emit pitchChanged(-1);
      pos.setInvalid();
      emit posChanged(pos);
      QGraphicsView::leaveEvent(event);
      }

//---------------------------------------------------------
//   ensureVisible
//---------------------------------------------------------

void PianoView::scrollToTick(int tick)
      {
      QRectF rect = mapToScene(viewport()->geometry()).boundingRect();
//      printf("scrollToTick x:%f y:%f width:%f height:%f \n", rect.x(), rect.y(), rect.width(), rect.height());
      
//      int ypos = verticalScrollBar()->value();
//      printf("ensureVisible tick:%d ypos:%d \n", tick, ypos);
      //QRectF rect = sceneRect();
      //QRectF rect = mapToScene(viewport()->geometry()).boundingRect();
//      printf("ensureVisible x:%f y:%f width:%f height:%f \n", rect.x(), rect.y(), rect.width(), rect.height());
      
      //QPointF pt = mapToScene(0, height() / 2);
      //qreal xpos = (tick + MAP_OFFSET) * _xZoom;
      qreal xpos = tickToPixelX(tick);
//      qreal ypos = matrix().dy();
//      QGraphicsView::ensureVisible(xpos, ypos, 240.0, 1.0);

//      horizontalScrollBar()->setValue(qMax(xpos - rect.width() / 2, 0.0));
      
      
      qreal margin = rect.width() / 2;
      if (xpos < rect.x() + margin)
            horizontalScrollBar()->setValue(qMax(xpos - margin, 0.0));
      else if (xpos >= rect.x() + rect.width() - margin)
            horizontalScrollBar()->setValue(qMax(xpos - rect.width() + margin, 0.0));
            
      
//      horizontalScrollBar()->setValue(qMax(xpos - margin, 0.0));
//      tick += MAP_OFFSET;
//      QPointF pt = mapToScene(0, height() / 2);
//      QGraphicsView::ensureVisible(qreal(tick), pt.y(), 240.0, 1.0);
//      verticalScrollBar()->setValue(ypos);
      }

//---------------------------------------------------------
//   updateBoundingSize
//---------------------------------------------------------
void PianoView::updateBoundingSize()
      {
      Measure* lm = staff->score()->lastMeasure();
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
      staff    = s;
      _locator = l;
      setEnabled(staff != nullptr);
      if (!staff) {
            scene()->blockSignals(true);  // block changeSelection()
            scene()->clear();
            scene()->blockSignals(false);
            return;
            }

      pos.setContext(staff->score()->tempomap(), staff->score()->sigmap());
//      Measure* lm = staff->score()->lastMeasure();
//      ticks       = lm->tick() + lm->ticks();
//      scene()->setSceneRect(0.0, 0.0, double(ticks + MAP_OFFSET * 2), keyHeight * 75);
      updateBoundingSize();

      updateNotes();

      //
      // move to something interesting
      //
      QList<QGraphicsItem*> items = scene()->items();
//      QList<QGraphicsItem*> items = scene()->selectedItems();
//      if (!items.empty())
//            {
//            items = scene()->items();
//            }
      
      QRectF boundingRect;
      bool brInit = false;
      QRectF boundingRectSel;
      bool brsInit = false;
      
      foreach (QGraphicsItem* item, items) {
            if (item->type() == PianoItemType)
                  {
                  if (!brInit)
                        {
                        boundingRect = item->boundingRect();
                        brInit = true;
                        }
                  else
                        {
                        boundingRect |= item->mapToScene(item->boundingRect()).boundingRect();
                        }
                  
                  if (item->isSelected())
                        {
                        if (!brsInit)
                              {
                              boundingRectSel = item->boundingRect();
                              brsInit = true;
                              }
                        else
                              {
                              boundingRectSel |= item->mapToScene(item->boundingRect()).boundingRect();
                              }
                        }
                  
                  //PianoItem *pi = (PianoItem)item;
//                  item->isSelected();
//                  boundingRect |= item->mapToScene(item->boundingRect()).boundingRect();
                  }
            }
      
      if (brsInit)
            {
            horizontalScrollBar()->setValue(boundingRectSel.x());
            verticalScrollBar()->setValue(qMax(boundingRectSel.y() - boundingRectSel.height() / 2, 0.0));
            }
      else if (brInit)
            {
            horizontalScrollBar()->setValue(boundingRect.x());
            verticalScrollBar()->setValue(qMax(boundingRect.y() - boundingRect.height() / 2, 0.0));
            }
      else
            {
            QRectF rect = mapToScene(viewport()->geometry()).boundingRect();
            
            horizontalScrollBar()->setValue(0);
            verticalScrollBar()->setValue(qMax(rect.y() - rect.height() / 2, 0.0));
            }
      //centerOn(boundingRect.center());
//      horizontalScrollBar()->setValue(0);
//      verticalScrollBar()->setValue(qMax(boundingRect.y() - boundingRect.height() / 2, 0.0));
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
                  scene()->addItem(new PianoItem(note, &e, this));
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
      createLocators();

      int staffIdx   = staff->idx();
      int startTrack = staffIdx * VOICES;
      int endTrack   = startTrack + VOICES;

      SegmentType st = SegmentType::ChordRest;
      for (Segment* s = staff->score()->firstSegment(st); s; s = s->next1(st)) {
            for (int track = startTrack; track < endTrack; ++track) {
                  Element* e = s->element(track);
                  if (e && e->isChord())
                        addChord(toChord(e));
                  }
            }
      for (int i = 0; i < 3; ++i)
            moveLocator(i);
      scene()->blockSignals(false);
      }
}

