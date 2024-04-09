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
#ifndef MUSE_DIAGNOSTICS_KEYNAVDEVSUBSECTION_H
#define MUSE_DIAGNOSTICS_KEYNAVDEVSUBSECTION_H

#include "abstractkeynavdevitem.h"

namespace muse::diagnostics {
class KeyNavDevSubSection : public AbstractKeyNavDevItem
{
    Q_OBJECT
    Q_PROPERTY(QString direction READ direction CONSTANT)
    Q_PROPERTY(QVariantList controls READ controls NOTIFY controlsChanged)
    Q_PROPERTY(int controlsCount READ controlsCount NOTIFY controlsCountChanged)

public:
    explicit KeyNavDevSubSection(muse::ui::INavigationPanel* subsection);

    QString direction() const;
    QVariantList controls() const;
    int controlsCount() const;

public slots:
    void setControls(const QVariantList& controls);

signals:
    void controlsChanged();
    void controlsCountChanged();

private:
    muse::ui::INavigationPanel* m_subsection = nullptr;
    QVariantList m_controls;
};
}

#endif // MUSE_DIAGNOSTICS_KEYNAVDEVSUBSECTION_H
