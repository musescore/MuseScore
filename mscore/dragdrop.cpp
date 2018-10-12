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
#include "libmscore/stafflines.h"
#include "musescore.h"
#include "scoreview.h"
#include "continuouspanel.h"

namespace Ms {

//---------------------------------------------------------
//   setDropTarget
//---------------------------------------------------------

void ScoreView::setDropTarget(const Element* el)
      {
      if (dropTarget != el) {
            if (dropTarget) {
                  dropTarget->setDropTarget(false);
                  dropTarget = 0;
                  }
            dropTarget = el;
            if (dropTarget) {
                  dropTarget->setDropTarget(true);
                  }
            }
      if (!dropAnchor.isNull()) {
            QRectF r;
            r.setTopLeft(dropAnchor.p1());
            r.setBottomRight(dropAnchor.p2());
            dropAnchor = QLineF();
            }
      if (dropRectangle.isValid()) {
            dropRectangle = QRectF();
            }
      update();
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
            QRectF rf;
            rf.setTopLeft(dropAnchor.p1());
            rf.setBottomRight(dropAnchor.p2());
            _score->addRefresh(rf.normalized());
            dropAnchor = QLineF();
            }
//      _score->addRefresh(r);
      update();
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
//            _score->addRefresh(r);
            }
      if (dropRectangle.isValid()) {
//            _score->addRefresh(dropRectangle);
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
//            _score->addRefresh(r);
            }
      update();
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
//    return true if there is a valid target
//---------------------------------------------------------

bool ScoreView::dragTimeAnchorElement(const QPointF& pos)
      {
      int staffIdx;
      Segment* seg;
      MeasureBase* mb = _score->pos2measure(pos, &staffIdx, 0, &seg, 0);
      int track  = staffIdx * VOICES;

      if (mb && mb->isMeasure() && seg->element(track)) {
            Measure* m = toMeasure(mb);
            System* s  = m->system();
            qreal y    = s->staff(staffIdx)->y() + s->pos().y() + s->page()->pos().y();
            QPointF anchor(seg->canvasBoundingRect().x(), y);
            setDropAnchor(QLineF(pos, anchor));
            editData.element->score()->addRefresh(editData.element->canvasBoundingRect());
            editData.element->setTrack(track);
            editData.element->score()->addRefresh(editData.element->canvasBoundingRect());
            return true;
            }
      editData.element->score()->addRefresh(editData.element->canvasBoundingRect());
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
            if (pos.x() < (b.x() + b.width() * .5) || m == _score->lastMeasureMM())
                  anchor = m->canvasBoundingRect().topLeft();
            else
                  anchor = m->canvasBoundingRect().topRight();
            setDropAnchor(QLineF(pos, anchor));
            return true;
            }
      editData.element->score()->addRefresh(editData.element->canvasBoundingRect());
      setDropTarget(0);
      return false;
      }

//---------------------------------------------------------
//   dragEnterEvent
//---------------------------------------------------------

void ScoreView::dragEnterEvent(QDragEnterEvent* event)
      {
      double _spatium = score()->spatium();
      editData.element = 0;

      const QMimeData* dta = event->mimeData();

      if (dta->hasFormat(mimeSymbolListFormat) || dta->hasFormat(mimeStaffListFormat)) {
            event->accept();
            return;
            }

      if (dta->hasFormat(mimeSymbolFormat)) {
            event->accept();

            QByteArray a = dta->data(mimeSymbolFormat);

            if (MScore::debugMode)
                  qDebug("ScoreView::dragEnterEvent Symbol: <%s>", a.data());

            XmlReader e(a);
            editData.dragOffset = QPoint();
            Fraction duration;  // dummy
            ElementType type = Element::readType(e, &editData.dragOffset, &duration);

            Element* el = Element::create(type, score());
            if (el) {
                  if (type == ElementType::BAR_LINE || type == ElementType::ARPEGGIO || type == ElementType::BRACKET)
                        el->setHeight(_spatium * 5);
                  editData.element = el;
                  editData.element->setParent(0);
                  editData.element->read(e);
                  editData.element->layout();
                  }
            return;
            }

      if (dta->hasUrls()) {
            QList<QUrl>ul = dta->urls();
            for (const QUrl& u : ul) {
                  if (MScore::debugMode)
                        qDebug("drag Url: %s", qPrintable(u.toString()));
                  if (u.scheme() == "file" || u.scheme() == "http" || u.scheme() == "https") {
                        QFileInfo fi(u.path());
                        QString suffix = fi.suffix().toLower();
                        if (suffix == "svg"
                           || suffix == "jpg"
                           || suffix == "jpeg"
                           || suffix == "png"
                           ) {
                              qDebug("accept <%s>\n", qPrintable(u.toString()));
                              event->accept();
                              break;
                              }
                        }
                  }
            return;
            }
      qDebug("unknown drop format: formats:");
      for (const QString& s : dta->formats())
            qDebug("  <%s>", qPrintable(s));
      event->ignore();
      }

//---------------------------------------------------------
//   getDropTarget
//---------------------------------------------------------

Element* ScoreView::getDropTarget(EditData& ed)
      {
      QList<Element*> el = elementsAt(ed.pos);
      setDropTarget(0);
      for (Element* e : el) {
            if (e->isStaffLines()) {
                  if (el.size() > 2)      // is not first class drop target
                        continue;
                  e = toStaffLines(e)->measure();
                  }
            if (e->acceptDrop(ed)) {
                  if (!e->isMeasure())
                        setDropTarget(e);
                  return e;
                  }
            }
      return nullptr;
      }

//---------------------------------------------------------
//   dragMoveEvent
//---------------------------------------------------------

void ScoreView::dragMoveEvent(QDragMoveEvent* event)
      {
      // we always accept the drop action
      // to get a "drop" Event:

      if (mscore->state() == STATE_PLAY) {  // no editing during play
            event->ignore();
            return;
            }

      // convert window to canvas position
      QPointF pos(imatrix.map(QPointF(event->pos())));

      if (editData.element) {
            switch (editData.element->type()) {
                  case ElementType::IMAGE:
                  case ElementType::SYMBOL:
                        {
                        // dragSymbol(pos);
                        const QList<Element*> el = elementsAt(pos);
                        const Element* e = el.isEmpty() ? 0 : el[0];
                        if (!e) {
                              int staffIdx;
                              e = _score->pos2measure(pos, &staffIdx, 0, 0, 0);
                              }
                        if (e && (e->isNote() || e->isSymbol() || e->isImage() || e->isTextBase())) {
                              EditData dropData(this);
                              dropData.pos        = pos;
                              dropData.element    = editData.element;
                              dropData.modifiers  = 0;

                              if (e->acceptDrop(dropData)) {
                                    setDropTarget(e);
                                    event->accept();
                                    }
                              else {
                                    setDropTarget(0);
                                    event->ignore();
                                    }
                              return;
                              }
                        }
                        // fall through

                  case ElementType::VOLTA:
                  case ElementType::PEDAL:
                  case ElementType::LET_RING:
                  case ElementType::VIBRATO:
                  case ElementType::PALM_MUTE:
                  case ElementType::DYNAMIC:
                  case ElementType::OTTAVA:
                  case ElementType::TRILL:
                  case ElementType::HAIRPIN:
                  case ElementType::TEXTLINE:
                        if (dragTimeAnchorElement(pos))
                              event->accept();
                        else
                              event->ignore();
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
                  case ElementType::FERMATA:
                  case ElementType::CHORDLINE:
                  case ElementType::BEND:
                  case ElementType::ACCIDENTAL:
                  case ElementType::TEXT:
                  case ElementType::FINGERING:
                  case ElementType::TEMPO_TEXT:
                  case ElementType::STAFF_TEXT:
                  case ElementType::SYSTEM_TEXT:
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
                  case ElementType::HARMONY:
                  case ElementType::BAGPIPE_EMBELLISHMENT:
                  case ElementType::AMBITUS:
                  case ElementType::TREMOLOBAR:
                  case ElementType::FIGURED_BASS:
                  case ElementType::LYRICS:
                  case ElementType::FRET_DIAGRAM:
                  case ElementType::STAFFTYPE_CHANGE: {
                        EditData dropData(this);
                        dropData.pos = pos;
                        dropData.element = editData.element;
                        dropData.modifiers = event->keyboardModifiers();

                        if (getDropTarget(dropData))
                              event->accept();
                        else
                              event->ignore();
                        }
                        break;
                  default:
                        event->ignore();
                        break;
                  }

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
                  if (el && (el->isNote() || el->isRest()))
                        setDropTarget(el);
                  else
                        setDropTarget(0);
                  event->accept();
                  }
//            _score->update();
            return;
            }
      QByteArray dta;
      ElementType etype;
      if (md->hasFormat(mimeSymbolListFormat)) {
            etype = ElementType::ELEMENT_LIST;
            dta = md->data(mimeSymbolListFormat);
            }
      else if (md->hasFormat(mimeStaffListFormat)) {
            etype = ElementType::STAFF_LIST;
            dta = md->data(mimeStaffListFormat);
            }
      else {
//            _score->update();
            return;
            }
      Element* el = elementAt(pos);
      if (el == 0 || el->type() != ElementType::MEASURE) {
//            _score->update();
            return;
            }
      else if (etype == ElementType::ELEMENT_LIST) {
            qDebug("accept drop element list");
            }
      else if (etype == ElementType::STAFF_LIST || etype == ElementType::MEASURE_LIST) {
//TODO            el->acceptDrop(this, pos, etype, e);
            }
//      _score->update();
      }

//---------------------------------------------------------
//   dropEvent
//---------------------------------------------------------

void ScoreView::dropEvent(QDropEvent* event)
      {
      switch (state) {
            case ViewState::PLAY:
                  event->ignore();
                  return;
            case ViewState::EDIT:
                  changeState(ViewState::NORMAL);
                  break;

            // TODO: check/handle more states

            case ViewState::NORMAL:
            default:
                  break;
            }
      QPointF pos(imatrix.map(QPointF(event->pos())));

      EditData dropData(this);
      dropData.pos        = pos;
      dropData.element    = editData.element;
      dropData.modifiers  = event->keyboardModifiers();

      if (editData.element) {
            bool applyUserOffset = false;
            editData.element->styleChanged();
            _score->startCmd();
            Q_ASSERT(editData.element->score() == score());
            _score->addRefresh(editData.element->canvasBoundingRect());
            switch (editData.element->type()) {
                  case ElementType::VOLTA:
                  case ElementType::OTTAVA:
                  case ElementType::TRILL:
                  case ElementType::PEDAL:
                  case ElementType::LET_RING:
                  case ElementType::VIBRATO:
                  case ElementType::PALM_MUTE:
                  case ElementType::HAIRPIN:
                  case ElementType::TEXTLINE:
                        {
                        Spanner* spanner = static_cast<Spanner*>(editData.element);
                        score()->cmdAddSpanner(spanner, pos);
                        score()->setUpdateAll();
                        event->acceptProposedAction();
                        }
                        break;
                  case ElementType::SYMBOL:
                  case ElementType::IMAGE:
                        applyUserOffset = true;
                        // fall-thru
                  case ElementType::DYNAMIC:
                  case ElementType::FRET_DIAGRAM:
                  case ElementType::HARMONY:
                        {
                        Element* el = elementAt(pos);
                        if (el == 0 || el->type() == ElementType::STAFF_LINES) {
                              int staffIdx;
                              Segment* seg;
                              QPointF offset;
                              el = _score->pos2measure(pos, &staffIdx, 0, &seg, &offset);
                              if (el && el->isMeasure()) {
                                    editData.element->setTrack(staffIdx * VOICES);
                                    editData.element->setParent(seg);
                                    if (applyUserOffset)
                                          editData.element->setUserOff(offset);
                                    score()->undoAddElement(editData.element);
                                    }
                              else {
                                    qDebug("cannot drop here");
                                    delete editData.element;
                                    }
                              }
                        else {
                              _score->addRefresh(el->canvasBoundingRect());
                              _score->addRefresh(editData.element->canvasBoundingRect());

                              if (!el->acceptDrop(dropData)) {
                                    qDebug("drop %s onto %s not accepted", editData.element->name(), el->name());
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
                  case ElementType::FERMATA:
                  case ElementType::CHORDLINE:
                  case ElementType::BEND:
                  case ElementType::ACCIDENTAL:
                  case ElementType::TEXT:
                  case ElementType::FINGERING:
                  case ElementType::TEMPO_TEXT:
                  case ElementType::STAFF_TEXT:
                  case ElementType::SYSTEM_TEXT:
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
                  case ElementType::BAGPIPE_EMBELLISHMENT:
                  case ElementType::AMBITUS:
                  case ElementType::TREMOLOBAR:
                  case ElementType::FIGURED_BASS:
                  case ElementType::LYRICS:
                  case ElementType::STAFFTYPE_CHANGE: {
                        Element* el = getDropTarget(dropData);
                        if (!el) {
                              if (!dropCanvas(editData.element)) {
                                    qDebug("cannot drop %s(%p) to canvas", editData.element->name(), editData.element);
                                    delete editData.element;
                                    }
                              break;
                              }
                        _score->addRefresh(el->canvasBoundingRect());

                        // HACK ALERT!
                        if (el->isMeasure() && editData.element->isLayoutBreak()) {
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
                        delete editData.element;
                        break;
                  }
            // If the state was changed to ViewState::EDIT,
            // (as a result of ScoreView::cmdAddSlur(), for example)
            // then do not set editData.element to 0.
            if (state != ViewState::EDIT)
                  editData.element = 0;
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
            else if (u.scheme() == "http" || u.scheme() == "https") {
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

      editData.element = 0;
      const QMimeData* md = event->mimeData();
      QByteArray dta;
      ElementType etype;
      if (md->hasFormat(mimeSymbolListFormat)) {
            etype = ElementType::ELEMENT_LIST;
            dta = md->data(mimeSymbolListFormat);
            }
      else if (md->hasFormat(mimeStaffListFormat)) {
            etype = ElementType::STAFF_LIST;
            dta = md->data(mimeStaffListFormat);
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
            XmlReader xml(dta);
            System* s = measure->system();
            int idx   = s->y2staff(pos.y());
            if (idx != -1) {
                  Segment* seg = measure->first();
                  // assume there is always a ChordRest segment
                  while (!seg->isChordRestType())
                        seg = seg->next();
                  score()->pasteStaff(xml, seg, idx);
                  }
            event->acceptProposedAction();
            _score->endCmd();
            }
      setDropTarget(0); // this also resets dropRectangle and dropAnchor
      }

//---------------------------------------------------------
//   dragLeaveEvent
//---------------------------------------------------------

void ScoreView::dragLeaveEvent(QDragLeaveEvent*)
      {
      if (editData.element) {
            _score->setUpdateAll();
            delete editData.element;
            editData.element = 0;
            _score->update();
            }
      setDropTarget(0);
      }

//---------------------------------------------------------
//   dropCanvas
//---------------------------------------------------------

bool ScoreView::dropCanvas(Element* e)
      {
      if (e->isIcon()) {
            switch (toIcon(e)->iconType()) {
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

