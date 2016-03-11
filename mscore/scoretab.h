//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
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
      Score* score;
      int part;
      TabScoreView(Score* s) {
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

   signals:
      void currentScoreViewChanged(ScoreView*);
      void tabCloseRequested(int);
      void actionTriggered(QAction*);

   public slots:
      void updateExcerpts();
      void setExcerpt(int);
      void setCurrent(int);

   public:
      ScoreTab(QList<MasterScore*>*, QWidget* parent = 0);
      ~ScoreTab();

      void insertTab(Score*);
      void setTabText(int, const QString&);
      int currentIndex() const;
      void setCurrentIndex(int);
      void removeTab(int);
      int count() const       { return scoreList->size(); }
      ScoreView* view(int) const;
      QSplitter* viewSplitter(int n) const;
      ScoreView* view() const { return view(currentIndex()); }
      bool contains(ScoreView*) const;
      void initScoreView(int idx, double mag, MagIdx magIdx, double xoffset, double yoffset);
      };


} // namespace Ms
#endif

