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

#ifndef AVS_LOG_H
#define AVS_LOG_H

#include <QDebug>

#define LOGD() qDebug()
#define LOGI() qInfo()
#define LOGW() qWarning()
#define LOGE() qCritical()

#define IF_ASSERT(cond) if (!(cond)) { \
        LOGE() << "\"ASSERT FAILED!\":" << #cond << __FILE__ << __LINE__; \
        Q_ASSERT(cond); \
} \
    if (!(cond)) \

#define IF_FAILED(cond) if (!(cond)) { \
        LOGE() << "\"FAILED!\":" << #cond << __FILE__ << __LINE__; \
} \
    if (!(cond)) \


#endif // AVS_LOG_H
