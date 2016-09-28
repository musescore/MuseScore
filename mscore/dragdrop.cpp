//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "libmscore/score.h"
#include "libmscore/element.h"
#include "libmscore/note.h"
#include "libmscore/rest.h"
#include "libmscore/measure.h"
#include "libmscore/system.h"
#include "libmscore/segment.h"
#include "libmscore/page.h"
#include "libmscore/image.h"
#include "libmscore/text.h"
#include "libmscore/spanner.h"
#include "libmscore/chord.h"
#include "libmscore/icon.h"
#include "libmscore/xml.h"
#include "musescore.h"
#include "scoreview.h"
#include "continuouspanel.h"

namespace Ms {

//---------------------------------------------------------
//   moveElement
//---------------------------------------------------------

static void moveElement(void* data, Element* e)
      {
      QPointF* pos = (QPointF*)data;
      e->score()->addRefresh(e->canvasBoundingRect());
      e->setPos(*pos);
      e->score()->addRefresh(e->canvasBoundingRect());
      }

//---------------------------------------------------------
//   setDropTarget
//---------------------------------------------------------

void ScoreView::setDropTarget(const Element* el)
      {
      if (dropTarget != el) {
            if (dropTarget) {
                  dropTarget->setDropTarget(false);
                  _score->addRefresh(dropTarget->canvasBoundingRect());
                  dropTarget = 0;
                  }
            dropTarget = el;
            if (dropTarget) {
                  dropTarget->setDropTarget(true);
                  _score->addRefresh(dropTarget->canvasBoundingRect());
                  }
            }
      if (!dropAnchor.isNull()) {
            QRectF r;
            r.setTopLeft(dropAnchor.p1());
            r.setBottomRight(dropAnchor.p2());
            _score->addRefresh(r.normalized());
            dropAnchor = QLineF();
            }
      if (dropRectangle.isValid()) {
            _score->addRefresh(dropRectangle);
            dropRectangle = QRectF();
            }
      }

//---------------------------------------------------------
//   setDropRectangle
//---------------------------------------------------------

void ScoreView::setDropRectangle(const QRectF& r)
      {
      if (dropRectangle.isValid())
            _score->addRefresh(dropRectangle);
      dropRectangle = r;
      if (dropTarget) {
            dropTarget->setDropTarget(false);
            _score->addRefresh(dropTarget->canvasBoundingRect());
            dropTarget = 0;
            }
      else if (!dropAnchor.isNull()) {
            QRectF r;
            r.setTopLeft(dropAnchor.p1());
            r.setBottomRight(dropAnchor.p2());
            _score->addRefresh(r.normalized());
            dropAnchor = QLineF();
            }
      _score->addRefresh(r);
      }

//---------------------------------------------------------
//   setDropAnchor
//---------------------------------------------------------

void ScoreView::setDropAnchor(const QLineF& l)
      {
      if (!dropAnchor.isNull()) {
            qreal w = 2 / _matrix.m11();
            QRectF r;
            r.setTopLeft(dropAnchor.p1());
            r.setBottomRight(dropAnchor.p2());
            r = r.normalized();
            r.adjust(-w, -w, 2*w, 2*w);
            _score->addRefresh(r);
            }
/*      if (dropTarget) {
            dropTarget->setDropTarget(false);
            _score->addRefresh(dropTarget->canvasBoundingRect());
            dropTarget = 0;
            }
      */
      if (dropRectangle.isValid()) {
            _score->addRefresh(dropRectangle);
            dropRectangle = QRectF();
            }
      dropAnchor = l;
      if (!dropAnchor.isNull()) {
            qreal w = 2 / _matrix.m11();
            QRectF r;
            r.setTopLeft(dropAnchor.p1());
            r.setBottomRight(dropAnchor.p2());
            r = r.normalized();
            r.adjust(-w, -w, 2*w, 2*w);
            _score->addRefresh(r);
            }
      }

//---------------------------------------------------------
//   setViewRect
//---------------------------------------------------------

void ScoreView::setViewRect(const QRectF& r)
      {
      QRectF rr = _matrix.mapRect(r);
      QPoint d = rr.topLeft().toPoint();
      int dx   = -d.x();
      int dy   = -d.y();
      QApplication::sendPostedEvents(this, 0);
      _matrix.setMatrix(_matrix.m11(), _matrix.m12(), _matrix.m13(), _matrix.m21(),
         _matrix.m22(), _matrix.m23(), _matrix.dx()+dx, _matrix.dy()+dy, _matrix.m33());
      imatrix = _matrix.inverted();
      scroll(dx, dy, QRect(0, 0, width(), height()));
      emit offsetChanged(_matrix.dx(), _matrix.dy());
      if (_continuousPanel->visible())
            update();
      }

//---------------------------------------------------------
//   dragTimeAnchorElement
//    pos is in canvas coordinates
//---------------------------------------------------------

void ScoreView::dragTimeAnchorElement(const QPointF& pos)
      {
      int staffIdx;
      Segment* seg;
      MeasureBase* mb = _score->pos2measure(pos, &staffIdx, 0, &seg, 0);
      int track  = staffIdx * VOICES;
      if (mb && mb->type() == Element::Type::MEASURE && seg->element(track)) {
            Measure* m = static_cast<Measure*>(mb);
            System* s  = m->system();
            qreal y    = s->staff(staffIdx)->y() + s->pos().y() + s->page()->pos().y();
            QPointF anchor(seg->canvasBoundingRect().x(), y);
            setDropAnchor(QLineF(pos, anchor));
            dragElement->score()->addRefresh(dragElement->canvasBoundingRect());
            dragElement->setTrack(track);
            dragElement->score()->addRefresh(dragElement->canvasBoundingRect());
            return;
            }
      dragElement->score()->addRefresh(dragElement->canvasBoundingRect());
      setDropTarget(0);
      }

//---------------------------------------------------------
//   dragMeasureAnchorElement
//---------------------------------------------------------

bool ScoreView::dragMeasureAnchorElement(const QPointF& pos)
      {
      Measure* m = _score->searchMeasure(pos);
      if (m) {
            QRectF b(m->canvasBoundingRect());

            QPointF anchor;
            if (pos.x() < (b.x() + b.width() * .5) || m == _score->lastMeasureMM())
                  anchor = m->canvasBoundingRect().topLeft();
            else
                  anchor = m->canvasBoundingRect().topRight();
            setDropAnchor(QLineF(pos, anchor));
            return true;
            }
      dragElement->score()->addRefresh(dragElement->canvasBoundingRect());
      setDropTarget(0);
      return false;
      }

//---------------------------------------------------------
//   dragEnterEvent
//---------------------------------------------------------

void ScoreView::dragEnterEvent(QDragEnterEvent* event)
      {
      if (MScore::debugMode)
            qDebug("dragEnterEvent");
      double _spatium = score()->spatium();
      dragElement = 0;

      const QMimeData* data = event->mimeData();

      if (data->hasFormat(mimeSymbolListFormat) || data->hasFormat(mimeStaffListFormat)) {
            event->acceptProposedAction();
            return;
            }

      if (data->hasFormat(mimeSymbolFormat)) {
            event->acceptProposedAction();

            QByteArray a = data->data(mimeSymbolFormat);

            if (MScore::debugMode)
                  qDebug("ScoreView::dragEnterEvent Symbol: <%s>", a.data());

            XmlReader e(a);
            dragOffset = QPoint();
            Fraction duration;  // dummy
            Element::Type type = Element::readType(e, &dragOffset, &duration);

            Element* el = Element::create(type, score());
            if (el) {
                  if (type == Element::Type::BAR_LINE || type == Element::Type::ARPEGGIO || type == Element::Type::BRACKET)
                        el->setHeight(_spatium * 5);
                  dragElement = el;
                  dragElement->setParent(0);
                  dragElement->read(e);
                  dragElement->layout();
                  }
            return;
            }

      if (data->hasUrls()) {
            QList<QUrl>ul = data->urls();
            foreach(const QUrl& u, ul) {
                  if (MScore::debugMode)
                        qDebug("drag Url: %s", qPrintable(u.toString()));
                  if (u.scheme() == "file" || u.scheme() == "http") {
                        QFileInfo fi(u.path());
                        QString suffix = fi.suffix().toLower();
                        if (suffix == "svg"
                           || suffix == "jpg"
                           || suffix == "jpeg"
                           || suffix == "png"
                           ) {
                              event->acceptProposedAction();
                              break;
                              }
                        }
                  }
            return;
            }
      QStringList formats = data->formats();
      qDebug("unknown drop format: formats:");
      foreach(const QString& s, formats)
            qDebug("  <%s>", qPrintable(s));
      }

//---------------------------------------------------------
//   dragSymbol
//    drag SYMBOL and IMAGE elements
//---------------------------------------------------------

void ScoreView::dragSymbol(const QPointF& pos)
      {
      const QList<Element*> el = elementsAt(pos);
      const Element* e = el.isEmpty() ? 0 : el[0];
      if (e && (e->type() == Element::Type::NOTE || e->type() == Element::Type::SYMBOL
         || e->type() == Element::Type::IMAGE || e->type() == Element::Type::TEXT)) {
            DropData dropData;
            dropData.view       = this;
            dropData.pos        = pos;
            dropData.element    = dragElement;
            dropData.modifiers  = 0;

            if (e->acceptDrop(dropData)) {
                  setDropTarget(e);
                  return;
                  }
            else {
                  setDropTarget(0);
                  return;
                  }
            }
      dragTimeAnchorElement(pos);
      }

//---------------------------------------------------------
//   dragMoveEvent
//---------------------------------------------------------

void ScoreView::dragMoveEvent(QDragMoveEvent* event)
      {
      // we always accept the drop action
      // to get a "drop" Event:

      event->acceptProposedAction();
      if (mscore->state() == STATE_PLAY)  // no editing during play
            return;

      // convert window to canvas position
      QPointF pos(imatrix.map(QPointF(event->pos())));

      DropData dropData;
      dropData.view       = this;
      dropData.pos        = pos;
      dropData.dragOffset = dragOffset;
      dropData.element    = dragElement;
      dropData.modifiers  = event->keyboardModifiers();

      if (dragElement) {
            switch(dragElement->type()) {
                  case Element::Type::VOLTA:
                        // dragMeasureAnchorElement(pos);
                        // break;
                  case Element::Type::PEDAL:
                  case Element::Type::DYNAMIC:
                  case Element::Type::OTTAVA:
                  case Element::Type::TRILL:
                  case Element::Type::HAIRPIN:
                  case Element::Type::TEXTLINE:
                  case Element::Type::FRET_DIAGRAM:
                        dragTimeAnchorElement(pos);
                        break;
                  case Element::Type::IMAGE:
                  case Element::Type::SYMBOL:
                        dragSymbol(pos);
                        break;
                  case Element::Type::KEYSIG:
                  case Element::Type::CLEF:
                  case Element::Type::TIMESIG:
                  case Element::Type::BAR_LINE:
                  case Element::Type::ARPEGGIO:
                  case Element::Type::BREATH:
                  case Element::Type::GLISSANDO:
                  case Element::Type::BRACKET:
                  case Element::Type::ARTICULATION:
                  case Element::Type::CHORDLINE:
                  case Element::Type::BEND:
                  case Element::Type::ACCIDENTAL:
                  case Element::Type::TEXT:
                  case Element::Type::FINGERING:
                  case Element::Type::TEMPO_TEXT:
                  case Element::Type::STAFF_TEXT:
                  case Element::Type::NOTEHEAD:
                  case Element::Type::TREMOLO:
                  case Element::Type::LAYOUT_BREAK:
                  case Element::Type::MARKER:
                  case Element::Type::STAFF_STATE:
                  case Element::Type::INSTRUMENT_CHANGE:
                  case Element::Type::REHEARSAL_MARK:
                  case Element::Type::JUMP:
                  case Element::Type::REPEAT_MEASURE:
                  case Element::Type::ICON:
                  case Element::Type::CHORD:
                  case Element::Type::SPACER:
                  case Element::Type::SLUR:
                  case Element::Type::HARMONY:
                  case Element::Type::BAGPIPE_EMBELLISHMENT:
                  case Element::Type::AMBITUS:
                  case Element::Type::TREMOLOBAR:
                  case Element::Type::FIGURED_BASS:
                  case Element::Type::LYRICS:
                        {
                        QList<Element*> el = elementsAt(pos);
                        bool found = false;
                        foreach(const Element* e, el) {
                              if (e->acceptDrop(dropData)) {
                                    if (e->type() != Element::Type::MEASURE)
                                          setDropTarget(const_cast<Element*>(e));
                                    found = true;
                                    break;
                                    }
                              }
                        if (!found)
                              setDropTarget(0);
                        }
                        break;
                  default:
                        break;
                  }

            dragElement->scanElements(&pos, moveElement, false);
            _score->update();
            return;
            }

      const QMimeData* md = event->mimeData();
      if (md->hasUrls()) {
            QList<QUrl>ul = md->urls();
            QUrl u = ul.front();
            if (u.scheme() == "file" || u.scheme() == "http") {
                  QFileInfo fi(u.path());
                  QString suffix(fi.suffix().toLower());
                  if (suffix != "svg"
                     && suffix != "jpg"
                     && suffix != "jpeg"
                     && suffix != "png"
                     ) {
                        return;
                        }
                  //
                  // special drop target Note
                  //
                  Element* el = elementAt(pos);
                  if (el && (el->type() == Element::Type::NOTE || el->type() == Element::Type::REST))
                        setDropTarget(el);
                  else
                        setDropTarget(0);
                  }
            _score->update();
            return;
            }
      QByteArray data;
      Element::Type etype;
      if (md->hasFormat(mimeSymbolListFormat)) {
            etype = Element::Type::ELEMENT_LIST;
            data = md->data(mimeSymbolListFormat);
            }
      else if (md->hasFormat(mimeStaffListFormat)) {
            etype = Element::Type::STAFF_LIST;
            data = md->data(mimeStaffListFormat);
            }
      else {
            _score->update();
            return;
            }
      Element* el = elementAt(pos);
      if (el == 0 || el->type() != Element::Type::MEASURE) {
            _score->update();
            return;
            }
      else if (etype == Element::Type::ELEMENT_LIST) {
            qDebug("accept drop element list");
            }
      else if (etype == Element::Type::STAFF_LIST || etype == Element::Type::MEASURE_LIST) {
//TODO            el->acceptDrop(this, pos, etype, e);
            }
      _score->update();
      }

//---------------------------------------------------------
//   dropEvent
//---------------------------------------------------------

void ScoreView::dropEvent(QDropEvent* event)
      {
      QPointF pos(imatrix.map(QPointF(event->pos())));

      DropData dropData;
      dropData.view       = this;
      dropData.pos        = pos;
      dropData.dragOffset = dragOffset;
      dropData.element    = dragElement;
      dropData.modifiers  = event->keyboardModifiers();

      if (dragElement) {
            bool applyUserOffset = false;
            _score->startCmd();
            Q_ASSERT(dragElement->score() == score());
            _score->addRefresh(dragElement->canvasBoundingRect());
            switch (dragElement->type()) {
                  case Element::Type::VOLTA:
                  case Element::Type::OTTAVA:
                  case Element::Type::TRILL:
                  case Element::Type::PEDAL:
                  case Element::Type::HAIRPIN:
                  case Element::Type::TEXTLINE:
                        {
                        Spanner* spanner = static_cast<Spanner*>(dragElement);
                        score()->cmdAddSpanner(spanner, pos);
                        score()->setUpdateAll();
                        event->acceptProposedAction();
                        }
                        break;
                  case Element::Type::SYMBOL:
                  case Element::Type::IMAGE:
                        applyUserOffset = true;
                        // fall-thru
                  case Element::Type::DYNAMIC:
                  case Element::Type::FRET_DIAGRAM:
                  case Element::Type::HARMONY:
                        {
                        Element* el = elementAt(pos);
                        if (el == 0 || el->type() == Element::Type::MEASURE) {
                              int staffIdx;
                              Segment* seg;
                              QPointF offset;
                              el = _score->pos2measure(pos, &staffIdx, 0, &seg, &offset);
                              if (el && el->type() == Element::Type::MEASURE) {
                                    dragElement->setTrack(staffIdx * VOICES);
                                    dragElement->setParent(seg);
                                    if (applyUserOffset)
                                          dragElement->setUserOff(offset);
                                    score()->undoAddElement(dragElement);
                                    }
                              else {
                                    qDebug("cannot drop here");
                                    delete dragElement;
                                    }
                              }
                        else {
                              _score->addRefresh(el->canvasBoundingRect());
                              _score->addRefresh(dragElement->canvasBoundingRect());

                              if (!el->acceptDrop(dropData)) {
                                    qDebug("drop %s onto %s not accepted", dragElement->name(), el->name());
                                    break;
                                    }
                              Element* dropElement = el->drop(dropData);
                              _score->addRefresh(el->canvasBoundingRect());
                              if (dropElement) {
                                    _score->select(dropElement, SelectType::SINGLE, 0);
                                    _score->addRefresh(dropElement->canvasBoundingRect());
                                    }
                              }
                        }
                        event->acceptProposedAction();
                        break;
                  case Element::Type::KEYSIG:
                  case Element::Type::CLEF:
                  case Element::Type::TIMESIG:
                  case Element::Type::BAR_LINE:
                  case Element::Type::ARPEGGIO:
                  case Element::Type::BREATH:
                  case Element::Type::GLISSANDO:
                  case Element::Type::BRACKET:
                  case Element::Type::ARTICULATION:
                  case Element::Type::CHORDLINE:
                  case Element::Type::BEND:
                  case Element::Type::ACCIDENTAL:
                  case Element::Type::TEXT:
                  case Element::Type::FINGERING:
                  case Element::Type::TEMPO_TEXT:
                  case Element::Type::STAFF_TEXT:
                  case Element::Type::NOTEHEAD:
                  case Element::Type::TREMOLO:
                  case Element::Type::LAYOUT_BREAK:
                  case Element::Type::MARKER:
                  case Element::Type::STAFF_STATE:
                  case Element::Type::INSTRUMENT_CHANGE:
                  case Element::Type::REHEARSAL_MARK:
                  case Element::Type::JUMP:
                  case Element::Type::REPEAT_MEASURE:
                  case Element::Type::ICON:
                  case Element::Type::NOTE:
                  case Element::Type::CHORD:
                  case Element::Type::SPACER:
                  case Element::Type::SLUR:
                  case Element::Type::BAGPIPE_EMBELLISHMENT:
                  case Element::Type::AMBITUS:
                  case Element::Type::TREMOLOBAR:
                  case Element::Type::FIGURED_BASS:
                  case Element::Type::LYRICS:
                        {
                        Element* el = 0;
                        for (const Element* e : elementsAt(pos)) {
                              if (e->acceptDrop(dropData)) {
                                    el = const_cast<Element*>(e);
                                    break;
                                    }
                              }
                        if (!el) {
                              if (!dropCanvas(dragElement)) {
                                    qDebug("cannot drop %s(%p) to canvas", dragElement->name(), dragElement);
                                    delete dragElement;
                                    }
                              break;
                              }
                        _score->addRefresh(el->canvasBoundingRect());

                        // HACK ALERT!
                        if (el->isMeasure() && dragElement->isLayoutBreak()) {
                              Measure* m = toMeasure(el);
                              if (m->isMMRest())
                                    el = m->mmRestLast();
                              }

                        Element* dropElement = el->drop(dropData);
                        _score->addRefresh(el->canvasBoundingRect());
                        if (dropElement) {
                              if (!_score->noteEntryMode())
                                    _score->select(dropElement, SelectType::SINGLE, 0);
                              _score->addRefresh(dropElement->canvasBoundingRect());
                              }
                        event->acceptProposedAction();
                        }
                        break;
                  default:
                        delete dragElement;
                        break;
                  }
            dragElement = 0;
            setDropTarget(0); // this also resets dropRectangle and dropAnchor
            score()->endCmd();
            // update input cursor position (must be done after layout)
            if (noteEntryMode())
                  moveCursor();
            return;
            }

      if (event->mimeData()->hasUrls()) {
            QList<QUrl>ul = event->mimeData()->urls();
            QUrl u = ul.front();
            if (u.scheme() == "file") {
                  QFileInfo fi(u.path());
                  Image* s = new Image(score());
                  _score->startCmd();
                  QString str(u.toLocalFile());
                  s->load(str);
                  qDebug("drop image <%s> <%s>", qPrintable(str), qPrintable(str));

                  Element* el = elementAt(pos);
                  if (el) {
                        dropData.element = s;
                        if (el->acceptDrop(dropData)) {
                              dropData.element = s;
                              el->drop(dropData);
                              }
                        }
                  event->acceptProposedAction();
                  score()->endCmd();
                  setDropTarget(0); // this also resets dropRectangle and dropAnchor
                  return;
                  }
            else if (u.scheme() == "http") {
                  QNetworkAccessManager manager;
                  QNetworkReply* reply = manager.get(QNetworkRequest(u));

                  // TODO:
                  //    feed progress bar in loop
                  //    implement timeout/abort

                  QMutex mutex;
                  QWaitCondition wc;
                  while (!reply->isFinished()) {
                        mutex.lock();
                        wc.wait(&mutex, 100);
                        qApp->processEvents();
                        mutex.unlock();
                        }
                  QByteArray ba = reply->readAll();

                  Image* s = new Image(score());
                  s->loadFromData(u.path(), ba);
                  delete reply;

                  _score->startCmd();

                  Element* el = elementAt(pos);
                  if (el) {
                        dropData.element = s;
                        if (el->acceptDrop(dropData)) {
                              dropData.element = s;
                              el->drop(dropData);
                              }
                        }
                  event->acceptProposedAction();
                  score()->endCmd();
                  setDropTarget(0); // this also resets dropRectangle and dropAnchor
                  return;
                  }
            return;
            }

      dragElement = 0;
      const QMimeData* md = event->mimeData();
      QByteArray data;
      Element::Type etype;
      if (md->hasFormat(mimeSymbolListFormat)) {
            etype = Element::Type::ELEMENT_LIST;
            data = md->data(mimeSymbolListFormat);
            }
      else if (md->hasFormat(mimeStaffListFormat)) {
            etype = Element::Type::STAFF_LIST;
            data = md->data(mimeStaffListFormat);
            }
      else {
            qDebug("cannot drop this object: unknown mime type");
            QStringList sl = md->formats();
            foreach(QString s, sl)
                  qDebug("  %s", qPrintable(s));
            _score->update();
            return;
            }

// qDebug("drop <%s>", data.data());

      Element* el = elementAt(pos);
      if (el == 0 || el->type() != Element::Type::MEASURE) {
            setDropTarget(0);
            return;
            }
      Measure* measure = (Measure*) el;

      if (etype == Element::Type::ELEMENT_LIST) {
            qDebug("drop element list");
            }
      else if (etype == Element::Type::MEASURE_LIST || etype == Element::Type::STAFF_LIST) {
            _score->startCmd();
            XmlReader xml(data);
            System* s = measure->system();
            int idx   = s->y2staff(pos.y());
            if (idx != -1) {
                  Segment* seg = measure->first();
                  // assume there is always a ChordRest segment
                  while (seg->segmentType() != Segment::Type::ChordRest)
                        seg = seg->next();
                  score()->pasteStaff(xml, seg, idx);
                  }
            event->acceptProposedAction();
            _score->setLayoutAll();
            _score->endCmd();
            }
      setDropTarget(0); // this also resets dropRectangle and dropAnchor
      }

//---------------------------------------------------------
//   dragLeaveEvent
//---------------------------------------------------------

void ScoreView::dragLeaveEvent(QDragLeaveEvent*)
      {
      if (dragElement) {
            _score->setUpdateAll();
            delete dragElement;
            dragElement = 0;
            _score->update();
            }
      setDropTarget(0);
      }

//---------------------------------------------------------
//   dropCanvas
//---------------------------------------------------------

bool ScoreView::dropCanvas(Element* e)
      {
      if (e->type() == Element::Type::ICON) {
            switch(static_cast<Icon*>(e)->iconType()) {
                  case IconType::VFRAME:
                        score()->insertMeasure(Element::Type::VBOX, 0);
                        break;
                  case IconType::HFRAME:
                        score()->insertMeasure(Element::Type::HBOX, 0);
                        break;
                  case IconType::TFRAME:
                        score()->insertMeasure(Element::Type::TBOX, 0);
                        break;
                  case IconType::FFRAME:
                        score()->insertMeasure(Element::Type::FBOX, 0);
                        break;
                  case IconType::MEASURE:
                        score()->insertMeasure(Element::Type::MEASURE, 0);
                        break;
                  default:
                        return false;
                  }
            delete e;
            return true;
            }
      return false;
      }

}

