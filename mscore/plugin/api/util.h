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
///   \class FileIO
///   Provides a simple API to perform file reading and
///   writing operations. To use this class in a plugin put
///   \code import FileIO 3.0 \endcode
///   to the top part of the .qml plugin file.
///   After that FileIO object can be declared and used
///   in QML code:
///   \code
///   import MuseScore 3.0
///   import FileIO 3.0
///   MuseScore {
///       FileIO {
///           id: exampleFile
///           source: "/tmp/example.txt"
///       }
///       onRun: {
///           var test = exampleFile.read();
///           console.log(test); // will print the file content
///       }
///   }
///   \endcode
//---------------------------------------------------------

class FileIO : public QObject {
      Q_OBJECT

   public:
      /** Path to the file which is operated on */
      Q_PROPERTY(QString source
               READ source
               WRITE setSource
               )
      /// \cond PLUGIN_API \private \endcond
      explicit FileIO(QObject *parent = 0);

      /**
       * Reads file contents and returns a string.
       * In case error occurs, error() signal is emitted
       * and an empty string is returned.
       */
      Q_INVOKABLE QString read();
      /** Returns true if the file exists */
      Q_INVOKABLE bool exists();
      /**
       * Writes a string to the file.
       * \warning This function overwrites all the contents of
       * the file pointed by FileIO::source so it becomes lost.
       * \returns `true` if an operation finished successfully.
       */
      Q_INVOKABLE bool write(const QString& data);
      /** Removes the file */
      Q_INVOKABLE bool remove();
      /** Returns user's home directory */
      Q_INVOKABLE QString homePath() {QDir dir; return dir.homePath();}
      /** Returns a path suitable for a temporary file */
      Q_INVOKABLE QString tempPath() {QDir dir; return dir.tempPath();}
      /** Returns the file's last modification time */
      Q_INVOKABLE int modifiedTime();

      /// \cond MS_INTERNAL
      QString source() { return mSource; }

   public slots:
      void setSource(const QString& source) { mSource = source; }
      /// \endcond

   signals:
      /**
       * Emitted on file operations errors.
       * Implement onError() in your FileIO object to handle this signal.
       * \param msg A short textual description of the error occurred.
       */
      void error(const QString& msg);

   private:
      QString mSource;
      };

//---------------------------------------------------------
//   MsProcess
//   @@ QProcess
///   \brief Start an external program.\ Available in QML
///   as \p QProcess.
///   \details Using this will most probably result in the
///   plugin to be platform dependant. \since MuseScore 3.2
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
///    This is an GUI element to show a score. \since MuseScore 3.2
//---------------------------------------------------------

class ScoreView : public QQuickPaintedItem, public MuseScoreView {
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

      virtual void setScore(Ms::Score*) override;

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
      /// \cond MS_INTERNAL
      ScoreView(QQuickItem* parent = 0);
      virtual ~ScoreView() {}
      QColor color() const            { return _color;        }
      void setColor(const QColor& c)  { _color = c;           }
      qreal scale() const             { return mag;        }
      void setScale(qreal v)          { mag = v;           }
      virtual const QRect geometry() const override { return QRect(QQuickPaintedItem::x(), y(), width(), height()); }
      /// \endcond
      };
} // namespace PluginAPI
} // namespace Ms
#endif
