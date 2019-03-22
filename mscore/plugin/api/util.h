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

#ifndef __PLUGIN_API_UTIL_H__
#define __PLUGIN_API_UTIL_H__

#include "config.h"

#include "libmscore/element.h"
#include "libmscore/mscoreview.h"
#include "libmscore/score.h"
#include "libmscore/utils.h"

namespace Ms {
namespace PluginAPI {

class Score;

//---------------------------------------------------------
//   @@ FileIO
//   @P source string
//---------------------------------------------------------

class FileIO : public QObject {
      Q_OBJECT

   public:
      Q_PROPERTY(QString source
               READ source
               WRITE setSource
               NOTIFY sourceChanged)
      explicit FileIO(QObject *parent = 0);

      //@ reads file contents and returns a string
      Q_INVOKABLE QString read();
      //@ return true if the file exists
      Q_INVOKABLE bool exists();
      //@ write a string to the file
      Q_INVOKABLE bool write(const QString& data);
      //@ removes the file
      Q_INVOKABLE bool remove();
      //@ returns user's home directory
      Q_INVOKABLE QString homePath() {QDir dir; return dir.homePath();}
      //@ returns a path suitable for a temporary file
      Q_INVOKABLE QString tempPath() {QDir dir; return dir.tempPath();}
      //@ returns the file modification time
      Q_INVOKABLE int modifiedTime();

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
//   @@ QProcess
//---------------------------------------------------------

class MsProcess : public QProcess {
      Q_OBJECT

   public:
      MsProcess(QObject* parent = 0) : QProcess(parent) {}

   public slots:
      //@ --
      Q_INVOKABLE void start(const QString& program)      { QProcess::start(program); }
      //@ --
      Q_INVOKABLE bool waitForFinished(int msecs = 30000) { return QProcess::waitForFinished(msecs); }
      //@ --
      Q_INVOKABLE QByteArray readAllStandardOutput()      { return QProcess::readAllStandardOutput(); }
      };

//---------------------------------------------------------
//   @@ ScoreView
///    This is an GUI element to show a score.
//---------------------------------------------------------

class MsScoreView : public QQuickPaintedItem, public MuseScoreView {
      Q_OBJECT
      /** Background color */
      Q_PROPERTY(QColor color READ color WRITE setColor)
      /** Scaling factor */
      Q_PROPERTY(qreal  scale READ scale WRITE setScale)

      Ms::Score* score;
      int _currentPage;
      QColor _color;
      qreal mag;
      int playPos;
      QRectF _boundingRect;

      QNetworkAccessManager* networkManager;

      virtual void dataChanged(const QRectF&) override { update(); }
      virtual void updateAll() override                { update(); }

      virtual void paint(QPainter*) override;

      virtual QRectF boundingRect() const override { return _boundingRect; }
      virtual void drawBackground(QPainter*, const QRectF&) const override {}

   public slots:
      //@ --
      Q_INVOKABLE void setScore(Ms::PluginAPI::Score*);
      //@ --
      Q_INVOKABLE void setCurrentPage(int n);
      //@ --
      Q_INVOKABLE void nextPage();
      //@ --
      Q_INVOKABLE void prevPage();

   public:
      MsScoreView(QQuickItem* parent = 0);
      virtual ~MsScoreView() {}
      QColor color() const            { return _color;        }
      void setColor(const QColor& c)  { _color = c;           }
      qreal scale() const             { return mag;        }
      void setScale(qreal v)          { mag = v;           }
      virtual const QRect geometry() const override { return QRect(QQuickPaintedItem::x(), y(), width(), height()); }
      };
} // namespace PluginAPI
} // namespace Ms
#endif
