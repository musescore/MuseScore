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

namespace Ms {

class MuseScore;
class ScoreView;
class Score;
enum class ZoomIndex : char;
struct ScoreViewState;

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
//   MsTabBar
//---------------------------------------------------------

class MsTabBar : public QTabBar {
      int _middleClickedTab { -1 };

   public:
      MsTabBar(QWidget* parent = nullptr) : QTabBar(parent) {}

   private:
      void mousePressEvent(QMouseEvent* e) override;
      void mouseReleaseEvent(QMouseEvent* e) override;
      };

//---------------------------------------------------------
//   ScoreTab
//---------------------------------------------------------

class ScoreTab : public QWidget {
      Q_OBJECT
      QList<MasterScore*>* scoreList { nullptr };
      MsTabBar* tab  { nullptr };                 // list of scores
      MsTabBar* tab2 { nullptr };                 // list of excerpts for current score
      QStackedLayout* stack { nullptr };
      MuseScore* mainWindow { nullptr };;
      void clearTab2();
      TabScoreView* tabScoreView(int idx);
      const TabScoreView* tabScoreView(int idx) const;
      std::map<int, ScoreViewState> scoreViewsToInit;

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

      MsTabBar* getTab() const { return tab; }

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
      void initScoreView(int idx);
      void queueInitScoreView(int idx, qreal logicalZoomLevel, ZoomIndex zoomIndex, qreal xoffset, qreal yoffset);
      };


} // namespace Ms
#endif

