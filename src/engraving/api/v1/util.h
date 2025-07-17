/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __PLUGIN_API_UTIL_H__
#define __PLUGIN_API_UTIL_H__

#include <QDir>
#include <QProcess>
#include <QNetworkAccessManager>

#include "uicomponents/view/quickpaintedview.h"

#include "engraving/dom/mscoreview.h"

namespace mu::engraving {
class Score;
}

namespace mu::plugins::api {
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

class FileIO : public QObject
{
    Q_OBJECT

public:
    /// Path to the file which is operated on
    Q_PROPERTY(QString source
               READ source
               WRITE setSource
               )
    /// \cond PLUGIN_API \private \endcond
    explicit FileIO(QObject* parent = 0);

    /// Reads file contents and returns a string.
    /// In case error occurs, error() signal is emitted
    /// and an empty string is returned.
    Q_INVOKABLE QString read();
    /// muse::Returns true if the file exists
    Q_INVOKABLE bool exists();
    /// Writes a string to the file.
    /// \warning This function overwrites all the contents of
    /// the file pointed by FileIO::source so it becomes lost.
    /// \returns `true` if an operation finished successfully.
    Q_INVOKABLE bool write(const QString& data);
    /// Removes the file
    Q_INVOKABLE bool remove();
    /// muse::Returns user's home directory
    Q_INVOKABLE QString homePath() { QDir dir; return dir.homePath(); }
    /// muse::Returns a path suitable for a temporary file
    Q_INVOKABLE QString tempPath() { QDir dir; return dir.tempPath(); }
    /// muse::Returns the file's last modification time
    Q_INVOKABLE int modifiedTime();

    /// \cond MS_INTERNAL
    QString source() { return m_source; }

public slots:
    void setSource(const QString& source) { m_source = source; }
    /// \endcond

signals:
    /// Emitted on file operations errors.
    /// Implement onError() in your FileIO object to handle this signal.
    /// \param msg A short textual description of the error occurred.
    void error(const QString& msg);

private:
    QString m_source;
};

//---------------------------------------------------------
//   MsProcess
//   @@ QProcess
///   \brief Start an external program.\ Available in QML
///   as \p QProcess.
///   \details Using this will most probably result in the
///   plugin to be platform dependant. \since MuseScore 3.2
//---------------------------------------------------------

class MsProcess : public QProcess
{
    Q_OBJECT

public:
    MsProcess(QObject* parent = 0)
        : QProcess(parent) {}

public slots:
    /// Execute an external command.
    /// \param command A command line to execute.
    /// \warning This function is deprecated. Use \ref startWithArgs instead.
    Q_INVOKABLE void start(const QString& command);
    /// Execute an external command.
    /// \param program A program to execute.
    /// \param args An array of arguments passed to the program.
    /// \since MuseScore 4.3
    Q_INVOKABLE void startWithArgs(const QString& program, const QStringList& args);
    //@ --
    Q_INVOKABLE bool waitForFinished(int msecs = 30000) { return QProcess::waitForFinished(msecs); }
    //@ --
    Q_INVOKABLE QByteArray readAllStandardOutput() { return QProcess::readAllStandardOutput(); }
};

//---------------------------------------------------------
//   @@ ScoreView
///    This is an GUI element to show a score. \since MuseScore 3.2
//---------------------------------------------------------

class ScoreView : public uicomponents::QuickPaintedView, public engraving::MuseScoreView
{
    Q_OBJECT
    /// Background color
    Q_PROPERTY(QColor color READ color WRITE setColor)
    /// Scaling factor
    Q_PROPERTY(qreal scale READ scale WRITE setScale)

    mu::engraving::Score* score;
    int m_currentPage;
    QColor m_color;
    qreal mag;
    int playPos;
    QRectF m_boundingRect;

    QNetworkAccessManager* networkManager;

    virtual void setScore(mu::engraving::Score*) override;

    virtual void dataChanged(const RectF&) override { update(); }
    virtual void updateAll() override { update(); }

    virtual void paint(QPainter*) override;

    virtual QRectF boundingRect() const override { return m_boundingRect; }
    virtual void drawBackground(muse::draw::Painter*, const RectF&) const override {}

public slots:
    //@ --
    Q_INVOKABLE void setScore(mu::plugins::api::Score*);
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
    QColor color() const { return m_color; }
    void setColor(const QColor& c) { m_color = c; }
    qreal scale() const { return mag; }
    void setScale(qreal v) { mag = v; }
    virtual const muse::Rect geometry() const override { return muse::Rect(x(), y(), width(), height()); }
    /// \endcond
};
} // namespace mu::plugins::api

#endif
