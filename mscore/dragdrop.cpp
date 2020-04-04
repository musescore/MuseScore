//=============================================================================
//  MuseScore
//  Music Composition & Notation
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
#include "tourhandler.h"

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
      if (!m_dropAnchorLines.isEmpty())
            m_dropAnchorLines.clear();

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
      else if (!m_dropAnchorLines.isEmpty()) {
            QRectF rf;
            rf.setTopLeft(m_dropAnchorLines.first().p1());
            rf.setBottomRight(m_dropAnchorLines.first().p2());
            _score->addRefresh(rf.normalized());
            m_dropAnchorLines.clear();
            }

      update();
      }

//---------------------------------------------------------
//   setDropAnchorList
//---------------------------------------------------------
void ScoreView::setDropAnchorLines(const QVector<QLineF>& anchorList)
      {
      if (m_dropAnchorLines != anchorList)
            m_dropAnchorLines = anchorList;

      if (dropRectangle.isValid())
            dropRectangle = QRectF();

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
            setDropAnchorLines({ QLineF(pos, anchor) });
            editData.dropElement->score()->addRefresh(editData.dropElement->canvasBoundingRect());
            editData.dropElement->setTrack(track);
            editData.dropElement->score()->addRefresh(editData.dropElement->canvasBoundingRect());
            return true;
            }
      editData.dropElement->score()->addRefresh(editData.dropElement->canvasBoundingRect());
      setDropTarget(0);
      return false;
      }

//---------------------------------------------------------
//   dragMeasureAnchorElement
//---------------------------------------------------------

bool ScoreView::dragMeasureAnchorElement(const QPointF& pos)
      {
      int staffIdx;
      Segment* seg;
      MeasureBase* mb = _score->pos2measure(pos, &staffIdx, 0, &seg, 0);
      if (!(editData.modifiers & Qt::ControlModifier))
            staffIdx = 0;
      int track = staffIdx * VOICES;

      if (mb && mb->isMeasure()) {
            Measure* m = toMeasure(mb);
            System* s  = m->system();
            qreal y    = s->staff(staffIdx)->y() + s->pos().y() + s->page()->pos().y();
            QRectF b(m->canvasBoundingRect());
            if (pos.x() >= (b.x() + b.width() * .5) && m != _score->lastMeasureMM() && m->nextMeasure()->system() == m->system())
                  m = m->nextMeasure();
            QPointF anchor(m->canvasBoundingRect().x(), y);
            setDropAnchorLines({ QLineF(pos, anchor) });
            editData.dropElement->score()->addRefresh(editData.dropElement->canvasBoundingRect());
            editData.dropElement->setTrack(track);
            editData.dropElement->score()->addRefresh(editData.dropElement->canvasBoundingRect());
            return true;
            }
      editData.dropElement->score()->addRefresh(editData.dropElement->canvasBoundingRect());
      setDropTarget(0);
      return false;
      }

//---------------------------------------------------------
//   dragEnterEvent
//---------------------------------------------------------

void ScoreView::dragEnterEvent(QDragEnterEvent* event)
      {
      double _spatium = score()->spatium();
      editData.dropElement = 0;

      const QMimeData* dta = event->mimeData();

      if (dta->hasFormat(mimeSymbolListFormat) || dta->hasFormat(mimeStaffListFormat)) {
            if (event->possibleActions() & Qt::CopyAction)
                  event->setDropAction(Qt::CopyAction);
            if (event->dropAction() == Qt::CopyAction)
                  event->accept();
            return;
            }

      if (dta->hasFormat(mimeSymbolFormat)) {
            if (event->possibleActions() & Qt::CopyAction)
                  event->setDropAction(Qt::CopyAction);
            if (event->dropAction() == Qt::CopyAction)
                  event->accept();

            mscore->notifyElementDraggedToScoreView();

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
                  editData.dropElement = el;
                  editData.dropElement->setParent(0);
                  editData.dropElement->read(e);
                  editData.dropElement->layout();
                  }
            return;
            }

      if (dta->hasUrls()) {
            QList<QUrl>ul = dta->urls();
            QUrl u = ul.front();

            QMimeDatabase db;
            if (!QImageReader::supportedMimeTypes().contains(db.mimeTypeForUrl(u).name().toLatin1())) {
                  event->ignore();
                  return;
                  }

            Image* image = 0;
            if (u.scheme() == "file") {
                  image = new Image(score());
                  QString str(u.toLocalFile());
                  image->load(str);
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

                  image = new Image(score());
                  image->loadFromData(u.path(), ba);
                  delete reply;
                  }
            if (image) {
                  editData.dropElement = image;
                  editData.dropElement->setParent(0);
                  editData.dropElement->layout();
                  event->accept();
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
      for (Element* e : qAsConst(el)) {
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

      if (MScore::debugMode) {
            if (!editData.dropElement)
                  qDebug("no drop element");
            else
                  qDebug("<%s>", editData.dropElement->name());
            }

      if (!editData.dropElement || mscore->state() == STATE_PLAY) {  // no editing during play
            event->ignore();
            return;
            }

      const QMimeData* dta = event->mimeData();
      if (dta->hasFormat(mimeSymbolFormat)
         || dta->hasFormat(mimeSymbolListFormat)
         || dta->hasFormat(mimeStaffListFormat)) {
            if (event->possibleActions() & Qt::CopyAction)
                  event->setDropAction(Qt::CopyAction);
            }

      // convert window to canvas position
      QPointF pos(imatrix.map(QPointF(event->pos())));
      editData.pos       = pos;
      editData.modifiers = event->keyboardModifiers();

      switch (editData.dropElement->type()) {
            case ElementType::VOLTA:
                  event->setAccepted(dragMeasureAnchorElement(pos));
                  break;
            case ElementType::PEDAL:
            case ElementType::LET_RING:
            case ElementType::VIBRATO:
            case ElementType::PALM_MUTE:
            case ElementType::OTTAVA:
            case ElementType::TRILL:
            case ElementType::HAIRPIN:
            case ElementType::TEXTLINE:
                  event->setAccepted(dragTimeAnchorElement(pos));
                  break;
            case ElementType::IMAGE:
            case ElementType::SYMBOL:
            case ElementType::FSYMBOL:
            case ElementType::DYNAMIC:
            case ElementType::KEYSIG:
            case ElementType::CLEF:
            case ElementType::TIMESIG:
            case ElementType::BAR_LINE:
            case ElementType::ARPEGGIO:
            case ElementType::BREATH:
            case ElementType::GLISSANDO:
            case ElementType::MEASURE_NUMBER:
            case ElementType::MMREST_RANGE:
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
            case ElementType::STAFFTYPE_CHANGE:
                  event->setAccepted(getDropTarget(editData));
                  break;
            default:
                  if (MScore::debugMode)
                        qDebug("no target");
                  event->ignore();
                  break;
            }
      }

//---------------------------------------------------------
//   dropEvent
//---------------------------------------------------------

void ScoreView::dropEvent(QDropEvent* event)
      {
      if (state == ViewState::PLAY) {
            event->ignore();
            return;
            }
      QPointF pos(imatrix.map(QPointF(event->pos())));

      editData.pos       = pos;
      editData.modifiers = event->keyboardModifiers();

      if (editData.dropElement) {
            bool firstStaffOnly = false;
            bool applyUserOffset = false;
            bool triggerSpannerDropApplyTour = editData.dropElement->isSpanner();
            editData.dropElement->styleChanged();
            _score->startCmd();
            Q_ASSERT(editData.dropElement->score() == score());
            _score->addRefresh(editData.dropElement->canvasBoundingRect());
            switch (editData.dropElement->type()) {
                  case ElementType::VOLTA:
                        // voltas drop to first staff by default, or closest staff if Control is held
                        firstStaffOnly = !(editData.modifiers & Qt::ControlModifier);
                        // fall-thru
                  case ElementType::OTTAVA:
                  case ElementType::TRILL:
                  case ElementType::PEDAL:
                  case ElementType::LET_RING:
                  case ElementType::VIBRATO:
                  case ElementType::PALM_MUTE:
                  case ElementType::HAIRPIN:
                  case ElementType::TEXTLINE:
                        {
                        Spanner* spanner = static_cast<Spanner*>(editData.dropElement);
                        score()->cmdAddSpanner(spanner, pos, firstStaffOnly);
                        score()->setUpdateAll();
                        event->acceptProposedAction();
                        }
                        break;
                  case ElementType::SYMBOL:
                  case ElementType::FSYMBOL:
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
                                    editData.dropElement->setTrack(staffIdx * VOICES);
                                    editData.dropElement->setParent(seg);
                                    if (applyUserOffset)
                                          editData.dropElement->setOffset(offset);
                                    score()->undoAddElement(editData.dropElement);
                                    }
                              else {
                                    qDebug("cannot drop here");
                                    delete editData.dropElement;
                                    }
                              }
                        else {
                              _score->addRefresh(el->canvasBoundingRect());
                              _score->addRefresh(editData.dropElement->canvasBoundingRect());

                              if (!el->acceptDrop(editData)) {
                                    qDebug("drop %s onto %s not accepted", editData.dropElement->name(), el->name());
                                    break;
                                    }
                              Element* dropElement = el->drop(editData);
                              _score->addRefresh(el->canvasBoundingRect());
                              if (dropElement) {
                                    _score->select(dropElement, SelectType::SINGLE, 0);
                                    _score->addRefresh(dropElement->canvasBoundingRect());
                                    }
                              }
                        }
                        event->acceptProposedAction();
                        break;
                  case ElementType::HBOX:
                  case ElementType::VBOX:
                  case ElementType::KEYSIG:
                  case ElementType::CLEF:
                  case ElementType::TIMESIG:
                  case ElementType::BAR_LINE:
                  case ElementType::ARPEGGIO:
                  case ElementType::BREATH:
                  case ElementType::GLISSANDO:
                  case ElementType::MEASURE_NUMBER:
                  case ElementType::MMREST_RANGE:
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
                        Element* el = getDropTarget(editData);
                        if (!el) {
                              if (!dropCanvas(editData.dropElement)) {
                                    qDebug("cannot drop %s(%p) to canvas", editData.dropElement->name(), editData.dropElement);
                                    delete editData.dropElement;
                                    }
                              break;
                              }
                        _score->addRefresh(el->canvasBoundingRect());

                        // TODO: HACK ALERT!
                        if (el->isMeasure() && editData.dropElement->isLayoutBreak()) {
                              Measure* m = toMeasure(el);
                              if (m->isMMRest())
                                    el = m->mmRestLast();
                              }

                        Element* dropElement = el->drop(editData);
                        if (dropElement && dropElement->isInstrumentChange()) {
                              mscore->currentScoreView()->selectInstrument(toInstrumentChange(dropElement));
                              }
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
                        delete editData.dropElement;
                        break;
                  }
            editData.dropElement = 0;
            setDropTarget(0); // this also resets dropRectangle and dropAnchor
            score()->endCmd();
            // update input cursor position (must be done after layout)
            if (noteEntryMode())
                  moveCursor();
            if (triggerSpannerDropApplyTour)
                  TourHandler::startTour("spanner-drop-apply");
            return;
            }

      editData.dropElement = 0;
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
            for (const QString& s : qAsConst(sl))
                  qDebug("  %s", qPrintable(s));
            _score->update();
            return;
            }

qDebug("drop <%s>", dta.data());

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
      if (editData.dropElement) {
            _score->setUpdateAll();
            delete editData.dropElement;
            editData.dropElement = 0;
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

} // namespace Ms
