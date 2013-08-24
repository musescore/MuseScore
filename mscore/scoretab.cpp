//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2009-2010 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "config.h"
#include "scoretab.h"
#include "scoreview.h"
#include "scoreview.h"
#include "libmscore/score.h"
#include "magbox.h"
#ifdef OMR
#include "omr/omr.h"
#include "omr/omrview.h"
#endif
#include "libmscore/excerpt.h"

namespace Ms {

//---------------------------------------------------------
//   ScoreTab
//---------------------------------------------------------

ScoreTab::ScoreTab(QList<Score*>* sl, QWidget* parent)
   : QWidget(parent)
      {
      scoreList = sl;
      QVBoxLayout* layout = new QVBoxLayout;
      setLayout(layout);
      layout->setSpacing(0);
      layout->setMargin(2);

      tab = new QTabBar;
      tab->setExpanding(false);
      tab->setSelectionBehaviorOnRemove(QTabBar::SelectRightTab);
      tab->setFocusPolicy(Qt::StrongFocus);
      tab->setTabsClosable(true);

      tab2 = new QTabBar;
      tab2->setExpanding(false);
      tab2->setSelectionBehaviorOnRemove(QTabBar::SelectRightTab);
      tab2->setFocusPolicy(Qt::StrongFocus);
      tab2->setVisible(false);
      tab2->setTabsClosable(false);

      stack = new QStackedLayout;
      layout->addWidget(tab);
      layout->addWidget(tab2);
      layout->addLayout(stack);

      foreach(Score* s, *sl)
            insertTab(s);

      connect(tab, SIGNAL(currentChanged(int)), this, SLOT(setCurrent(int)));
      connect(tab2, SIGNAL(currentChanged(int)), this, SLOT(setExcerpt(int)));
      connect(tab, SIGNAL(tabCloseRequested(int)), this, SIGNAL(tabCloseRequested(int)));
      }

//---------------------------------------------------------
//   view
//---------------------------------------------------------

ScoreView* ScoreTab::view(int n) const
      {
      QSplitter* s = viewSplitter(n);
      if (s)
            return static_cast<ScoreView*>(s->widget(0));
      return 0;
      }

//---------------------------------------------------------
//   viewSplitter
//---------------------------------------------------------

QSplitter* ScoreTab::viewSplitter(int n) const
      {
      TabScoreView* tsv = static_cast<TabScoreView*>(tab->tabData(n).value<void*>());
      if (tsv == 0) {
            // qDebug("ScoreTab::viewSplitter %d is zero\n", n);
            return 0;
            }
      Score* score = tsv->score;
      if (tsv->part) {
            QList<Excerpt*>& excerpts = score->excerpts();
            if (!excerpts.isEmpty() && ((tsv->part - 1) < excerpts.size()))
                  score = excerpts.at(tsv->part - 1)->score();
            }

      int nn = stack->count();
      for (int i = 0; i < nn; ++i) {
            QSplitter* sp = static_cast<QSplitter*>(stack->widget(i));
            if (sp->count() == 0)
                  return 0;
            ScoreView* v = static_cast<ScoreView*>(sp->widget(0));
            if (v->score() == score)
                  return sp;
            }
      return 0;
      }

//---------------------------------------------------------
//   clearTab2
//---------------------------------------------------------

void ScoreTab::clearTab2()
      {
      tab2->blockSignals(true);
      int n = tab2->count();
      for (int i = 0; i < n; ++i)
            tab2->removeTab(0);
      tab2->blockSignals(false);
      }

//---------------------------------------------------------
//   setCurrent
//---------------------------------------------------------

void ScoreTab::setCurrent(int n)
      {
      if (n == -1) {
            clearTab2();
            tab2->setVisible(false);
            // clearTab2();      //??
            emit currentScoreViewChanged(0);
            return;
            }
      TabScoreView* tsv = static_cast<TabScoreView*>(tab->tabData(n).value<void*>());
      QSplitter* vs = viewSplitter(n);

      ScoreView* v;
      if (!vs) {
            vs = new QSplitter;
            v  = new ScoreView;
            tab2->blockSignals(true);
            tab2->setCurrentIndex(0);
            tab2->blockSignals(false);
            vs->addWidget(v);
            v->setScore(scoreList->value(n));
            stack->addWidget(vs);
            }
      else {
            v = static_cast<ScoreView*>(vs->widget(0));
            }
#ifdef OMR
      if (v) {
            Score* score = v->score();
            if (score->showOmr() && score->omr()) {
                  if (vs->count() < 2) {
                        Omr* omr    = score->omr();
                        OmrView* sv = omr->newOmrView(v);
                        v->setOmrView(sv);
                        vs->addWidget(sv);
                        connect(v, SIGNAL(scaleChanged(double)), sv, SLOT(setScale(double)));
                        connect(v, SIGNAL(offsetChanged(double,double)), sv, SLOT(setOffset(double,double)));
                        const QTransform _matrix = v->matrix();
                        double _spatium = score->spatium();
                        double scale = _matrix.m11() * _spatium;
                        sv->setScale(scale);
                        sv->setOffset(_matrix.dx(), _matrix.dy());
                        QList<int> sizes;
                        sizes << 100 << 100;
                        vs->setSizes(sizes);
                        }
                  }
            else {
                  if (vs->count() > 1) {
                        QWidget* w = vs->widget(1);
                        delete w;
                        }
                  }
            }
#endif
      stack->setCurrentWidget(vs);
      clearTab2();
      if (v) {
            Score* score = v->score();
            if (score->parentScore())
                  score = score->parentScore();
            QList<Excerpt*>& excerpts = score->excerpts();
            if (!excerpts.isEmpty()) {
                  tab2->blockSignals(true);
                  tab2->addTab(score->name().replace("&","&&"));
                  foreach(const Excerpt* excerpt, excerpts) {
                        tab2->addTab(excerpt->score()->name().replace("&","&&"));
                        }
                  tab2->setCurrentIndex(tsv->part);
                  tab2->blockSignals(false);
                  tab2->setVisible(true);
                  }
            else {
                  tab2->setVisible(false);
                  }
            }
      else {
            tab2->setVisible(false);
            }
      emit currentScoreViewChanged(v);
      }

//---------------------------------------------------------
//   updateExcerpts
//    number of excerpts in score changed
//---------------------------------------------------------

void ScoreTab::updateExcerpts()
      {
      int idx = currentIndex();
      if (idx == -1)
            return;
      ScoreView* v = view(idx);
      if (!v)
            return;
      Score* score = v->score()->rootScore();
      clearTab2();
      //delete all scoreviews for parts, especially for the deleted ones
      int n = stack->count() - 1;
      for (int i = n; i >= 0; --i) {
            QSplitter* vs = static_cast<QSplitter*>(stack->widget(i));
            ScoreView* sview = static_cast<ScoreView*>(vs->widget(0));

            if (sview->score() != score && sview->score()->rootScore() == score) {
                  stack->takeAt(i);
                  sview->deleteLater();
                  }
            }

      QList<Excerpt*>& excerpts = score->excerpts();
      if (!excerpts.isEmpty()) {
            tab2->blockSignals(true);
            tab2->addTab(score->name().replace("&","&&"));
            foreach(const Excerpt* excerpt, excerpts)
                  tab2->addTab(excerpt->score()->name().replace("&","&&"));
            tab2->blockSignals(false);
            tab2->setVisible(true);

            setExcerpt(0);
            }
      else {
            tab2->setVisible(false);
            setExcerpt(0);
            }
      }

//---------------------------------------------------------
//   setExcerpt
//---------------------------------------------------------

void ScoreTab::setExcerpt(int n)
      {
      if (n == -1)
            return;
      int idx           = tab->currentIndex();
      TabScoreView* tsv = static_cast<TabScoreView*>(tab->tabData(idx).value<void*>());
      if (tsv == 0)
            return;
      tsv->part     = n;
      QSplitter* vs = viewSplitter(idx);
      ScoreView* v;
      Score* score = tsv->score;
      if (n) {
            QList<Excerpt*>& excerpts = score->excerpts();
            if (!excerpts.isEmpty()) {
                  score = excerpts.at(n - 1)->score();
                  }
            }
      if (!vs) {
            vs = new QSplitter;
            v = new ScoreView;
            vs->addWidget(v);
            v->setScore(score);
            stack->addWidget(vs);
            }
      else
            v = static_cast<ScoreView*>(vs->widget(0));
      stack->setCurrentWidget(vs);
      emit currentScoreViewChanged(v);
      }

//---------------------------------------------------------
//   insertTab
//---------------------------------------------------------

void ScoreTab::insertTab(Score* s)
      {
      int idx = scoreList->indexOf(s);
      tab->blockSignals(true);
      tab->insertTab(idx, s->name().replace("&","&&"));
      tab->setTabData(idx, QVariant::fromValue<void*>(new TabScoreView(s)));
      tab->blockSignals(false);
      }

//---------------------------------------------------------
//   setTabText
//---------------------------------------------------------

void ScoreTab::setTabText(int idx, const QString& s)
      {
      QString text(s);
      text.replace("&","&&");
      tab->setTabText(idx, text);
      if (tab2)
            tab2->setTabText(0, text);
      }

//---------------------------------------------------------
//   currentIndex
//---------------------------------------------------------

int ScoreTab::currentIndex() const
      {
      return tab->currentIndex();
      }

//---------------------------------------------------------
//   setCurrentIndex
//---------------------------------------------------------

void ScoreTab::setCurrentIndex(int idx)
      {
      if (currentIndex() == idx)
            setCurrent(idx);
      else
            tab->setCurrentIndex(idx);
      tab->setTabText(idx, tab->tabText(idx));  // HACK
      }

//---------------------------------------------------------
//   removeTab
//---------------------------------------------------------

void ScoreTab::removeTab(int idx)
      {
      TabScoreView* tsv = static_cast<TabScoreView*>(tab->tabData(idx).value<void*>());
      Score* score = tsv->score;

      for (int i = 0; i < stack->count(); ++i) {
            QSplitter* vs = static_cast<QSplitter*>(stack->widget(i));
            ScoreView* v = static_cast<ScoreView*>(vs->widget(0));
            if (v->score() == score) {
                  stack->takeAt(i);
                  delete v;
                  break;
                  }
            }
      foreach (Excerpt* excerpt, score->excerpts()) {
            Score* sc = excerpt->score();
            for (int i = 0; i < stack->count(); ++i) {
                  QSplitter* vs = static_cast<QSplitter*>(stack->widget(i));
                  ScoreView* v = static_cast<ScoreView*>(vs->widget(0));
                  if (v->score() == sc) {
                        stack->takeAt(i);
                        delete v;
                        break;
                        }
                  }
            }

      int cidx = currentIndex();
      tab->removeTab(idx);

      if (cidx > idx)
            cidx -= 1;
      setCurrentIndex(cidx);
      }

//---------------------------------------------------------
//   initScoreView
//---------------------------------------------------------

void ScoreTab::initScoreView(int idx, double mag, int magIdx, double xoffset, double yoffset)
      {
      ScoreView* v = view(idx);
      if (!v)  {
            v = new ScoreView;
            Score* sc = scoreList->value(idx);
            if( sc != 0 )
                  v->setScore(sc);
            else {
                  delete v;
                  return;
                  }
            QSplitter* vs = new QSplitter;
            vs->addWidget(v);
            stack->addWidget(vs);
            }
      v->setMag(magIdx, mag);
      v->setOffset(xoffset, yoffset);
      }
}

