//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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

#include "taskbarprogress.h"

#ifdef Q_OS_WIN
#include <QWinTaskbarProgress>
#endif

using namespace Ms;

TaskbarProgress::TaskbarProgress(QObject* parent)
    : QObject(parent)
{
#ifdef Q_OS_WIN
    _winTaskbarProgress = new QWinTaskbarProgress();
#endif
}

TaskbarProgress::~TaskbarProgress()
{
#ifdef Q_OS_WIN
    delete _winTaskbarProgress;
#endif
}

//---------------------------------------------------------
//   isAvalabled
//---------------------------------------------------------

bool TaskbarProgress::isAvalabled() const
{
#ifdef Q_OS_WIN
    return true;
#else
    return false;
#endif
}

//---------------------------------------------------------
//   show
//---------------------------------------------------------

void TaskbarProgress::show()
{
#ifdef Q_OS_WIN
    _winTaskbarProgress->show();
#else
    // noop
#endif
}

//---------------------------------------------------------
//   hide
//---------------------------------------------------------

void TaskbarProgress::hide()
{
#ifdef Q_OS_WIN
    _winTaskbarProgress->hide();
#else
    // noop
#endif
}

//---------------------------------------------------------
//   stop
//---------------------------------------------------------

void TaskbarProgress::stop()
{
#ifdef Q_OS_WIN
    _winTaskbarProgress->stop();
#else
    // noop
#endif
}

//---------------------------------------------------------
//   setRange
//---------------------------------------------------------

void TaskbarProgress::setRange(int min, int max)
{
#ifdef Q_OS_WIN
    _winTaskbarProgress->setRange(min, max);
#else
    Q_UNUSED(min);
    Q_UNUSED(max);
    // noop
#endif
}

//---------------------------------------------------------
//   setValue
//---------------------------------------------------------

void TaskbarProgress::setValue(int val)
{
#ifdef Q_OS_WIN
    _winTaskbarProgress->setValue(val);
#else
    Q_UNUSED(val);
    // noop
#endif
}
