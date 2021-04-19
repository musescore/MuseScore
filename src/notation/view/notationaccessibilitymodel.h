/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#ifndef MU_NOTATION_NOTATIONACCESSIBILITYMODEL_H
#define MU_NOTATION_NOTATIONACCESSIBILITYMODEL_H

#include <QtQml>

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "async/asyncable.h"

namespace mu {
namespace notation {
class NotationAccessibilityModel : public QObject, public async::Asyncable
{
    Q_OBJECT

    INJECT(notation, context::IGlobalContext, globalContext)

    Q_PROPERTY(QString accessibilityInfo READ accessibilityInfo NOTIFY accessibilityInfoChanged)

public:
    QString accessibilityInfo() const;

    Q_INVOKABLE void load();

signals:
    void accessibilityInfoChanged(const QString& info);

private:
    void setAccessibilityInfo(const std::string& info);

    QString m_accessibilityInfo;
};
}
}

#endif // MU_NOTATION_NOTATIONACCESSIBILITYMODEL_H
