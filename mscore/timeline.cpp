//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "timeline.h"
#include "navigator.h"
#include "musescore.h"
#include "libmscore/score.h"
#include "libmscore/page.h"
#include "preferences.h"
#include "libmscore/mscore.h"
#include "libmscore/system.h"
#include "libmscore/measurebase.h"
#include "libmscore/measure.h"
#include "libmscore/chord.h"
#include "libmscore/staff.h"
#include "libmscore/rest.h"
#include "libmscore/part.h"
#include "libmscore/tempo.h"
#include "libmscore/keysig.h"
#include "libmscore/timesig.h"
#include "libmscore/key.h"
#include "libmscore/tempotext.h"
#include "libmscore/text.h"
#include "libmscore/rehearsalmark.h"

namespace Ms {

//---------------------------------------------------------
//   showTimeline
//---------------------------------------------------------

void MuseScore::showTimeline(bool visible)
      {
      QSplitter* s = static_cast<QSplitter*>(_timeline->widget());
      Timeline* t = static_cast<Timeline*>(s->widget(1));

      QAction* a = getAction("toggle-timeline");
      if (t == 0 && visible) {
            t = new Timeline(_timeline);
            t->setScore(cv->score());
            t->setScoreView(cv);
            }
      connect(_timeline, SIGNAL(closed(bool)), a, SLOT(setChecked(bool)));
      _timeline->setVisible(visible);

      getAction("toggle-timeline")->setChecked(visible);
      }

//---------------------------------------------------------
//   TDockWidget
//---------------------------------------------------------

TDockWidget::TDockWidget(QWidget* w)
   : QDockWidget(w)
      {
      setFocusPolicy(Qt::NoFocus);
      _grid = new QSplitter();
      setWidget(_grid);
      _grid->setHandleWidth(0);
      _grid->setMinimumWidth(0);
      setAllowedAreas(Qt::DockWidgetAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea));
      setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

      setMinimumHeight(50);

      setWindowTitle(tr("Timeline"));
      }

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void TDockWidget::closeEvent(QCloseEvent *event)
      {
      emit closed(false);
      QWidget::closeEvent(event);
      }

//---------------------------------------------------------
//   TRowLabels
//---------------------------------------------------------

TRowLabels::TRowLabels(TDockWidget *sa, Timeline *t, QGraphicsView *w)
  : QGraphicsView(w)
      {
      setFocusPolicy(Qt::NoFocus);
      scrollArea = sa;
      parent = t;
      setScene(new QGraphicsScene);
      scene()->setBackgroundBrush(Qt::lightGray);
      setSceneRect(0, 0, 50, t->height());

      setMinimumWidth(0);

      QSplitter* s = scrollArea->grid();
      QList<int> sizes;
      sizes << 50 << 10000;
      s->setSizes(sizes);

      setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      setContentsMargins(0, 0, 0, 0);
      setAlignment(Qt::Alignment((Qt::AlignLeft | Qt::AlignTop)));

      connect(verticalScrollBar(),
              SIGNAL(valueChanged(int)),
              t->verticalScrollBar(),
              SLOT(setValue(int)));

      connect(verticalScrollBar(),
              SIGNAL(valueChanged(int)),
              this,
              SLOT(restrict_scroll(int)));
      }

//---------------------------------------------------------
//   restrict_scroll
//---------------------------------------------------------

void TRowLabels::restrict_scroll(int value)
      {
      if (value > parent->verticalScrollBar()->maximum())
            verticalScrollBar()->setValue(parent->verticalScrollBar()->maximum());
      for (std::vector<std::pair<QGraphicsItem*, int>>::iterator it = meta_labels.begin();
           it != meta_labels.end(); ++it) {
            std::pair<QGraphicsItem*, int> p = *it;

            QGraphicsItem* gi = p.first;
            QGraphicsRectItem* ri = dynamic_cast<QGraphicsRectItem*>(gi);
            QGraphicsLineItem* li = dynamic_cast<QGraphicsLineItem*>(gi);
            int r = p.second * 20;
            int v = verticalScrollBar()->value();
            if (ri) {
                  QRectF rf = ri->rect();
                  rf.setY(qreal(v + r));
                  rf.setHeight(20);
                  ri->setRect(rf);
                  }
            else if (li) {
                  QLineF lf = li->line();
                  lf.setLine(lf.x1(), r + v + 1, lf.x2(), r + v + 1);
                  li->setLine(lf);
                  }
            else {
                  gi->setY(qreal(v + r));
                  }
            }
      viewport()->update();

      }

//---------------------------------------------------------
//   updateLabels
//---------------------------------------------------------

void TRowLabels::updateLabels(std::vector<QString> labels, int height)
      {

      scene()->clear();
      meta_labels.clear();
      if (labels.empty())
            return;

      int max_width = -1;
      for (unsigned int r = 0; r < labels.size(); r++) {
            //Draw instrument name rectangle
            int ypos = (r < parent->nmetas())? r*20 + verticalScrollBar()->value() : r*20 + 3;
            QGraphicsRectItem* ri = new QGraphicsRectItem(0,
                                                          ypos,
                                                          width(),
                                                          height);
            QGraphicsTextItem* t = new QGraphicsTextItem(labels[r]);
            max_width = max(max_width, int(t->boundingRect().width()));
            QFontMetrics f(QApplication::font());
            QString part_name = f.elidedText(labels[r],
                                             Qt::ElideRight,
                                             width());
            t->setPlainText(part_name);
            t->setX(0);
            t->setY(ypos);
            ri->setPen(QPen(QColor(150, 150, 150)));
            ri->setBrush(QBrush(QColor(211, 211, 211)));
            t->setDefaultTextColor(QColor(0, 0, 0));
            t->setZValue(-1);
            ri->setZValue(-1);

            scene()->addItem(ri);
            scene()->addItem(t);
            if (r < parent->nmetas()) {
                  std::pair<QGraphicsItem*, int> p1(ri, r);
                  std::pair<QGraphicsItem*, int> p2(t, r);
                  meta_labels.push_back(p1);
                  meta_labels.push_back(p2);
                  ri->setZValue(1);
                  t->setZValue(2);
                  }
            }
      QGraphicsLineItem* li = new QGraphicsLineItem(0,
                                                    20 * parent->nmetas() + verticalScrollBar()->value() + 1,
                                                    max_width,
                                                    20 * parent->nmetas() + verticalScrollBar()->value() + 1);
      li->setPen(QPen(QColor(150, 150, 150), 4));
      li->setZValue(0);
      scene()->addItem(li);

      std::pair<QGraphicsItem*, int> li_p(li, parent->nmetas());
      meta_labels.push_back(li_p);

      setMaximumWidth(max_width);
      setSceneRect(0, 0, max_width, parent->getHeight());
      }

//---------------------------------------------------------
//   resizeEvent
//---------------------------------------------------------

void TRowLabels::resizeEvent(QResizeEvent* /*event*/)
      {
      std::vector<QString> labels = parent->getLabels();
      updateLabels(labels, 20);
      }

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void TRowLabels::mousePressEvent(QMouseEvent *event)
      {
      dragging = true;
      this->setCursor(Qt::SizeAllCursor);
      QPointF scenePt = mapToScene(event->pos());
      old_loc = QPoint(int(scenePt.x()), int(scenePt.y()));
      }

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void TRowLabels::mouseMoveEvent(QMouseEvent *event)
      {
      if (dragging) {
            this->setCursor(Qt::SizeAllCursor);

            QPointF new_loc = mapToScene(event->pos());
            int yo = int(old_loc.y()) - int(new_loc.y());
            verticalScrollBar()->setValue(verticalScrollBar()->value() + yo);
            }
      }

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void TRowLabels::mouseReleaseEvent(QMouseEvent*)
      {
      this->setCursor(Qt::ArrowCursor);
      dragging = false;
      }

//---------------------------------------------------------
//   Timeline
//---------------------------------------------------------

Timeline::Timeline(TDockWidget* sa, QWidget* parent)
  : QGraphicsView(parent)
      {
      setFocusPolicy(Qt::NoFocus);
      setAlignment(Qt::Alignment((Qt::AlignLeft | Qt::AlignTop)));
      setAttribute(Qt::WA_NoBackground);
      scrollArea     = sa;
      QSplitter* s = static_cast<QSplitter*>(scrollArea->widget());

      row_names = new TRowLabels(sa, this);
      s->addWidget(row_names);
      s->addWidget(this);
      s->setChildrenCollapsible(false);
      s->setStretchFactor(0, 0);
      s->setStretchFactor(1, 0);

      setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

      setScene(new QGraphicsScene);
      setSceneRect(0, 0, 100, 100);
      scene()->setBackgroundBrush(Qt::lightGray);

      connect(verticalScrollBar(),
              SIGNAL(valueChanged(int)),
              row_names->verticalScrollBar(),
              SLOT(setValue(int)));

      connect(verticalScrollBar(),
              SIGNAL(valueChanged(int)),
              this,
              SLOT(handle_scroll(int)));

      }

//---------------------------------------------------------
//   drawGrid
//---------------------------------------------------------

void Timeline::drawGrid(int rows, int cols)
      {
      scene()->clear();
      meta_rows.clear();

      if (rows == 0 || cols == 0) return;

      int length = cols*20;
      int height = 20;

      Measure* curr = _score->firstMeasure();
      for (int c = 0; c < cols; c++) {
            for (int r = 0; r < rows; r++) {
                  QGraphicsRectItem* rect = new QGraphicsRectItem(c * 20, r * 20 + height * nmetas() + 3, 20, 20);

                  rect->setData(0, QVariant::fromValue<int>(correctStave(r)));
                  rect->setData(1, QVariant::fromValue<ElementType>(ElementType::INVALID));
                  rect->setData(2, QVariant::fromValue<void*>(curr));
                  rect->setData(3, QVariant::fromValue<bool>(false));

                  rect->setPen(QPen(QColor(204, 204, 204)));
                  rect->setBrush(QBrush(colorBox(rect)));
                  rect->setFlags(rect->flags());
                  rect->setZValue(-3);
                  scene()->addItem(rect);
                  }

            curr = curr->nextMeasure();
            }
      setSceneRect(0, 0, getWidth(), getHeight());

      //Draw horizontal line for separation
      QGraphicsLineItem* separator = new QGraphicsLineItem(0,
                                                           height * nmetas() + verticalScrollBar()->value() + 1,
                                                           getWidth() - 1,
                                                           height * nmetas() + verticalScrollBar()->value() + 1);
      separator->setPen(QPen(QColor(150, 150, 150), 4));
      separator->setZValue(-2);
      scene()->addItem(separator);
      std::pair<QGraphicsItem*, int> sep_p(separator, nmetas());
      meta_rows.push_back(sep_p);

      //Draw tempo
      QGraphicsRectItem* tempo_box = new QGraphicsRectItem(0,
                                                           verticalScrollBar()->value(),
                                                           length,
                                                           height);
      tempo_box->setBrush(QBrush(QColor(211,211,211)));
      tempo_box->setData(0, QVariant::fromValue<int>(-1));
      tempo_box->setPen(QPen(QColor(150, 150, 150)));
      scene()->addItem(tempo_box);
      std::pair<QGraphicsItem*, int> tempo_p(tempo_box, 0);
      meta_rows.push_back(tempo_p);


      for (Segment* s = _score->firstMeasure()->first(); s; s = s->next1()) {
            const std::vector<Element*> annotations = s->annotations();
            for (Element* e : annotations) {
                  if (e->type() == ElementType::TEMPO_TEXT) {
                        Text* t = toText(e);
                        qreal pos = 0;

                        Measure* curr = _score->firstMeasure();
                        Measure* target = s->measure();

                        for (int m = 0; curr; m++, curr = curr->nextMeasure()) {
                              if (curr == target) {
                                    pos = m * 20;
                                    break;
                                    }
                              }

                        QGraphicsTextItem* text = new QGraphicsTextItem(t->plainText());
                        int text_width = text->boundingRect().width();
                        if (text_width + pos > cols * 20) {
                              text_width = cols * 20 - pos;
                              }
                        QFontMetrics f(QApplication::font());
                        QString part_name = f.elidedText(t->plainText(),
                                                         Qt::ElideRight,
                                                         text_width);
                        text->setPlainText(part_name);
                        text->setX(pos);
                        text->setY(verticalScrollBar()->value());
                        QGraphicsRectItem* tempo = new QGraphicsRectItem(pos,
                                                                         verticalScrollBar()->value(),
                                                                         text->boundingRect().width(),
                                                                         height);
                        tempo->setData(0, QVariant::fromValue<int>(-1));
                        tempo->setData(1, QVariant::fromValue<ElementType>(ElementType::TEMPO_TEXT));
                        tempo->setData(2, QVariant::fromValue<void*>(s->measure()));
                        tempo->setData(3, QVariant::fromValue<bool>(true));
                        tempo->setData(4, QVariant::fromValue<void*>(e));

                        text->setData(0, QVariant::fromValue<int>(-1));
                        text->setData(1, QVariant::fromValue<ElementType>(ElementType::TEMPO_TEXT));
                        text->setData(2, QVariant::fromValue<void*>(s->measure()));
                        text->setData(3, QVariant::fromValue<bool>(true));
                        text->setData(4, QVariant::fromValue<void*>(e));

                        tempo->setPen(QPen(Qt::black));
                        tempo->setBrush(QBrush(QColor(150,150,150)));
                        scene()->addItem(tempo);
                        scene()->addItem(text);

                        std::pair<QGraphicsItem*, int> tempo1(tempo, 0);
                        std::pair<QGraphicsItem*, int> tempo2(text, 0);
                        meta_rows.push_back(tempo1);
                        meta_rows.push_back(tempo2);

                        }
                  }
            }

      //Draw time signature
      QGraphicsRectItem* time_box = new QGraphicsRectItem(0,
                                                          height + verticalScrollBar()->value(),
                                                          length,
                                                          height);
      time_box->setBrush(QBrush(QColor(211,211,211)));
      time_box->setPen(QPen(QColor(150, 150, 150)));
      time_box->setData(0, QVariant::fromValue<int>(-1));

      std::pair<QGraphicsItem*, int> time_p(time_box, 1);
      meta_rows.push_back(time_p);

      scene()->addItem(time_box);

      Segment* s = _score->firstSegment(SegmentType::TimeSig);

      for (; s; s = s->next1(SegmentType::TimeSig)) {
            qreal pos = 0;

            Measure* curr = _score->firstMeasure();
            Measure* target = s->measure();

            for (int m = 0; curr; m++, curr = curr->nextMeasure()) {
                  if (curr == target) {
                        pos = m * 20;
                        break;
                        }
                  }

            TimeSig* ts = toTimeSig(s->element(0));
            if (!ts) continue;

            const int size = _score->staves().size();
            bool same = true;
            for (int track = 0; track < size; track++) {
                  const TimeSig* curr = toTimeSig(s->element(track * 4));
                  if (!curr) {
                        same = false;
                        break;
                        }
                  if (*curr == *ts) {
                        continue;
                        }
                  same = false;
                  break;
                  }
            if (!same) continue;

            QGraphicsTextItem* text = new QGraphicsTextItem(QString::number(ts->numerator()) +
                                                            tr("/") +
                                                            QString::number(ts->denominator()));
            int text_width = text->boundingRect().width();
            if (text_width + pos > cols * 20) {
                  text_width = cols * 20 - pos;
                  }
            QFontMetrics f(QApplication::font());
            QString part_name = f.elidedText(text->toPlainText(),
                                             Qt::ElideRight,
                                             text_width);
            text->setPlainText(part_name);
            text->setX(pos);
            text->setY(height + verticalScrollBar()->value());
            QGraphicsRectItem* time = new QGraphicsRectItem(pos,
                                                            height + verticalScrollBar()->value(),
                                                            text->boundingRect().width(),
                                                            height);
            time->setData(0, QVariant::fromValue<int>(-1));
            time->setData(1, QVariant::fromValue<ElementType>(ElementType::TIMESIG));
            time->setData(2, QVariant::fromValue<void*>(s->measure()));
            time->setData(3, QVariant::fromValue<bool>(true));

            text->setData(0, QVariant::fromValue<int>(-1));
            text->setData(1, QVariant::fromValue<ElementType>(ElementType::TIMESIG));
            text->setData(2, QVariant::fromValue<void*>(s->measure()));
            text->setData(3, QVariant::fromValue<bool>(true));

            time->setPen(QPen(Qt::black));
            time->setBrush(QBrush(QColor(150,150,150)));
            scene()->addItem(time);
            scene()->addItem(text);

            std::pair<QGraphicsItem*, int> time1(time, 1);
            std::pair<QGraphicsItem*, int> time2(text, 1);
            meta_rows.push_back(time1);
            meta_rows.push_back(time2);
            }

      //Draw key signature
      QGraphicsRectItem* key_box = new QGraphicsRectItem(0,
                                                         height * 2 + verticalScrollBar()->value(),
                                                         length,
                                                         height);
      key_box->setBrush(QBrush(QColor(211,211,211)));
      key_box->setPen(QPen(QColor(150, 150, 150)));
      key_box->setData(0, QVariant::fromValue<int>(-1));
      scene()->addItem(key_box);

      std::pair<QGraphicsItem*, int> key_p(key_box, 2);
      meta_rows.push_back(key_p);

      s = _score->firstSegment(SegmentType::KeySig);

      Measure* m = _score->firstMeasure();
      bool has_keysig = false;
      for (Segment* seg = m->first(); seg; seg = seg->next()) {
            if (seg->segmentType() == SegmentType::KeySig) {
                  has_keysig = true;
                  break;
                  }
            }
      if (!has_keysig) {
            Key tmp = Key::C;
            Interval tmp_interval = _score->staff(0)->part()->instrument()->transpose();
            QList<Staff*> staves = _score->staves();
            bool different = false;
            for (Staff* s : staves) {
                  if (tmp_interval != s->part()->instrument()->transpose()) {
                        different = true;
                        break;
                        }
                  }
            if (!different || _score->styleB(StyleIdx::concertPitch)) {
                  tmp = transposeKey(tmp, tmp_interval);

                  int keyInt = static_cast<int>(tmp);

                  QString keys;
                  if (keyInt == 0)
                        keys = "\u266E";
                  else if (keyInt < 0)
                        keys = QString::number(abs(keyInt)) + "\u266D";
                  else
                        keys = QString::number(abs(keyInt)) + "\u266F";

                  QGraphicsTextItem* text = new QGraphicsTextItem(keys);
                  int text_width = text->boundingRect().width();
                  if (text_width> cols * 20) {
                        text_width = cols * 20;
                        }
                  QFontMetrics f(QApplication::font());
                  QString part_name = f.elidedText(text->toPlainText(),
                                                   Qt::ElideRight,
                                                   text_width);
                  text->setPlainText(part_name);
                  text->setX(0);
                  text->setY(height * 2 + verticalScrollBar()->value());
                  QGraphicsRectItem* key = new QGraphicsRectItem(0,
                                                                 height * 2 + verticalScrollBar()->value(),
                                                                 max(text_width, 20),
                                                                 height);
                  key->setData(0, QVariant::fromValue<int>(-1));
                  key->setData(1, QVariant::fromValue<ElementType>(ElementType::KEYSIG));
                  key->setData(2, QVariant::fromValue<void*>(_score->firstMeasure()));
                  key->setData(3, QVariant::fromValue<bool>(true));

                  text->setData(0, QVariant::fromValue<int>(-1));
                  text->setData(1, QVariant::fromValue<ElementType>(ElementType::KEYSIG));
                  text->setData(2, QVariant::fromValue<void*>(_score->firstMeasure()));
                  text->setData(3, QVariant::fromValue<bool>(true));

                  key->setPen(QPen(Qt::black));
                  key->setBrush(QBrush(QColor(150,150,150)));
                  scene()->addItem(key);
                  scene()->addItem(text);

                  std::pair<QGraphicsItem*, int> key1(key, 2);
                  std::pair<QGraphicsItem*, int> key2(text, 2);
                  meta_rows.push_back(key1);
                  meta_rows.push_back(key2);
                  }
            }

      for (s = _score->firstSegment(SegmentType::KeySig); s; s = s->next1(SegmentType::KeySig)) {
            const KeySig* k = toKeySig(s->element(0));
            int staff_track = 0;
            while (!k && staff_track < rows) {
                  staff_track++;
                  k = toKeySig(s->element(staff_track * VOICES));
                  }
            if (!k) continue;
            if (k->generated()) continue;

            KeySig* nk = new KeySig(*k);
            QList<Staff*> staves = _score->staves();

            if (!nk->isAtonal() && !nk->isCustom() && !_score->styleB(StyleIdx::concertPitch)) {
                  const Key key = k->key();
                  const Interval firstInterval = staves.at(staff_track)->part()->instrument()->transpose();
                  Key nKey = transposeKey(key, firstInterval);
                  nk->setKey(nKey);
                  }

            int track = 0;
            bool same = true;
            for (Staff* st : staves) {
                  if (!st->isPitchedStaff(s->tick())) {
                        track += VOICES;
                        continue;
                        }
                  const KeySig* curr = toKeySig(s->element(track));
                  if (!curr) {
                        const Interval curr_interval = st->part()->instrument()->transpose();
                        Key tmp = Key::C;
                        tmp = transposeKey(tmp, curr_interval);
                        if (nk->key() == tmp) {
                              track += VOICES;
                              continue;
                              }
                        same = false;
                        break;
                        }
                  KeySig* ncurr = new KeySig(*curr);
                  //Transpose to concert pitch
                  if (!ncurr->isAtonal() && !ncurr->isCustom() && !_score->styleB(StyleIdx::concertPitch)) {
                        const Key key  = ncurr->key();
                        const Interval firstInterval = st->part()->instrument()->transpose();
                        Key nKey = transposeKey(key, firstInterval);
                        ncurr->setKey(nKey);
                        }
                  if (*ncurr == *nk) {
                        track += VOICES;
                        continue;
                        }
                  same = false;
                  break;
                  }
            if (!same) continue;

            int keyInt = static_cast<int>(nk->key());

            QString keys;
            if (nk->isAtonal())
                  keys = "X";
            else if (nk->isCustom())
                  keys = tr("?");
            else if (keyInt == 0)
                  keys = "\u266E";
            else if (keyInt < 0)
                  keys = QString::number(abs(keyInt)) + "\u266D";
            else
                  keys = QString::number(abs(keyInt)) + "\u266F";

            qreal pos = 0;

            Measure* curr = _score->firstMeasure();
            Measure* target = s->measure();

            for (int m = 0; curr; m++, curr = curr->nextMeasure()) {
                  if (curr == target) {
                        pos = m * 20;
                        break;
                        }
                  }

            QGraphicsTextItem* text = new QGraphicsTextItem(keys);
            int text_width = text->boundingRect().width();
            if (text_width + pos > cols * 20) {
                  text_width = cols * 20 - pos;
                  }
            QFontMetrics f(QApplication::font());
            QString part_name = f.elidedText(text->toPlainText(),
                                             Qt::ElideRight,
                                             text_width);
            text->setPlainText(part_name);
            text->setX(pos);
            text->setY(height * 2 + verticalScrollBar()->value());
            QGraphicsRectItem* key = new QGraphicsRectItem(pos,
                                                           height * 2 + verticalScrollBar()->value(),
                                                           max(text_width, 20),
                                                           height);
            key->setData(0, QVariant::fromValue<int>(-1));
            key->setData(1, QVariant::fromValue<ElementType>(ElementType::KEYSIG));
            key->setData(2, QVariant::fromValue<void*>(s->measure()));
            key->setData(3, QVariant::fromValue<bool>(true));

            text->setData(0, QVariant::fromValue<int>(-1));
            text->setData(1, QVariant::fromValue<ElementType>(ElementType::KEYSIG));
            text->setData(2, QVariant::fromValue<void*>(s->measure()));
            text->setData(3, QVariant::fromValue<bool>(true));

            key->setPen(QPen(Qt::black));
            key->setBrush(QBrush(QColor(150,150,150)));
            scene()->addItem(key);
            scene()->addItem(text);

            std::pair<QGraphicsItem*, int> key1(key, 2);
            std::pair<QGraphicsItem*, int> key2(text, 2);
            meta_rows.push_back(key1);
            meta_rows.push_back(key2);
            }

      //Draw rehearsal marks
      QGraphicsRectItem* rehearsal_box = new QGraphicsRectItem(0,
                                                               height * 3 + verticalScrollBar()->value(),
                                                               length,
                                                               height);
      rehearsal_box->setBrush(QBrush(QColor(211,211,211)));
      rehearsal_box->setPen(QPen(QColor(150, 150, 150)));
      rehearsal_box->setData(0, QVariant::fromValue<int>(-1));
      scene()->addItem(rehearsal_box);

      std::pair<QGraphicsItem*, int> rehearsal_p(rehearsal_box, 3);
      meta_rows.push_back(rehearsal_p);

      for (Segment* seg = _score->firstSegment(SegmentType::ChordRest); seg; seg = seg->next1(SegmentType::ChordRest)) {
            for (Element* e : seg->annotations()) {
                  if (e->type() == ElementType::REHEARSAL_MARK) {
                        int pos = 0;

                        Measure* curr = _score->firstMeasure();
                        Measure* target = seg->measure();
                        for (int m = 0; curr; m++, curr = curr->nextMeasure()) {
                              if (curr == target) {
                                    pos = m * 20;
                                    break;
                                    }
                              }

                        RehearsalMark* rm = toRehearsalMark(e);
                        if (!rm) continue;

                        QGraphicsTextItem* text = new QGraphicsTextItem(rm->plainText());
                        int text_width = text->boundingRect().width();
                        if (text_width + pos > cols * 20) {
                              text_width = cols * 20 - pos;
                              }
                        QFontMetrics f(QApplication::font());
                        QString part_name = f.elidedText(text->toPlainText(),
                                                         Qt::ElideRight,
                                                         text_width);
                        text->setPlainText(part_name);
                        text->setX(pos);
                        text->setY(height * 3 + verticalScrollBar()->value());
                        QGraphicsRectItem* rehearse = new QGraphicsRectItem(pos,
                                                                            height * 3 + verticalScrollBar()->value(),
                                                                            text->boundingRect().width(),
                                                                            height);
                        rehearse->setData(0, QVariant::fromValue<int>(-1));
                        rehearse->setData(1, QVariant::fromValue<ElementType>(ElementType::REHEARSAL_MARK));
                        rehearse->setData(2, QVariant::fromValue<void*>(seg->measure()));
                        rehearse->setData(3, QVariant::fromValue<bool>(true));
                        rehearse->setData(4, QVariant::fromValue<void*>(e));

                        text->setData(0, QVariant::fromValue<int>(-1));
                        text->setData(1, QVariant::fromValue<ElementType>(ElementType::REHEARSAL_MARK));
                        text->setData(2, QVariant::fromValue<void*>(seg->measure()));
                        text->setData(3, QVariant::fromValue<bool>(true));
                        text->setData(4, QVariant::fromValue<void*>(e));

                        rehearse->setPen(QPen(Qt::black));
                        rehearse->setBrush(QBrush(QColor(150,150,150)));
                        scene()->addItem(rehearse);
                        scene()->addItem(text);

                        std::pair<QGraphicsItem*, int> rehearse1(rehearse, 3);
                        std::pair<QGraphicsItem*, int> rehearse2(text, 3);
                        meta_rows.push_back(rehearse1);
                        meta_rows.push_back(rehearse2);
                        }
                  }
            }

      //Draw measure numbers
      QGraphicsRectItem* measure_box = new QGraphicsRectItem(0,
                                                             height * 4 + verticalScrollBar()->value(),
                                                             length,
                                                             height);
      measure_box->setBrush(QBrush(QColor(211,211,211)));
      measure_box->setPen(QPen(QColor(150, 150, 150)));
      measure_box->setData(0, QVariant::fromValue<int>(-1));
      scene()->addItem(measure_box);

      std::pair<QGraphicsItem*, int> measure_p(measure_box, 4);
      meta_rows.push_back(measure_p);

      for (int c = 0; c < cols; c++) {
            if ((c + 1) % 5 == 0 || c == 0) {
                  //Make sure to center measure numbers and decrease text size

                  QGraphicsTextItem* num = new QGraphicsTextItem(QString::number(c + 1));
                  num->setDefaultTextColor(QColor(0, 0, 0));
                  num->setX(c * 20);
                  num->setY(height * 4 + verticalScrollBar()->value());

                  QFont f = num->font();
                  f.setPointSizeF(7.0);
                  num->setFont(f);

                  //Center text
                  qreal remainder_w =  20 - num->boundingRect().width();
                  qreal remainder_h =  height - num->boundingRect().height();
                  num->setX(num->x() + remainder_w / 2);
                  num->setY(num->y() + remainder_h / 2);

                  scene()->addItem(num);

                  std::pair<QGraphicsItem*, int> measure1(num, 4);
                  meta_rows.push_back(measure1);
                  }
            }

      row_names->updateLabels(getLabels(), height);
      }

//---------------------------------------------------------
//   getWidth
//---------------------------------------------------------

int Timeline::getWidth()
      {
      if (_score)
            return int(_score->nmeasures() * 20);
      else
            return 0;
      }

//---------------------------------------------------------
//   getHeight
//---------------------------------------------------------

int Timeline::getHeight()
      {
      if (_score)
            return int((nstaves() + nmetas()) * 20 + 3);
      else
            return 0;
      }

//---------------------------------------------------------
//   correctStave
//---------------------------------------------------------

int Timeline::correctStave(int stave)
      {
      //Find correct stave (skipping hidden staves)
      QList<Staff*> list = _score->staves();
      int c = 0;
      while (stave >= c) {
            if (!list.at(c)->show())
                  stave++;
            c++;
            }
      return stave;
      }


//---------------------------------------------------------
//   correctPart
//---------------------------------------------------------

int Timeline::correctPart(int stave)
      {
      QList<Staff*> list = _score->staves();
      int c = 0;
      while (stave >= c) {
            if (!list.at(c)->show())
                  stave++;
            c++;
            }
      return _score->parts().indexOf(list.at(stave)->part());
      }


//---------------------------------------------------------
//   changeSelection
//---------------------------------------------------------

void Timeline::changeSelection(SelState)
      {
      scene()->blockSignals(true);
      scene()->clearSelection();

      QRectF selection = path.boundingRect();
      if (selection == QRectF()) return;

      int nmeta = nmetas();
      int height = 20;

      int left = horizontalScrollBar()->value();
      int right = horizontalScrollBar()->value() + viewport()->width();
      int top = verticalScrollBar()->value() + nmeta * height;
      int bottom = verticalScrollBar()->value() + viewport()->height();

      bool extend_up = false, extend_left = false, extend_right = false, extend_down = false;

      if (selection.top() < top) extend_up = true;
      if (selection.left() < left - 1) extend_left = true;
      if (selection.right() > right) extend_right = true;
      if (selection.bottom() > bottom) extend_down = true;

      if (extend_down && old_selection.bottom() != selection.bottom() && !meta_value)
            verticalScrollBar()->setValue(int(verticalScrollBar()->value() +
                                              selection.bottom() - bottom));
      else if (extend_up && !extend_down && old_selection.bottom() != selection.bottom() && !meta_value && old_selection.contains(selection))
                  verticalScrollBar()->setValue(int(verticalScrollBar()->value() +
                                                      selection.bottom() - bottom));

      if (extend_right && old_selection.right() != selection.right())
            horizontalScrollBar()->setValue(int(horizontalScrollBar()->value() +
                                                selection.right() - right));
      if (extend_up && old_selection.top() != selection.top() && !meta_value)
            verticalScrollBar()->setValue(int(selection.top()) - nmeta * height);
      if (extend_left && old_selection.left() != selection.left())
            horizontalScrollBar()->setValue(int(selection.left()));

      if (extend_left && !extend_right && old_selection.right() != selection.right() && old_selection.contains(selection)) {
            horizontalScrollBar()->setValue(int(horizontalScrollBar()->value() +
                                                selection.right() - right));
            }
      if (extend_right && !extend_left && old_selection.left() != selection.left() && old_selection.contains(selection)) {
            horizontalScrollBar()->setValue(int(selection.left()));
            }

      if (extend_down && !extend_up && old_selection.top() != selection.top() && !meta_value && old_selection.contains(selection)) {
            verticalScrollBar()->setValue(int(selection.top()) - nmeta * height);
            }

      old_selection = selection;

      meta_value = false;
      scene()->blockSignals(false);
      }

//---------------------------------------------------------
//   drawSelection
//---------------------------------------------------------

void Timeline::drawSelection()
      {
      path = QPainterPath();
      path.setFillRule(Qt::WindingFill);

      std::set<std::tuple<Measure*, int, ElementType>> ml;

      const Selection sel = _score->selection();
      QList<Element*> el = sel.elements();
      for (Element* e : el) {


            if (e->tick() == -1)
                  continue;
            else {
                  switch (e->type()) {
                        case ElementType::INSTRUMENT_NAME:
                        case ElementType::VBOX:
                        case ElementType::HBOX:
                        case ElementType::TEXT:
                        case ElementType::TIE_SEGMENT:
                        case ElementType::SLUR_SEGMENT:
                        case ElementType::TIE:
                        case ElementType::SLUR:
                              continue;
                              break;
                        default: break;
                        }
                  }

            int staffIdx;
            Measure* m = _score->tick2measure(e->tick());
            staffIdx = e->staffIdx();

            if ((e->type() == ElementType::TEMPO_TEXT ||
                 e->type() == ElementType::KEYSIG ||
                 e->type() == ElementType::TIMESIG||
                 e->type() == ElementType::REHEARSAL_MARK) &&
                !e->generated())
                  staffIdx = -1;

            //e->type() for meta rows, invalid for everything else
            ElementType etype = (staffIdx == -1)? e->type():ElementType::INVALID;

            std::tuple<Measure*, int, ElementType> tmp(m, staffIdx, etype);
            ml.insert(tmp);

            }

      QList<QGraphicsItem*> gl = scene()->items();
      for (QGraphicsItem* gi : gl) {
            int stave = gi->data(0).value<int>();
            ElementType etype = gi->data(1).value<ElementType>();
            Measure *m = static_cast<Measure*>(gi->data(2).value<void*>());

            std::tuple<Measure*, int, ElementType> tmp(m, stave, etype);
            std::set<std::tuple<Measure*, int, ElementType>>::iterator it;
            it = ml.find(tmp);
            if (stave == -1 && it != ml.end()) {
                  //Make sure the element is correct
                  QList<Element*> el = _score->selection().elements();
                  Element* target = static_cast<Element*>(gi->data(4).value<void*>());
                  if (target) {
                        for (Element* e : el) {
                              if (e == target) {
                                    QGraphicsRectItem* r = dynamic_cast<QGraphicsRectItem*>(gi);
                                    if (r)
                                          r->setBrush(QBrush(QColor(173,216,230)));
                                    }
                              }
                        }
                  else {
                        QGraphicsRectItem* r = dynamic_cast<QGraphicsRectItem*>(gi);
                        if (r)
                              r->setBrush(QBrush(QColor(173,216,230)));
                        }
                  }
            //Change color from gray to only blue
            else if (it != ml.end()) {
                  QGraphicsRectItem* r = dynamic_cast<QGraphicsRectItem*>(gi);
                  r->setBrush(QBrush(QColor(r->brush().color().red(),
                                            r->brush().color().green(),
                                            255)));
                  path.addRect(gi->boundingRect());
                  }
            }

      QGraphicsPathItem* pi = new QGraphicsPathItem(path.simplified());
      pi->setPen(QPen(QColor(0, 0, 255), 3));
      pi->setBrush(Qt::NoBrush);
      pi->setZValue(-1);
      scene()->addItem(pi);
      }

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void Timeline::mousePressEvent(QMouseEvent* ev)
      {
      if (!_score)
            return;

      //Set as clicked
      mouse_pressed = true;
      scene()->clearSelection();

      QPointF scenePt = mapToScene(ev->pos());
      //Set as old location
      old_loc = QPoint(int(scenePt.x()), int(scenePt.y()));
      if (QGraphicsItem* item = scene()->itemAt(scenePt, transform())) {

            int stave = item->data(0).value<int>();

            Measure* curr = static_cast<Measure*>(item->data(2).value<void*>());
            if (!curr) {
                  int nmeta = nmetas();
                  int height = 20;
                  int end_meta = nmeta*height + verticalScrollBar()->value();

                  //Handle measure box clicks
                  if (scenePt.y() > (nmeta - 1) * height + verticalScrollBar()->value() &&
                      scenePt.y() < end_meta) {

                        QRectF tmp(scenePt.x(), 0, 3, nmeta*height + nstaves()*height);
                        QList<QGraphicsItem*> gil = scene()->items(tmp);
                        Measure* m = nullptr;

                        for (QGraphicsItem* gi : gil) {
                              m = static_cast<Measure*>(gi->data(2).value<void*>());

                              //-3 z value is the grid square values
                              if (gi->zValue() == -3 && m) {
                                    break;
                                    }

                              }
                        if (m) {
                              _cv->adjustCanvasPosition(m, false);
                              }
                        }
                  if (scenePt.y() < end_meta) return;

                  QList<QGraphicsItem*> gl = items(ev->pos());
                  for (QGraphicsItem* gi : gl) {
                        curr = static_cast<Measure*>(gi->data(2).value<void*>());
                        stave = gi->data(0).value<int>();
                        if (curr) break;
                        }
                  if (!curr) {
                        _score->select(0, SelectType::SINGLE, 0);
                        return;
                        }
                  }

            bool full_measure = item->data(3).value<bool>();

            scene()->clearSelection();
            if (full_measure) {

                  meta_value = true;
                  old_selection = QRect();

                  _cv->adjustCanvasPosition(curr, false, 0);
                  verticalScrollBar()->setValue(0);

                  //Also select the elements that they correspond to
                  ElementType etype = item->data(1).value<ElementType>();
                  SegmentType stype = SegmentType::Invalid;
                  if (etype == ElementType::KEYSIG) stype = SegmentType::KeySig;
                  else if (etype == ElementType::TIMESIG) stype = SegmentType::TimeSig;
                  if (stype != SegmentType::Invalid) {
                        Segment* s = curr->first();
                        for (; s && s->segmentType() != stype; s = s->next()) {

                              }
                        if (s) {
                              _score->deselectAll();
                              for (int j = 0; j < _score->nstaves(); j++) {
                                    Element* e = s->firstElement(j);
                                    if (e)
                                          _score->select(e, SelectType::ADD);
                                    }
                              }
                        }
                  else {
                        _score->deselectAll();
                        _score->select(curr, SelectType::ADD, 0);
                        //Select just the element for tempo_text
                        Element* e = static_cast<Element*>(item->data(4).value<void*>());
                        _score->deselectAll();
                        _score->select(e);
                        }
                  }
            else {

                  if (ev->modifiers() == Qt::ShiftModifier) {
                        _score->select(curr, SelectType::RANGE, stave);
                        _score->setUpdateAll();
                        }
                  else if (ev->modifiers() == Qt::ControlModifier) {

                        if (_score->selection().isNone()) {
                              _score->select(curr, SelectType::RANGE, 0);
                              _score->select(curr, SelectType::RANGE, _score->nstaves()-1);
                              _score->setUpdateAll();
                              }
                        else
                              _score->deselectAll();
                        }
                  else {
                        _score->select(curr, SelectType::SINGLE, stave);
                        }
                  _cv->adjustCanvasPosition(curr, false, stave);

                  }
            mscore->endCmd();
            _score->update();
            }
      else {
            _score->deselectAll();
            }
      _score->setUpdateAll();
      }

//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void Timeline::mouseMoveEvent(QMouseEvent *event)
      {
      if (!mouse_pressed)
            return;

      if (state == ViewState::NORMAL) {
            if (event->modifiers() == Qt::ShiftModifier) {
                  QPointF new_loc = mapToScene(event->pos());
                  //Slight wiggle room for selection (Same as score)
                  if (abs(new_loc.x() - old_loc.x()) > 2 ||
                      abs(new_loc.y() - old_loc.y()) > 2) {
                        _score->deselectAll();
                        updateGrid();
                        state = ViewState::LASSO;
                        selection_box = new QGraphicsRectItem();
                        selection_box->setRect(old_loc.x(), old_loc.y(), 0, 0);
                        selection_box->setPen(QPen(QColor(0, 0, 255), 2));
                        selection_box->setBrush(QBrush(QColor(0, 0, 255, 50)));
                        scene()->addItem(selection_box);
                        }
                  }
            else {
                  state = ViewState::DRAG;
                  this->setCursor(Qt::SizeAllCursor);
                  }
            }

      if (state == ViewState::LASSO) {
            QPointF scene_point = mapToScene(event->pos());
            QRect tmp = QRect((old_loc.x() < scene_point.x())? old_loc.x() : scene_point.x(),
                              (old_loc.y() < scene_point.y())? old_loc.y() : scene_point.y(),
                              abs(scene_point.x() - old_loc.x()),
                              abs(scene_point.y() - old_loc.y()));
            selection_box->setRect(tmp);
            }
      else if (state == ViewState::DRAG) {
            QPointF new_loc = mapToScene(event->pos());
            int xo = int(old_loc.x()) - int(new_loc.x());
            int yo = int(old_loc.y()) - int(new_loc.y());
            horizontalScrollBar()->setValue(horizontalScrollBar()->value() + xo);
            verticalScrollBar()->setValue(verticalScrollBar()->value() + yo);
            }
      }

//---------------------------------------------------------
//   mouseReleaseEvent
//---------------------------------------------------------

void Timeline::mouseReleaseEvent(QMouseEvent *)
      {
      mouse_pressed = false;
      if (state == ViewState::LASSO) {

            scene()->removeItem(selection_box);
            _score->deselectAll();

            int w, h;
            QPoint loc = mapFromScene(selection_box->rect().topLeft());
            w = int(selection_box->rect().width());
            h = int(selection_box->rect().height());

            QList<QGraphicsItem*> gl = items(QRect(loc.x(), loc.y(), w, h));
            //Find top left and bottom right to create selection
            QGraphicsItem* tl = nullptr;

            QGraphicsItem* br = nullptr;
            for (QGraphicsItem* item : gl) {
                  Measure* curr = static_cast<Measure*>(item->data(2).value<void*>());
                  if (!curr) continue;
                  int stave = item->data(0).value<int>();
                  if (stave == -1) continue;

                  if (!tl && !br) {
                        tl = item;
                        br = item;
                        continue;
                        }

                  if (item->boundingRect().top() < tl->boundingRect().top())
                        tl = item;
                  if (item->boundingRect().left() < tl->boundingRect().left())
                        tl = item;

                  if (item->boundingRect().bottom() > br->boundingRect().bottom())
                        br = item;
                  if (item->boundingRect().right() > br->boundingRect().right())
                        br = item;

                  }

            //Select single tl and then range br
            if (tl && br) {
                  Measure* m1 = static_cast<Measure*>(tl->data(2).value<void*>());
                  int s1 = tl->data(0).value<int>();
                  Measure* m2 = static_cast<Measure*>(br->data(2).value<void*>());
                  int s2 = br->data(0).value<int>();
                  if (m1 && m2) {
                        _score->select(m1, SelectType::SINGLE, s1);
                        _score->select(m2, SelectType::RANGE, s2);
                        }
                  _cv->adjustCanvasPosition(m1, false, s1);
                  }

            _score->update();
            mscore->endCmd();
            }
      else if (state == ViewState::DRAG) {
            this->setCursor(Qt::ArrowCursor);
            mscore->endCmd();
            }
      state = ViewState::NORMAL;
      }

//---------------------------------------------------------
//   updateGrid
//---------------------------------------------------------

void Timeline::updateGrid()
      {
      if (_score) {
            drawGrid(nstaves(), _score->nmeasures());
            updateView(0, 0);
            drawSelection();
            }

      viewport()->update();
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Timeline::setScore(Score* s)
      {
      _score = s;
      scene()->clear();

      if (_score) {
            drawGrid(nstaves(), _score->nmeasures());
            changeSelection(SelState::NONE);
            }
      else {
            //Clear timeline if no score is present
            QSplitter* s = scrollArea->grid();
            if (s && s->count() > 0) {
                  TRowLabels* t = static_cast<TRowLabels*>(s->widget(0));
                  std::vector<QString> empty;
                  t->updateLabels(empty, 0);
                  }
            meta_rows.clear();
            setSceneRect(0, 0, 0, 0);
            }

      viewport()->update();
      }

//---------------------------------------------------------
//   setScoreView
//---------------------------------------------------------

void Timeline::setScoreView(ScoreView* v)
      {
      _cv = v;
      if (_cv) {
            connect(_cv, SIGNAL(offsetChanged(double, double)), this, SLOT(updateView(double, double)));
            updateView(0, 0);
            }
      }

//---------------------------------------------------------
//   updateView
//---------------------------------------------------------

void Timeline::updateView(double, double)
      {
      if (_cv) {
            QRectF canvas = QRectF(_cv->matrix().inverted().mapRect(_cv->geometry()));

            std::set<std::pair<Measure*, int>> visible_items;

            //Find visible measures of score
            for (Measure* m = _score->firstMeasure(); m; m = m->nextMeasure()) {
                  System* sys = m->system();

                  if (m->mmRest() && _score->styleB(StyleIdx::createMultiMeasureRests)) {
                        //Handle mmRests
                        Measure* mmr = m->mmRest();
                        sys = mmr->system();
                        if (!sys)
                              continue;

                        //Add all measures within mmRest to visible_items if mmRest_visible
                        for (; m != mmr->mmRestLast(); m = m->nextMeasure()) {
                              for (int staff = 0; staff < _score->staves().length(); staff++) {
                                    if (!_score->staff(staff)->show())
                                          continue;
                                    QRectF stave = QRectF(sys->canvasBoundingRect().left(),
                                                          sys->staffCanvasYpage(staff),
                                                          sys->width(),
                                                          sys->staff(staff)->bbox().height());
                                    QRectF showRect = mmr->canvasBoundingRect().intersected(stave);

                                    if (canvas.intersects(showRect)) {
                                          std::pair<Measure*, int> p(m, staff);
                                          visible_items.insert(p);
                                          }
                                    }
                              }

                        //Handle last measure in mmRest
                        for (int staff = 0; staff < _score->staves().length(); staff++) {
                              if (!_score->staff(staff)->show())
                                    continue;
                              QRectF stave = QRectF(sys->canvasBoundingRect().left(),
                                                    sys->staffCanvasYpage(staff),
                                                    sys->width(),
                                                    sys->staff(staff)->bbox().height());
                              QRectF showRect = mmr->canvasBoundingRect().intersected(stave);

                              if (canvas.intersects(showRect)) {
                                    std::pair<Measure*, int> p(m, staff);
                                    visible_items.insert(p);
                                    }
                              }
                        continue;
                        }

                  if (!sys)
                        continue;

                  for (int staff = 0; staff < _score->staves().length(); staff++) {
                        if (!_score->staff(staff)->show())
                              continue;
                        QRectF stave = QRectF(sys->canvasBoundingRect().left(),
                                              sys->staffCanvasYpage(staff),
                                              sys->width(),
                                              sys->staff(staff)->bbox().height());
                        QRectF showRect = m->canvasBoundingRect().intersected(stave);

                        if (canvas.intersects(showRect)) {
                              std::pair<Measure*, int> p(m, staff);
                              visible_items.insert(p);
                              }


                        }
                  }

            //Find respective visible elements in timeline
            QPainterPath pp = QPainterPath();
            pp.setFillRule(Qt::WindingFill);
            for (QGraphicsItem* gi : scene()->items()) {
                  int stave = gi->data(0).value<int>();
                  Measure* m = static_cast<Measure*>(gi->data(2).value<void*>());

                  std::pair<Measure*, int> tmp(m, stave);
                  std::set<std::pair<Measure*, int>>::iterator it;
                  it = visible_items.find(tmp);
                  if (it != visible_items.end())
                        pp.addRect(gi->boundingRect());
                  }

            QPainterPath final = QPainterPath();
            final.setFillRule(Qt::WindingFill);

            QRectF rf = QRectF(0, 0, getWidth(), getHeight());
            final.addRect(rf);

            final = final.subtracted(pp);

            QGraphicsPathItem* non_visible = new QGraphicsPathItem(final.simplified());

            QPen non_visible_p = QPen(QColor(0, 150, 150), 2);
            QBrush non_visible_b = QBrush(QColor(0, 150, 150, 50));
            non_visible->setPen(QPen(non_visible_b.color()));
            non_visible->setBrush(non_visible_b);
            non_visible->setZValue(-3);

            QGraphicsPathItem* visible = new QGraphicsPathItem(pp.simplified());
            visible->setPen(non_visible_p);
            visible->setBrush(Qt::NoBrush);
            visible->setZValue(-2);

            //Find old path, remove it
            for (QGraphicsItem* gi : scene()->items()) {
                  if (gi->type() == QGraphicsPathItem().type()) {
                        QGraphicsPathItem* p = static_cast<QGraphicsPathItem*>(gi);
                        QBrush b1 = p->brush();
                        QPen p1 = p->pen();
                        if (b1 == non_visible_b || p1 == non_visible_p)
                              scene()->removeItem(gi);
                        }

                  }

            scene()->addItem(non_visible);
            scene()->addItem(visible);
            }
      else {
            qDebug() << "No scoreview found.";
            }

      }



//---------------------------------------------------------
//   nstaves
//---------------------------------------------------------

int Timeline::nstaves()
      {
      QList<Staff*> list = _score->staves();
      int total = 0;
      for (Staff* s : list) {
            if (s->show())
                  total++;
            }
      return total;
      }

//---------------------------------------------------------
//   colorBox
//---------------------------------------------------------

QColor Timeline::colorBox(QGraphicsRectItem* item)
      {
      Measure* meas = static_cast<Measure*>(item->data(2).value<void*>());
      int stave = item->data(0).value<int>();
      for (Segment* s = meas->first(); s; s = s->next()) {
            if (s->segmentType() != SegmentType::ChordRest) {
                  continue;
                  }
            for (int i = stave * VOICES; i < stave * VOICES + VOICES; i++) {
                  ChordRest* cr = s->cr(i);
                  if (cr) {
                        if (cr->type() == ElementType::CHORD) {
                              return QColor(Qt::gray);
                              }
                        }
                  }
            }
      return QColor(211,211,211);
      }

//---------------------------------------------------------
//   colorBox
//---------------------------------------------------------

std::vector<QString> Timeline::getLabels()
      {
      if (!_score) {
            std::vector<QString> empty;
            return empty;
            }
      QList<Part*> pl = _score->parts();
      //transfer them into a vector of qstrings and then add the meta row names
      std::vector<QString> labels;
      labels.push_back(tr("Tempo"));
      labels.push_back(tr("Time Signature"));
      labels.push_back(tr("Key Signature"));
      labels.push_back(tr("Rehearsal Mark"));
      labels.push_back(tr("Measures"));
      for (int r = 0; r < nstaves(); r++) {
            QTextDocument doc;
            doc.setHtml(pl.at(correctPart(r))->longName());
            QString name = doc.toPlainText();
            labels.push_back(name);
            }
      return labels;
      }

//---------------------------------------------------------
//   handle_scroll
//---------------------------------------------------------

void Timeline::handle_scroll(int value)
      {
      if (!_score)
            return;
      for (std::vector<std::pair<QGraphicsItem*, int>>::iterator it = meta_rows.begin();
           it != meta_rows.end(); ++it) {
            std::pair<QGraphicsItem*, int> p = *it;

            QGraphicsItem* gi = p.first;
            QGraphicsRectItem* ri = dynamic_cast<QGraphicsRectItem*>(gi);
            QGraphicsLineItem* li = dynamic_cast<QGraphicsLineItem*>(gi);
            int r = p.second * 20;
            int v = value;
            if (ri) {
                  QRectF rf = ri->rect();
                  rf.setY(qreal(v + r));
                  rf.setHeight(20);
                  ri->setRect(rf);
                  }
            else if (li) {
                  QLineF lf = li->line();
                  lf.setLine(lf.x1(), r + v + 1, lf.x2(), r + v + 1);
                  li->setLine(lf);
                  }
            else {
                  gi->setY(qreal(v + r));
                  }
            }
      viewport()->update();

      }

}
