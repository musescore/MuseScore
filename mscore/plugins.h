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
#include "musescore.h"
#include "libmscore/score.h"
#include "libmscore/utils.h"

//---------------------------------------------------------
//   MsFile
//    @@ QFile
//---------------------------------------------------------

class MsFile : public QFile {
      Q_OBJECT

   public:
      MsFile() : QFile() {}
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
      void start(const QString& program)      { QProcess::start(program); }
      bool waitForFinished(int msecs = 30000) { return QProcess::waitForFinished(msecs); }
      QByteArray readAllStandardOutput()      { return QProcess::readAllStandardOutput(); }
      };

//---------------------------------------------------------
//   @@ ScoreView
//   @P color QColor    background color
//   @P scale qreal     scaling factor
//---------------------------------------------------------

class MsScoreView : public QDeclarativeItem, public MuseScoreView {
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

      virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*);

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
      MsScoreView(QDeclarativeItem* parent = 0);
      virtual ~MsScoreView() {}
      QColor color() const            { return _color;        }
      void setColor(const QColor& c)  { _color = c;           }
      qreal scale() const             { return mag;        }
      void setScale(qreal v)          { mag = v;           }
      };

//---------------------------------------------------------
//   QmlPlugin
//   @@ MuseScore
//   @P menuPath              QString
//   @P pluginType            QString
//   @P dockArea              QString
//   @P division              int
//   @P mscoreVersion         int
//   @P mscoreMajorVersion    int
//   @P mscoreMinorVersion    int
//   @P mscoreUpdateVersion   int
//   @P mscoreDPI             qreal
//   @P curScore              Score*
//   @P scores                array[Score]
//---------------------------------------------------------

class QmlPlugin : public QDeclarativeItem {
      Q_OBJECT
      Q_PROPERTY(QString menuPath        READ menuPath WRITE setMenuPath)
      Q_PROPERTY(QString pluginType      READ pluginType WRITE setPluginType)
      Q_PROPERTY(QString dockArea        READ dockArea WRITE setDockArea)
      Q_PROPERTY(int division            READ division)
      Q_PROPERTY(int mscoreVersion       READ mscoreVersion)
      Q_PROPERTY(int mscoreMajorVersion  READ mscoreMajorVersion)
      Q_PROPERTY(int mscoreMinorVersion  READ mscoreMinorVersion)
      Q_PROPERTY(int mscoreUpdateVersion READ mscoreUpdateVersion)
      Q_PROPERTY(qreal mscoreDPI         READ mscoreDPI)
      Q_PROPERTY(Score* curScore         READ curScore)
      Q_PROPERTY(QDeclarativeListProperty<Score> scores READ scores);

      QString _menuPath;
      QString _pluginType;
      QString _dockArea;

   signals:
      void run();

   public:
      QmlPlugin(QDeclarativeItem* parent = 0);
      ~QmlPlugin();

      void setMenuPath(const QString& s)   { _menuPath = s;    }
      QString menuPath() const             { return _menuPath; }
      void setPluginType(const QString& s) { _pluginType = s;    }
      QString pluginType() const           { return _pluginType; }
      void setDockArea(const QString& s)   { _dockArea = s;    }
      QString dockArea() const             { return _dockArea; }
      void runPlugin()                     { emit run();       }

      int division() const                { return MScore::division; }
      int mscoreVersion() const           { return version();       }
      int mscoreMajorVersion() const      { return majorVersion();  }
      int mscoreMinorVersion() const      { return minorVersion();  }
      int mscoreUpdateVersion() const     { return updateVersion(); }
      qreal mscoreDPI() const             { return MScore::DPI;     }

      Score* curScore() const;
      QDeclarativeListProperty<Score> scores();

      Q_INVOKABLE Score* newScore(const QString& name, const QString& part, int measures);
      Q_INVOKABLE Element* newElement(int);
      Q_INVOKABLE void cmd(const QString&);
      Q_INVOKABLE MsProcess* newQProcess() { return new MsProcess(this); }
      Q_INVOKABLE MsFile* newQFile()       { return new MsFile(); }
      Q_INVOKABLE bool writeScore(Score*, const QString& name, const QString& ext);
      Q_INVOKABLE Score* readScore(const QString& name);
      };
#endif
#endif

