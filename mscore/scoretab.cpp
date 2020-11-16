//=============================================================================
//  MuseScore
//  Linux Music Score Editor
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

#include <QAction>

#include "log.h"

#include "shortcut.h"
#include "musescore.h"
#include "config.h"
#include "scoretab.h"
#include "scoreview.h"
#include "libmscore/score.h"
#include "zoombox.h"
#ifdef OMR
#include "omr/omr.h"
#include "omr/omrview.h"
#endif

#ifdef AVSOMR
#include "avsomr/avsomr.h"
#include "avsomr/ui/setupavsomrview.h"
#endif

#include "libmscore/excerpt.h"

namespace Ms {

//---------------------------------------------------------
//   MsTabBar::mousePressEvent
//---------------------------------------------------------

void MsTabBar::mousePressEvent(QMouseEvent* e)
      {
      QTabBar::mousePressEvent(e);
      if (e->button() == Qt::MiddleButton)
            _middleClickedTab = tabAt(e->pos());
      }

//---------------------------------------------------------
//   MsTabBar::mouseReleaseEvent
//---------------------------------------------------------

void MsTabBar::mouseReleaseEvent(QMouseEvent* e)
      {
      QTabBar::mouseReleaseEvent(e);
      if (e->button() == Qt::MiddleButton) {
            if (tabAt(e->pos()) == _middleClickedTab)
                  emit tabCloseRequested(_middleClickedTab);
            _middleClickedTab = -1; // reset
            }
      }

//---------------------------------------------------------
//   ScoreTab
//---------------------------------------------------------

ScoreTab::ScoreTab(QList<MasterScore*>* sl, QWidget* parent)
   : QWidget(parent)
      {
      setObjectName("scoretab");
      setAccessibleName("");
      mainWindow = static_cast<MuseScore*>(parent);
      scoreList = sl;
      QVBoxLayout* layout = new QVBoxLayout;
      setLayout(layout);
      layout->setSpacing(0);
      layout->setMargin(2);

      QActionGroup* ag = Shortcut::getActionGroupForWidget(MsWidget::SCORE_TAB, Qt::WidgetWithChildrenShortcut);
      ag->setParent(this);
      this->addActions(ag->actions());

      connect(ag, SIGNAL(triggered(QAction*)), this, SIGNAL(actionTriggered(QAction*)));

      tab = new MsTabBar(this);
      tab->setObjectName("primarytab");
      tab->setAccessibleName("");
      tab->setExpanding(false);
      tab->setSelectionBehaviorOnRemove(QTabBar::SelectRightTab);
      tab->setFocusPolicy(Qt::ClickFocus);
      tab->setTabsClosable(true);
      tab->setMovable(true);

      tab2 = new MsTabBar(this);
      tab2->setObjectName("secondarytab");
      tab2->setAccessibleName("");
      tab2->setExpanding(false);
      tab2->setSelectionBehaviorOnRemove(QTabBar::SelectRightTab);
      tab2->setFocusPolicy(Qt::ClickFocus);
      tab2->setVisible(false);
      tab2->setTabsClosable(false);

      stack = new QStackedLayout;
      layout->addWidget(tab);
      layout->addWidget(tab2);
      layout->addLayout(stack);

      for (MasterScore* s : *sl)
            insertTab(s);

      connect(tab, SIGNAL(currentChanged(int)), this, SLOT(setCurrent(int)));
      connect(tab2, SIGNAL(currentChanged(int)), this, SLOT(setExcerpt(int)));
      connect(tab, SIGNAL(tabCloseRequested(int)), this, SIGNAL(tabCloseRequested(int)));
      connect(tab, SIGNAL(tabMoved(int,int)), this, SLOT(tabMoved(int,int)));
      }

ScoreTab::~ScoreTab()
      {
//      while (tab->count() > 0)
//            tab->removeTab(0);
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
      const TabScoreView* tsv = tabScoreView(n);
      if(!tsv) {
            return nullptr;
            }

      Score* score = tsv->score;
      if (tsv->part) {
            QList<Excerpt*>& excerpts = score->excerpts();
            if (!excerpts.isEmpty() && ((tsv->part - 1) < excerpts.size()))
                  score = excerpts.at(tsv->part - 1)->partScore();
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
//   tabScoreView
//---------------------------------------------------------

TabScoreView* ScoreTab::tabScoreView(int idx)
      {
      if (!tab) {
            return nullptr;
            }

      QVariant tabData = tab->tabData(idx);
      if (!tabData.isValid()) {
            return nullptr;
            }

      return static_cast<TabScoreView*>(tabData.value<void*>());
      }

//---------------------------------------------------------
//   tabScoreView const
//---------------------------------------------------------

const TabScoreView* ScoreTab::tabScoreView(int idx) const
      {
      return const_cast<ScoreTab*>(this)->tabScoreView(idx);
      }

//---------------------------------------------------------
//   tabMoved
//---------------------------------------------------------

void ScoreTab::tabMoved(int from, int to)
      {
      static bool scoreListChanged = false;
      if (scoreListChanged == false) {
            qDebug("Moved score tab %d to %d", from, to);
            scoreList->move(from, to);

            // now move the tab in the other ScoreTab...but don't update the scoreList a second time!
            scoreListChanged = true;
            if (this == mscore->getTab1()) {
                  if (mscore->getTab2())
                        mscore->getTab2()->getTab()->moveTab(from, to);
                  }
            else if (this == mscore->getTab2()) {
                  if (mscore->getTab1())
                        mscore->getTab1()->getTab()->moveTab(from, to);
                  }
            scoreListChanged = false;
            }
      }

//---------------------------------------------------------
//   setCurrent
//    Sets current view to the content of n-th tab.
//    You will rarely need to call this function directly,
//    consider using setCurrentIndex instead.
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

      QSplitter* vs = viewSplitter(n);

      ScoreView* v;
      if (!vs) {
            vs = new QSplitter;
            vs->setObjectName("score");
            vs->setAccessibleName("");
            v  = new ScoreView;
            tab2->blockSignals(true);
            tab2->setCurrentIndex(0);
            tab2->blockSignals(false);
            vs->addWidget(v);
            v->setScore(scoreList->value(n));
            stack->addWidget(vs);
#ifdef AVSOMR
            Score* score = v->score();
            if (score->masterScore()->avsOmr()) {
                  Avs::SetupAvsOmrView setuperView;
                  setuperView.setupView(v, score->masterScore()->avsOmr());
                  }
#endif
            }
      else {
            v = static_cast<ScoreView*>(vs->widget(0));
            }
#ifdef OMR
      if (v) {
            Score* score = v->score();
            if (score->masterScore()->showOmr() && score->masterScore()->omr()) {
                  if (vs->count() < 2) {
                        Omr* omr    = score->masterScore()->omr();
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
            MasterScore* score = v->score()->masterScore();
            QList<Excerpt*>& excerpts = score->excerpts();
            if (!excerpts.isEmpty()) {
                  TabScoreView* tsv = tabScoreView(n);
                  IF_ASSERT_FAILED(tsv) {
                        return;
                        }
                  tab2->blockSignals(true);
                  tab2->addTab(score->fileInfo()->completeBaseName().replace("&","&&") + (score->dirty() ? "*" : ""));
                  for (const Excerpt* excerpt : excerpts)
                        tab2->addTab(excerpt->title().replace("&","&&"));
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
      TabScoreView* tsv = tabScoreView(idx);
      IF_ASSERT_FAILED(tsv) {
            return;
            }

      MasterScore* score = tsv->score;
      if (!score->excerptsChanged())
            return;

      clearTab2();
      //delete all scoreviews for parts, especially for the deleted ones
      int n = stack->count() - 1;
      for (int i = n; i >= 0; --i) {
            QSplitter* vs = static_cast<QSplitter*>(stack->widget(i));
            ScoreView* sview = static_cast<ScoreView*>(vs->widget(0));

            if (sview->score() != score && sview->score()->masterScore() == score) {
                  stack->takeAt(i);
                  sview->deleteLater();
                  }
            }

      QList<Excerpt*>& excerpts = score->excerpts();
      if (!excerpts.isEmpty()) {
            tab2->blockSignals(true);
            tab2->addTab(score->fileInfo()->completeBaseName().replace("&","&&") + (score->dirty() ? "*" : ""));
            for (const Excerpt* excerpt : excerpts)
                  tab2->addTab(excerpt->title().replace("&","&&"));
            tab2->blockSignals(false);
            tab2->setVisible(true);
            }
      else {
            tab2->setVisible(false);
            }
      setExcerpt(0);
      }

//---------------------------------------------------------
//   setExcerpt
//    Sets the currently selected excerpt tab to the n-th
//    excerpt tab (where 0 is a tab for a master score).
//    For selecting the current score see
//    ScoreTab::setCurrentIndex
//---------------------------------------------------------

void ScoreTab::setExcerpt(int n)
      {
      if (n == -1)
            return;

      IF_ASSERT_FAILED(tab) {
            return;
            }

      int idx           = tab->currentIndex();
      TabScoreView* tsv = tabScoreView(idx);
      IF_ASSERT_FAILED(tsv) {
            return;
            }

      tsv->part     = n;
      QSplitter* vs = viewSplitter(idx);
      ScoreView* v;
      Score* score = tsv->score;
      if (n) {
            QList<Excerpt*>& excerpts = score->excerpts();
            if (!excerpts.isEmpty()) {
                  score = excerpts.at(n - 1)->partScore();
                  }
            }
      if (!vs) {
            vs = new QSplitter;
            vs->setObjectName("part");
            vs->setAccessibleName("");
            v  = new ScoreView;
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

void ScoreTab::insertTab(MasterScore* s)
      {
      int idx = scoreList->indexOf(s);
      tab->blockSignals(true);
      tab->insertTab(idx, s->fileInfo()->completeBaseName().replace("&","&&") + (s->dirty() ? "*" : ""));
      tab->setTabData(idx, QVariant::fromValue<void*>(new TabScoreView(s)));
      tab->blockSignals(false);
      emit tabInserted(idx);
      }

//---------------------------------------------------------
//   setTabText
//---------------------------------------------------------

void ScoreTab::setTabText(int idx, const QString& s)
      {
      QString text(s);
      text.replace("&","&&");
      tab->setTabText(idx, text);
      if (tab2 && currentIndex() == idx)
            tab2->setTabText(0, text);
      emit tabRenamed(idx);
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
//    Sets the currently selected score tab to the tab
//    number idx.
//    For selecting the current excerpt see
//    ScoreTab::setExcerpt.
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
//   setCurrentScore
//    Changes the currently selected score tab and excerpt
//    tab to display the given score.
//    Returns true on success.
//---------------------------------------------------------

bool ScoreTab::setCurrentScore(Score* s)
      {
      MasterScore* ms = s->masterScore();
      const int idx = scoreList->indexOf(ms);
      if (idx == -1)
            return false;
      const int exIdx = (ms == s) ? 0 : ms->scoreList().indexOf(s);
      if (exIdx == -1)
            return false;

      setCurrentIndex(idx);
      setExcerpt(exIdx);
      return true;
      }

//---------------------------------------------------------
//   removeTab
//---------------------------------------------------------

void ScoreTab::removeTab(int idx, bool noCurrentChangedSignal)
      {
      TabScoreView* tsv = tabScoreView(idx);
      IF_ASSERT_FAILED(tsv) {
            return;
            }
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
            Score* sc = excerpt->partScore();
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

      bool blocked = false;
      if (noCurrentChangedSignal)
            blocked = blockSignals(true);

      tab->removeTab(idx); // Will call setCurrent via a signal-slot connection

      if (noCurrentChangedSignal)
            blockSignals(blocked);

      emit tabRemoved(idx);
      }

//---------------------------------------------------------
//   initScoreView
//---------------------------------------------------------

void ScoreTab::initScoreView(const int idx, const qreal logicalZoomLevel, const ZoomIndex zoomIndex, const qreal xoffset, const qreal yoffset)
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
            vs->setObjectName("score");
            vs->setAccessibleName("");
            vs->addWidget(v);
            stack->addWidget(vs);
            }
      v->setLogicalZoom(zoomIndex, logicalZoomLevel);
      v->setOffset(xoffset, yoffset);
      }
}

