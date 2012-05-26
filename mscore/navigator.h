//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: navigator.h 4785 2011-09-14 10:06:35Z wschweer $
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#ifndef __NAVIGATOR_H__
#define __NAVIGATOR_H__

class Score;
class ScoreView;
class Page;
class Navigator;

//---------------------------------------------------------
//   NScrollArea
//    modified QScrollArea for Navigator
//---------------------------------------------------------

class NScrollArea : public QScrollArea {
      Q_OBJECT

      virtual void resizeEvent(QResizeEvent*);

   public:
      NScrollArea(QWidget* w = 0);
      };

//---------------------------------------------------------
//   PageCache
//---------------------------------------------------------

struct PageCache {
      bool valid;
      Page* page;
      QImage pm;
      QTransform matrix;
      Navigator* navigator;
      };

//---------------------------------------------------------
//   Navigator
//---------------------------------------------------------

class Navigator : public QWidget {
      Q_OBJECT

      Score* _score;
      NScrollArea* scrollArea;
      QPointer<ScoreView> _cv;

      QRect viewRect;
      QPoint startMove;
      QList<PageCache> pcl;
      QList<PageCache*> npcl;
      QTransform matrix;

      QFuture<void> updatePixmap;
      QFutureWatcher<void> watcher;
      bool recreatePixmap;

      int cachedWidth;

      void rescale();

      virtual void paintEvent(QPaintEvent*);
      virtual void mousePressEvent(QMouseEvent*);
      virtual void mouseMoveEvent(QMouseEvent*);
      virtual void resizeEvent(QResizeEvent*);

   private slots:
      void pmFinished();

   public slots:
      void updateViewRect();
      void layoutChanged();

   signals:
      void viewRectMoved(const QRectF&);

   public:
      Navigator(NScrollArea* sa, QWidget* parent = 0);
      void setScoreView(ScoreView*);
      void setScore(Score*);
      Score* score() const { return _score; }
      void setViewRect(const QRectF& r);
      };

#endif

