//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2009 Werner Schweer and others
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

#ifndef __SCORETAB_H__
#define __SCORETAB_H__
#include "musescore.h"
namespace Ms {

class ScoreView;
class Score;
enum class MagIdx : char;

//---------------------------------------------------------
//   TabScoreView
//---------------------------------------------------------

struct TabScoreView {
      MasterScore* score;
      int part;
      TabScoreView(MasterScore* s) {
            score   = s;
            part    = 0;
            }
      };

//---------------------------------------------------------
//   ScoreTab
//---------------------------------------------------------

class ScoreTab : public QWidget {
      Q_OBJECT
      QList<MasterScore*>* scoreList;
      QTabBar* tab;                 // list of scores
      QTabBar* tab2;                // list of excerpts for current score
      QStackedLayout* stack;
      MuseScore* mainWindow;
      void clearTab2();
      TabScoreView* tabScoreView(int idx) { return static_cast<TabScoreView*>(tab->tabData(idx).value<void*>()); }
      const TabScoreView* tabScoreView(int idx) const { return const_cast<ScoreTab*>(this)->tabScoreView(idx); }

   signals:
      void currentScoreViewChanged(ScoreView*);
      void tabCloseRequested(int);
      void actionTriggered(QAction*);
      void tabInserted(int);
      void tabRemoved(int);
      void tabRenamed(int);

   private slots:
      void setCurrent(int);

   public slots:
      void updateExcerpts();
      void setExcerpt(int);
      void tabMoved(int, int);

   public:
      ScoreTab(QList<MasterScore*>*, QWidget* parent = 0);
      ~ScoreTab();

      QTabBar* getTab() const { return tab; }

      void insertTab(MasterScore*);
      void setTabText(int, const QString&);
      int currentIndex() const;
      void setCurrentIndex(int);
      bool setCurrentScore(Score* s);
      void removeTab(int, bool noCurrentChangedSignal = false);
      int count() const       { return scoreList->size(); }
      ScoreView* view(int) const;
      QSplitter* viewSplitter(int n) const;
      ScoreView* view() const { return view(currentIndex()); }
      bool contains(ScoreView*) const;
      void initScoreView(int idx, double mag, MagIdx magIdx, double xoffset, double yoffset);
      };


} // namespace Ms
#endif

