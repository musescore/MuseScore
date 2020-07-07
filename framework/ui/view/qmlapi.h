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
#ifndef MU_FRAMEWORK_QMLAPI_H
#define MU_FRAMEWORK_QMLAPI_H

#include <QObject>

#include "qmltheme.h"
#include "qmllauncher.h"

namespace mu {
namespace framework {
class QmlApi : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QmlLauncher * launcher READ launcher CONSTANT)

public:
    explicit QmlApi(QObject* parent = nullptr);

    QmlLauncher* launcher() const;

private:

    QmlLauncher* m_launcher = nullptr;
};
}
}

#endif // MU_FRAMEWORK_QMLAPI_H
