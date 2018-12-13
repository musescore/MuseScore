//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "config.h"
#ifdef SCRIPT_INTERFACE

#include "qmlplugin.h"
#include "shortcut.h"
#include "libmscore/musescoreCore.h"
#include "libmscore/score.h"

#include <QQmlEngine>

namespace Ms {

//---------------------------------------------------------
//   QmlPlugin
//---------------------------------------------------------

QmlPlugin::QmlPlugin(QQuickItem* parent)
   : QQuickItem(parent)
      {
      msc = MuseScoreCore::mscoreCore;
      _requiresScore = true;              // by default plugins require a score to work
      }

QmlPlugin::~QmlPlugin()
      {
      }

//---------------------------------------------------------
//   curScore
//---------------------------------------------------------

Score* QmlPlugin::curScore() const
      {
      return msc->currentScore();
      }

//---------------------------------------------------------
//   scores
//---------------------------------------------------------

#if 0 // TODO-ws
QQmlListProperty<Score> QmlPlugin::scores()
      {
      return QQmlListProperty<Score>(this, msc->scores());
      }
#endif

//---------------------------------------------------------
//   writeScore
//---------------------------------------------------------

bool QmlPlugin::writeScore(Score* s, const QString& name, const QString& ext)
      {
      if(!s)
            return false;
      return msc->saveAs(s, true, name, ext);
      }

//---------------------------------------------------------
//   readScore
//
// noninteractive can be used to avoid a 'save changes'
// dialog on closing a score that is either imported
// or was created with an older version of MuseScore
//---------------------------------------------------------

Score* QmlPlugin::readScore(const QString& name, bool noninteractive)
      {
      Score * score = msc->openScore(name, true);
      // tell QML not to garbage collect this score
      if (score) {
            QQmlEngine::setObjectOwnership(score, QQmlEngine::CppOwnership);
            if (noninteractive)
	          score->setCreated(false);
            }
      return score;
      }

//---------------------------------------------------------
//   closeScore
//---------------------------------------------------------

void QmlPlugin::closeScore(Ms::Score* score)
      {
      msc->closeScore(score);
      }

//---------------------------------------------------------
//   newElement
//---------------------------------------------------------

Ms::Element* QmlPlugin::newElement(int t)
      {
      Score* score = curScore();
      if (score == 0)
            return 0;
      Element* e = Element::create(ElementType(t), score);
      // tell QML not to garbage collect this score
//TODO      Ms::MScore::qml()->setObjectOwnership(e, QQmlEngine::CppOwnership);
      return e;
      }

//---------------------------------------------------------
//   newScore
//---------------------------------------------------------

Score* QmlPlugin::newScore(const QString& /*name*/, const QString& /*part*/, int /*measures*/)
      {
#if 0 // TODO
      if (msc->currentScore())
            msc->currentScore()->endCmd();
      MasterScore* score = new MasterScore(MScore::defaultStyle());
      score->setName(name);
      score->appendPart(part);
      score->appendMeasures(measures);
      score->doLayout();
      int view = msc->appendScore(score);
      msc->setCurrentView(0, view);
      qApp->processEvents();
      // tell QML not to garbage collect this score
      QQmlEngine::setObjectOwnership(score, QQmlEngine::CppOwnership);
      score->startCmd();
      return score;
#endif
      return 0;
      }

//---------------------------------------------------------
//   cmd
//---------------------------------------------------------

void QmlPlugin::cmd(const QString& s)
      {
      Shortcut* sc = Shortcut::getShortcut(qPrintable(s));
      if (sc)
            msc->cmd(sc->action());
      else
            qDebug("QmlPlugin:cmd: not found <%s>", qPrintable(s));
      }

//---------------------------------------------------------
//   openLog
//---------------------------------------------------------

void QmlPlugin::openLog(const QString& name)
      {
      if (logFile.isOpen())
            logFile.close();
      logFile.setFileName(name);
      if (!logFile.open(QIODevice::WriteOnly))
            qDebug("QmlPlugin::openLog: failed");
      }

//---------------------------------------------------------
//   closeLog
//---------------------------------------------------------

void QmlPlugin::closeLog()
      {
      if (logFile.isOpen())
            logFile.close();
      }

//---------------------------------------------------------
//   log
//---------------------------------------------------------

void QmlPlugin::log(const QString& txt)
      {
      if (logFile.isOpen())
            logFile.write(txt.toLocal8Bit());
      }

//---------------------------------------------------------
//   logn
//---------------------------------------------------------

void QmlPlugin::logn(const QString& txt)
      {
      log(txt);
      if (logFile.isOpen())
            logFile.write("\n");
      }

//---------------------------------------------------------
//   log2
//---------------------------------------------------------

void QmlPlugin::log2(const QString& txt, const QString& txt2)
      {
      logFile.write(txt.toLocal8Bit());
      logFile.write(txt2.toLocal8Bit());
      logFile.write("\n");
      }

//---------------------------------------------------------
//   newQProcess
//---------------------------------------------------------

MsProcess* QmlPlugin::newQProcess()
      {
      return 0; // TODO: new MsProcess(this);
      }

}
#endif

