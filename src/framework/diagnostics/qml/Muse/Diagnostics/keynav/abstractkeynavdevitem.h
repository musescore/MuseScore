/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#pragma once

#include <QObject>
#include <QVariant>
#include <qqmlintegration.h>

#include "ui/inavigation.h"
#include "async/asyncable.h"

namespace muse::diagnostics {
class AbstractKeyNavDevItem : public QObject, public muse::async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QVariant index READ index NOTIFY indexChanged)
    Q_PROPERTY(bool enabled READ enabled NOTIFY enabledChanged)
    Q_PROPERTY(bool active READ active NOTIFY activeChanged)

    QML_ELEMENT;
    QML_UNCREATABLE("Not creatable as it is an abstract base class")

public:
    explicit AbstractKeyNavDevItem(muse::ui::INavigation* keynav);

    QString name() const;
    QVariant index() const;
    bool enabled() const;
    bool active() const;

signals:
    void indexChanged();
    void enabledChanged();
    void activeChanged();

private:
    muse::ui::INavigation* m_keynav = nullptr;
};
}
