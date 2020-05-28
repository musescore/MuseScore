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

#ifndef MS_LOG_H
#define MS_LOG_H

#include <QDebug>
#include <QElapsedTimer>

#define LOGD() qDebug()
#define LOGI() qInfo()
#define LOGW() qWarning()
#define LOGE() qCritical()

#define IF_ASSERT_FAILED(cond) if (!(cond)) { \
    LOGE() << "\"ASSERT FAILED!\":" << #cond << __FILE__ << __LINE__; \
    Q_ASSERT(cond); \
} \
if (!(cond)) \

#define IF_FAILED(cond) if (!(cond)) { \
    LOGE() << "\"FAILED!\":" << #cond << __FILE__ << __LINE__; \
} \
if (!(cond)) \


#define TIMER_START \
    QElapsedTimer __timer; \
    int __lastElapsed = 0; \
    __timer.start(); \
    auto __getElapsed = [&__timer, &__lastElapsed]() { \
        int delta = __timer.elapsed() - __lastElapsed; \
        __lastElapsed = __timer.elapsed(); \
        return QString("elapsed: %1, delta: %2").arg(__lastElapsed).arg(delta); \
    }; \

#define PRINT_TIMER_ELAPSED(info) LOGI() << info << " elapsed: " << __getElapsed();


#endif // MS_LOG_H
