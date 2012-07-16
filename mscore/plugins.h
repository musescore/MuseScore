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
      Q_INVOKABLE bool remove(const QString& data);
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
//   @P menuPath              QString        where the plugin is placed in menu
//   @P pluginType            QString
//   @P dockArea              QString
//   @P division              int            number of MIDI ticks for 1/4 note, read only
//   @P mscoreVersion         int            the complete version number of MuseScore in the form: MMmmuu, read only
//   @P mscoreMajorVersion    int            the 1st part of the MuseScore version, read only
//   @P mscoreMinorVersion    int            the 2nd part of the MuseScore version, read only
//   @P mscoreUpdateVersion   int            the 3rd part of the MuseScore version, read only
//   @P mscoreDPI             qreal          read only
//   @P curScore              Score*         the current score, if any, read only
//   @P scores                array[Score]   all currently open scores, read only
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
      Q_INVOKABLE bool writeScore(Score*, const QString& name, const QString& ext);
      Q_INVOKABLE Score* readScore(const QString& name);
      };
#endif
#endif

