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

bool ScoreView::dragTimeAnchorElement(const QPointF& pos)
      {
      int staffIdx;
      Segment* seg;
      MeasureBase* mb = _score->pos2measure(pos, &staffIdx, 0, &seg, 0);
      if (mb && mb->type() == ElementType::MEASURE) {
            Measure* m = static_cast<Measure*>(mb);
            System* s  = m->system();
            qreal y    = s->staff(staffIdx)->y() + s->pos().y() + s->page()->pos().y();
            QPointF anchor(seg->canvasBoundingRect().x(), y);
            setDropAnchor(QLineF(pos, anchor));
            dragElement->setTrack(staffIdx * VOICES);
            return true;
            }
      setDropTarget(0);
      return false;
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
            if (pos.x() < (b.x() + b.width() * .5))
                  anchor = m->canvasBoundingRect().topLeft();
            else
                  anchor = m->canvasBoundingRect().topRight();
            setDropAnchor(QLineF(pos, anchor));
            return true;
            }
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

      if (data->hasFormat(mimeSymbolListFormat)
         || data->hasFormat(mimeStaffListFormat)) {
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
            ElementType type = Element::readType(e, &dragOffset, &duration);

            Element* el = Element::create(type, score());
            if (el) {
                  if (type == ElementType::BAR_LINE || type == ElementType::ARPEGGIO || type == ElementType::BRACKET)
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
                  if (u.scheme() == "file") {
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
      if (e && (e->type() == ElementType::NOTE || e->type() == ElementType::SYMBOL
         || e->type() == ElementType::IMAGE || e->type() == ElementType::TEXT)) {
            if (e->acceptDrop(this, pos, dragElement)) {
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

      if (dragElement) {
            switch(dragElement->type()) {
                  case ElementType::VOLTA:
                        dragMeasureAnchorElement(pos);
                        break;
                  case ElementType::PEDAL:
                  case ElementType::DYNAMIC:
                  case ElementType::OTTAVA:
                  case ElementType::TRILL:
                  case ElementType::HAIRPIN:
                  case ElementType::TEXTLINE:
                  case ElementType::FRET_DIAGRAM:
                        dragTimeAnchorElement(pos);
                        break;
                  case ElementType::IMAGE:
                  case ElementType::SYMBOL:
                        dragSymbol(pos);
                        break;
                  case ElementType::KEYSIG:
                  case ElementType::CLEF:
                  case ElementType::TIMESIG:
                  case ElementType::BAR_LINE:
                  case ElementType::ARPEGGIO:
                  case ElementType::BREATH:
                  case ElementType::GLISSANDO:
                  case ElementType::BRACKET:
                  case ElementType::ARTICULATION:
                  case ElementType::CHORDLINE:
                  case ElementType::BEND:
                  case ElementType::ACCIDENTAL:
                  case ElementType::TEXT:
                  case ElementType::FINGERING:
                  case ElementType::TEMPO_TEXT:
                  case ElementType::STAFF_TEXT:
                  case ElementType::NOTEHEAD:
                  case ElementType::TREMOLO:
                  case ElementType::LAYOUT_BREAK:
                  case ElementType::MARKER:
                  case ElementType::STAFF_STATE:
                  case ElementType::INSTRUMENT_CHANGE:
                  case ElementType::REHEARSAL_MARK:
                  case ElementType::JUMP:
                  case ElementType::REPEAT_MEASURE:
                  case ElementType::ICON:
                  case ElementType::CHORD:
                  case ElementType::SPACER:
                  case ElementType::SLUR:
                  case ElementType::ACCIDENTAL_BRACKET:
                  case ElementType::HARMONY:
                  case ElementType::BAGPIPE_EMBELLISHMENT:
                  case ElementType::AMBITUS:
                        {
                        QList<Element*> el = elementsAt(pos);
                        bool found = false;
                        foreach(const Element* e, el) {
                              if (e->acceptDrop(this, pos, dragElement)) {
                                    if (e->type() != ElementType::MEASURE)
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
            _score->end();
            return;
            }

      if (event->mimeData()->hasUrls()) {
            QList<QUrl>ul = event->mimeData()->urls();
            QUrl u = ul.front();
            if (u.scheme() == "file") {
                  QFileInfo fi(u.path());
                  QString suffix(fi.suffix().toLower());
                  if (suffix != "svg"
                     && suffix != "jpg"
                     && suffix != "jpeg"
                     && suffix != "png"
                     )
                        return;
                  //
                  // special drop target Note
                  //
                  Element* el = elementAt(pos);
                  if (el && (el->type() == ElementType::NOTE || el->type() == ElementType::REST))
                        setDropTarget(el);
                  else
                        setDropTarget(0);
                  }
            _score->end();
            return;
            }
      const QMimeData* md = event->mimeData();
      QByteArray data;
      ElementType etype;
      if (md->hasFormat(mimeSymbolListFormat)) {
            etype = ElementType::ELEMENT_LIST;
            data = md->data(mimeSymbolListFormat);
            }
      else if (md->hasFormat(mimeStaffListFormat)) {
            etype = ElementType::STAFF_LIST;
            data = md->data(mimeStaffListFormat);
            }
      else {
            _score->end();
            return;
            }
      Element* el = elementAt(pos);
      if (el == 0 || el->type() != ElementType::MEASURE) {
            _score->end();
            return;
            }
      else if (etype == ElementType::ELEMENT_LIST) {
            qDebug("accept drop element list");
            }
      else if (etype == ElementType::STAFF_LIST || etype == ElementType::MEASURE_LIST) {
//TODO            el->acceptDrop(this, pos, etype, e);
            }
      _score->end();
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
            _score->startCmd();
            dragElement->setScore(_score);      // CHECK: should already be ok
            _score->addRefresh(dragElement->canvasBoundingRect());
            switch(dragElement->type()) {
                  case ElementType::VOLTA:
                  case ElementType::OTTAVA:
                  case ElementType::TRILL:
                  case ElementType::PEDAL:
                  case ElementType::HAIRPIN:
                  case ElementType::TEXTLINE:
                        {
                        dragElement->setScore(score());
                        Spanner* spanner = static_cast<Spanner*>(dragElement);
                        score()->cmdAddSpanner(spanner, pos);
                        event->acceptProposedAction();
                        }
                        break;
                  case ElementType::SYMBOL:
                  case ElementType::IMAGE:
                  case ElementType::DYNAMIC:
                  case ElementType::FRET_DIAGRAM:
                  case ElementType::HARMONY:
                        {
                        Element* el = elementAt(pos);
                        if (el == 0 || el->type() == ElementType::MEASURE) {
                              int staffIdx;
                              Segment* seg;
                              el = _score->pos2measure(pos, &staffIdx, 0, &seg, 0);
                              if (el && el->type() == ElementType::MEASURE) {
                                    dragElement->setTrack(staffIdx * VOICES);
                                    dragElement->setParent(seg);
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

                              if (!el->acceptDrop(this, pos, dragElement)) {
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
                  case ElementType::KEYSIG:
                  case ElementType::CLEF:
                  case ElementType::TIMESIG:
                  case ElementType::BAR_LINE:
                  case ElementType::ARPEGGIO:
                  case ElementType::BREATH:
                  case ElementType::GLISSANDO:
                  case ElementType::BRACKET:
                  case ElementType::ARTICULATION:
                  case ElementType::CHORDLINE:
                  case ElementType::BEND:
                  case ElementType::ACCIDENTAL:
                  case ElementType::TEXT:
                  case ElementType::FINGERING:
                  case ElementType::TEMPO_TEXT:
                  case ElementType::STAFF_TEXT:
                  case ElementType::NOTEHEAD:
                  case ElementType::TREMOLO:
                  case ElementType::LAYOUT_BREAK:
                  case ElementType::MARKER:
                  case ElementType::STAFF_STATE:
                  case ElementType::INSTRUMENT_CHANGE:
                  case ElementType::REHEARSAL_MARK:
                  case ElementType::JUMP:
                  case ElementType::REPEAT_MEASURE:
                  case ElementType::ICON:
                  case ElementType::NOTE:
                  case ElementType::CHORD:
                  case ElementType::SPACER:
                  case ElementType::SLUR:
                  case ElementType::ACCIDENTAL_BRACKET:
                  case ElementType::BAGPIPE_EMBELLISHMENT:
                  case ElementType::AMBITUS:
                        {
                        Element* el = 0;
                        for (const Element* e : elementsAt(pos)) {
                              if (e->acceptDrop(this, pos, dragElement)) {
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
                        if (el->type() == ElementType::MEASURE && dragElement->type() == ElementType::LAYOUT_BREAK) {
                              Measure* m = static_cast<Measure*>(el);
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
            mscore->endCmd();
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
                        if (el->acceptDrop(this, pos, s)) {
                              dropData.element = s;
                              el->drop(dropData);
                              }
                        }
                  event->acceptProposedAction();
                  score()->endCmd();
                  mscore->endCmd();
                  setDropTarget(0); // this also resets dropRectangle and dropAnchor
                  return;
                  }
            return;
            }

      dragElement = 0;
      const QMimeData* md = event->mimeData();
      QByteArray data;
      ElementType etype;
      if (md->hasFormat(mimeSymbolListFormat)) {
            etype = ElementType::ELEMENT_LIST;
            data = md->data(mimeSymbolListFormat);
            }
      else if (md->hasFormat(mimeStaffListFormat)) {
            etype = ElementType::STAFF_LIST;
            data = md->data(mimeStaffListFormat);
            }
      else {
            qDebug("cannot drop this object: unknown mime type");
            QStringList sl = md->formats();
            foreach(QString s, sl)
                  qDebug("  %s", qPrintable(s));
            _score->end();
            return;
            }

// qDebug("drop <%s>", data.data());

      Element* el = elementAt(pos);
      if (el == 0 || el->type() != ElementType::MEASURE) {
            setDropTarget(0);
            return;
            }
      Measure* measure = (Measure*) el;

      if (etype == ElementType::ELEMENT_LIST) {
            qDebug("drop element list");
            }
      else if (etype == ElementType::MEASURE_LIST || etype == ElementType::STAFF_LIST) {
            _score->startCmd();
            XmlReader xml(data);
            System* s = measure->system();
            int idx   = s->y2staff(pos.y());
            if (idx != -1) {
                  Segment* seg = measure->first();
                  // assume there is always a ChordRest segment
                  while (seg->segmentType() != SegmentType::ChordRest)
                        seg = seg->next();
                  score()->pasteStaff(xml, seg, idx);
                  }
            event->acceptProposedAction();
            _score->setLayoutAll(true);
            _score->endCmd();
            mscore->endCmd();
            }
      setDropTarget(0); // this also resets dropRectangle and dropAnchor
      }

//---------------------------------------------------------
//   dragLeaveEvent
//---------------------------------------------------------

void ScoreView::dragLeaveEvent(QDragLeaveEvent*)
      {
      if (dragElement) {
            _score->setLayoutAll(false);
//            _score->addRefresh(dragElement->canvasBoundingRect());
            _score->setUpdateAll(true);
            delete dragElement;
            dragElement = 0;
            _score->end();
            }
      setDropTarget(0);
      }

//---------------------------------------------------------
//   dropCanvas
//---------------------------------------------------------

bool ScoreView::dropCanvas(Element* e)
      {
      if (e->type() == ElementType::ICON) {
            switch(static_cast<Icon*>(e)->iconType()) {
                  case IconType::VFRAME:
                        score()->insertMeasure(ElementType::VBOX, 0);
                        break;
                  case IconType::HFRAME:
                        score()->insertMeasure(ElementType::HBOX, 0);
                        break;
                  case IconType::TFRAME:
                        score()->insertMeasure(ElementType::TBOX, 0);
                        break;
                  case IconType::FFRAME:
                        score()->insertMeasure(ElementType::FBOX, 0);
                        break;
                  case IconType::MEASURE:
                        score()->insertMeasure(ElementType::MEASURE, 0);
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

