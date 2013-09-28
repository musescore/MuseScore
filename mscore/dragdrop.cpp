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
#include "musescore.h"
#include "scoreview.h"

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
      if (mb && mb->type() == Element::MEASURE) {
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

// printf("dragEnter: <%s>\n", a.data());

            XmlReader e(a);
            dragOffset = QPoint();
            Fraction duration;  // dummy
            Element::ElementType type = Element::readType(e, &dragOffset, &duration);

            Element* el = Element::create(type, score());
            if (el) {
                  if (type == Element::BAR_LINE || type == Element::ARPEGGIO || type == Element::BRACKET)
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
                           || suffix == "png"
                           || suffix == "gif"
                           || suffix == "xpm"
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
      if (e && (e->type() == Element::NOTE || e->type() == Element::SYMBOL || e->type() == Element::IMAGE)) {
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
                  case Element::VOLTA:
                        dragMeasureAnchorElement(pos);
                        break;
                  case Element::PEDAL:
                  case Element::DYNAMIC:
                  case Element::OTTAVA:
                  case Element::TRILL:
                  case Element::HAIRPIN:
                  case Element::TEXTLINE:
                  case Element::FRET_DIAGRAM:
                        dragTimeAnchorElement(pos);
                        break;
                  case Element::IMAGE:
                  case Element::SYMBOL:
                        dragSymbol(pos);
                        break;
                  case Element::KEYSIG:
                  case Element::CLEF:
                  case Element::TIMESIG:
                  case Element::BAR_LINE:
                  case Element::ARPEGGIO:
                  case Element::BREATH:
                  case Element::GLISSANDO:
                  case Element::Element::BRACKET:
                  case Element::ARTICULATION:
                  case Element::CHORDLINE:
                  case Element::BEND:
                  case Element::ACCIDENTAL:
                  case Element::TEXT:
                  case Element::FINGERING:
                  case Element::TEMPO_TEXT:
                  case Element::STAFF_TEXT:
                  case Element::NOTEHEAD:
                  case Element::TREMOLO:
                  case Element::LAYOUT_BREAK:
                  case Element::MARKER:
                  case Element::STAFF_STATE:
                  case Element::INSTRUMENT_CHANGE:
                  case Element::REHEARSAL_MARK:
                  case Element::JUMP:
                  case Element::REPEAT_MEASURE:
                  case Element::ICON:
                  case Element::CHORD:
                  case Element::SPACER:
                  case Element::SLUR:
                  case Element::ACCIDENTAL_BRACKET:
                  case Element::HARMONY:
                  case Element::BAGPIPE_EMBELLISHMENT:
                        {
                        QList<Element*> el = elementsAt(pos);
                        bool found = false;
                        foreach(const Element* e, el) {
                              if (e->acceptDrop(this, pos, dragElement)) {
                                    if (e->type() != Element::MEASURE)
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
                     && suffix != "png"
                     && suffix != "gif"
                     && suffix != "xpm"
                     )
                        return;
                  //
                  // special drop target Note
                  //
                  Element* el = elementAt(pos);
                  if (el && (el->type() == Element::NOTE || el->type() == Element::REST))
                        setDropTarget(el);
                  else
                        setDropTarget(0);
                  }
            _score->end();
            return;
            }
      const QMimeData* md = event->mimeData();
      QByteArray data;
      int etype;
      if (md->hasFormat(mimeSymbolListFormat)) {
            etype = Element::ELEMENT_LIST;
            data = md->data(mimeSymbolListFormat);
            }
      else if (md->hasFormat(mimeStaffListFormat)) {
            etype = Element::STAFF_LIST;
            data = md->data(mimeStaffListFormat);
            }
      else {
            _score->end();
            return;
            }
      Element* el = elementAt(pos);
      if (el == 0 || el->type() != Element::MEASURE) {
            _score->end();
            return;
            }
      else if (etype == Element::ELEMENT_LIST) {
            qDebug("accept drop element list");
            }
      else if (etype == Element::STAFF_LIST || etype == Element::MEASURE_LIST) {
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
                  case Element::VOLTA:
                  case Element::OTTAVA:
                  case Element::TRILL:
                  case Element::PEDAL:
                  case Element::HAIRPIN:
                  case Element::TEXTLINE:
                        {
                        dragElement->setScore(score());
                        Spanner* spanner = static_cast<Spanner*>(dragElement);
                        score()->cmdAddSpanner(spanner, pos);
                        event->acceptProposedAction();
                        }
                        break;
                  case Element::SYMBOL:
                  case Element::IMAGE:
                  case Element::DYNAMIC:
                  case Element::FRET_DIAGRAM:
                  case Element::HARMONY:
                        {
                        Element* el = elementAt(pos);
                        if (el == 0) {
                              int staffIdx;
                              Segment* seg;
                              el = _score->pos2measure(pos, &staffIdx, 0, &seg, 0);
                              if (el == 0) {
                                    qDebug("cannot drop here");
                                    delete dragElement;
                                    break;
                                    }
                             }
                        _score->addRefresh(el->canvasBoundingRect());
                        _score->addRefresh(dragElement->canvasBoundingRect());

                        if (!el->acceptDrop(this, pos, dragElement))
                              break;
                        Element* dropElement = el->drop(dropData);
                        _score->addRefresh(el->canvasBoundingRect());
                        if (dropElement) {
                              _score->select(dropElement, SELECT_SINGLE, 0);
                              _score->addRefresh(dropElement->canvasBoundingRect());
                              }
                        }
                        event->acceptProposedAction();
                        break;
                  case Element::KEYSIG:
                  case Element::CLEF:
                  case Element::TIMESIG:
                  case Element::BAR_LINE:
                  case Element::ARPEGGIO:
                  case Element::BREATH:
                  case Element::GLISSANDO:
                  case Element::BRACKET:
                  case Element::ARTICULATION:
                  case Element::CHORDLINE:
                  case Element::BEND:
                  case Element::ACCIDENTAL:
                  case Element::TEXT:
                  case Element::FINGERING:
                  case Element::TEMPO_TEXT:
                  case Element::STAFF_TEXT:
                  case Element::NOTEHEAD:
                  case Element::TREMOLO:
                  case Element::LAYOUT_BREAK:
                  case Element::MARKER:
                  case Element::STAFF_STATE:
                  case Element::INSTRUMENT_CHANGE:
                  case Element::REHEARSAL_MARK:
                  case Element::JUMP:
                  case Element::REPEAT_MEASURE:
                  case Element::ICON:
                  case Element::NOTE:
                  case Element::CHORD:
                  case Element::SPACER:
                  case Element::SLUR:
                  case Element::ACCIDENTAL_BRACKET:
                  case Element::BAGPIPE_EMBELLISHMENT:
                        {
                        Element* el = 0;
                        foreach(const Element* e, elementsAt(pos)) {
                              if (e->acceptDrop(this, pos, dragElement)) {
                                    el = const_cast<Element*>(e);
                                    break;
                                    }
                              }
                        if (!el) {
                              if (!dropCanvas(dragElement)) {
                                    qDebug("cannot drop %s to canvas", dragElement->name());
                                    delete dragElement;
                                    dragElement = 0;
                                    }
                              break;
                              }
                        _score->addRefresh(el->canvasBoundingRect());
                        Element* dropElement = el->drop(dropData);
                        _score->addRefresh(el->canvasBoundingRect());
                        if (dropElement) {
                              if (!_score->noteEntryMode())
                                    _score->select(dropElement, SELECT_SINGLE, 0);
                              _score->addRefresh(dropElement->canvasBoundingRect());

#if 0
                              Qt::KeyboardModifiers keyState = event->keyboardModifiers();
                              if (keyState == (Qt::ShiftModifier | Qt::ControlModifier)) {
                                    //
                                    // continue drag with a cloned element
                                    //
//??                                    CloneEvent* ce = new CloneEvent(dropElement);
//                                    QCoreApplication::postEvent(this, ce);
                                    }
#endif
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
      int etype;
      if (md->hasFormat(mimeSymbolListFormat)) {
            etype = Element::ELEMENT_LIST;
            data = md->data(mimeSymbolListFormat);
            }
      else if (md->hasFormat(mimeStaffListFormat)) {
            etype = Element::STAFF_LIST;
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
      if (el == 0 || el->type() != Element::MEASURE) {
            setDropTarget(0);
            return;
            }
      Measure* measure = (Measure*) el;

      if (etype == Element::ELEMENT_LIST) {
            qDebug("drop element list");
            }
      else if (etype == Element::MEASURE_LIST || etype == Element::STAFF_LIST) {
            _score->startCmd();
            XmlReader xml(data);
            System* s = measure->system();
            int idx   = s->y2staff(pos.y());
            if (idx != -1) {
                  Segment* seg = measure->first();
                  // assume there is always a ChordRest segment
                  while (seg->segmentType() != Segment::SegChordRest)
                        seg = seg->next();
                  score()->pasteStaff(xml, (ChordRest*)(seg->element(idx * VOICES)));
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
      if (e->type() == Element::ICON) {
            switch(static_cast<Icon*>(e)->iconType()) {
                  case ICON_VFRAME:
                        score()->insertMeasure(Element::VBOX, 0);
                        break;
                  case ICON_HFRAME:
                        score()->insertMeasure(Element::HBOX, 0);
                        break;
                  case ICON_TFRAME:
                        score()->insertMeasure(Element::TBOX, 0);
                        break;
                  case ICON_FFRAME:
                        score()->insertMeasure(Element::FBOX, 0);
                        break;
                  case ICON_MEASURE:
                        score()->insertMeasure(Element::MEASURE, 0);
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

