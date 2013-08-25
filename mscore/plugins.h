//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __PLUGINS_H__
#define __PLUGINS_H__

#include "config.h"
#ifdef SCRIPT_INTERFACE
#include "libmscore/element.h"
#include "libmscore/score.h"
#include "libmscore/utils.h"

namespace Ms {

//---------------------------------------------------------
//   @@ FileIO
//   @P source QString
//---------------------------------------------------------

class FileIO : public QObject {
      Q_OBJECT

   public:
      Q_PROPERTY(QString source
               READ source
               WRITE setSource
               NOTIFY sourceChanged)
      explicit FileIO(QObject *parent = 0);

      Q_INVOKABLE QString read();
      Q_INVOKABLE bool write(const QString& data);
      Q_INVOKABLE bool remove();
      Q_INVOKABLE QString tempPath() {QDir dir; return dir.tempPath();};

      QString source() { return mSource; };

   public slots:
      void setSource(const QString& source) { mSource = source; };

   signals:
      void sourceChanged(const QString& source);
      void error(const QString& msg);

   private:
      QString mSource;
      };

//---------------------------------------------------------
//   MsProcess
//    @@ QProcess
//---------------------------------------------------------

class MsProcess : public QProcess {
      Q_OBJECT

   public:
      MsProcess(QObject* parent = 0) : QProcess(parent) {}

   public slots:
      Q_INVOKABLE void start(const QString& program)      { QProcess::start(program); }
      Q_INVOKABLE bool waitForFinished(int msecs = 30000) { return QProcess::waitForFinished(msecs); }
      Q_INVOKABLE QByteArray readAllStandardOutput()      { return QProcess::readAllStandardOutput(); }
      };

//---------------------------------------------------------
//   @@ ScoreView
///   This is an GUI element to show a score.
//
//   @P color QColor    background color
//   @P scale qreal     scaling factor
//---------------------------------------------------------

class MsScoreView : public QQuickPaintedItem, public MuseScoreView {
      Q_OBJECT
      Q_PROPERTY(QColor color READ color WRITE setColor)
      Q_PROPERTY(qreal  scale READ scale WRITE setScale)

      Score* score;
      int _currentPage;
      QColor _color;
      qreal mag;
      int playPos;
      QRectF _boundingRect;

      QNetworkAccessManager* networkManager;

      virtual void dataChanged(const QRectF&)   { update(); }
      virtual void updateAll()                  { update(); }
      virtual void moveCursor()                 {}
      virtual void updateLoopCursors()          {}
      virtual void showLoopCursors()            {}
      virtual void hideLoopCursors()            {}
      virtual void adjustCanvasPosition(const Element*, bool) {}
      virtual void removeScore()                {}
      virtual void changeEditElement(Element*)  {}
      virtual int gripCount() const             { return 0; }
      virtual const QRectF& getGrip(int) const;
      virtual const QTransform& matrix() const;
      virtual void setDropRectangle(const QRectF&) {}
      virtual void cmdAddSlur(Note*, Note*)     {}
      virtual void startEdit()                  {}
      virtual void startEdit(Element*, int)     {}
      virtual Element* elementNear(QPointF)     { return 0; }

      virtual void paint(QPainter*);

      virtual void setCursor(const QCursor&)    {}
      virtual QCursor cursor() const            { return QCursor(); }
      virtual QRectF boundingRect() const       { return _boundingRect; }
      virtual void drawBackground(QPainter*, const QRectF&) const {}

   public slots:
      Q_INVOKABLE void setScore(Score*);
      Q_INVOKABLE void setCurrentPage(int n);
      Q_INVOKABLE void nextPage();
      Q_INVOKABLE void prevPage();

   public:
      MsScoreView(QQuickItem* parent = 0);
      virtual ~MsScoreView() {}
      QColor color() const            { return _color;        }
      void setColor(const QColor& c)  { _color = c;           }
      qreal scale() const             { return mag;        }
      void setScale(qreal v)          { mag = v;           }
      };

class PluginDescription;
extern void collectPluginMetaInformation(PluginDescription*);

#endif

} // namespace Ms
#endif


