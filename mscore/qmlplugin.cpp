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

#include "qmlplugin.h"
// #include "plugins.h"
#include "shortcut.h"
#include "musescoreCore.h"
#include "libmscore/score.h"

namespace Ms {

extern MuseScoreCore* mscoreCore;

//---------------------------------------------------------
//   QmlPlugin
//---------------------------------------------------------

QmlPlugin::QmlPlugin(QQuickItem* parent)
   : QQuickItem(parent)
      {
      msc = mscoreCore;
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

QQmlListProperty<Score> QmlPlugin::scores()
      {
      return QQmlListProperty<Score>(this, msc->scores());
      }

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
//---------------------------------------------------------

Score* QmlPlugin::readScore(const QString& name)
      {
      return msc->openScore(name);
      }

//---------------------------------------------------------
//   newElement
//---------------------------------------------------------

Ms::Element* QmlPlugin::newElement(int t)
      {
      Score* score = curScore();
      if (score == 0)
            return 0;
      return Element::create(Element::ElementType(t), score);
      }

//---------------------------------------------------------
//   newScore
//---------------------------------------------------------

Score* QmlPlugin::newScore(const QString& name, const QString& part, int measures)
      {
      if (msc->currentScore()) {
            msc->currentScore()->endCmd();
            msc->endCmd();
            }
      Score* score = new Score(MScore::defaultStyle());
      score->setName(name);
      score->appendPart(part);
      score->appendMeasures(measures);
      score->doLayout();
      int view = msc->appendScore(score);
      msc->setCurrentView(0, view);
      qApp->processEvents();
      score->startCmd();
      return score;
      }

//---------------------------------------------------------
//   cmd
//---------------------------------------------------------

void QmlPlugin::cmd(const QString& s)
      {
      Shortcut* sc = Shortcut::getShortcut(s.toLatin1().data());
      if (sc)
            msc->cmd(sc->action());
      else
            printf("QmlPlugin:cmd: not found <%s>\n", qPrintable(s));
      }

//---------------------------------------------------------
//   newQProcess
//---------------------------------------------------------

MsProcess* QmlPlugin::newQProcess()
      {
      return 0; // TODO: new MsProcess(this);
      }
}

