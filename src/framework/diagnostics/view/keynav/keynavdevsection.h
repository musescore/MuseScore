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
#ifndef MUSE_DIAGNOSTICS_KEYNAVDEVSECTION_H
#define MUSE_DIAGNOSTICS_KEYNAVDEVSECTION_H

#include "abstractkeynavdevitem.h"

namespace muse::diagnostics {
class KeyNavDevSection : public AbstractKeyNavDevItem
{
    Q_OBJECT
    Q_PROPERTY(QVariantList subsections READ subsections NOTIFY subsectionsChanged)
    Q_PROPERTY(int panelsCount READ panelsCount NOTIFY panelsCountChanged)
    Q_PROPERTY(int controlsCount READ controlsCount NOTIFY controlsCountChanged)

public:
    explicit KeyNavDevSection(muse::ui::INavigationSection* section);

    QVariantList subsections() const;
    int panelsCount() const;
    int controlsCount() const;

public slots:
    void setSubsections(const QVariantList& subsections);

signals:
    void subsectionsChanged();
    void panelsCountChanged();
    void controlsCountChanged();

private:
    muse::ui::INavigationSection* m_section = nullptr;
    QVariantList m_subsections;
};
}

#endif // MUSE_DIAGNOSTICS_KEYNAVDEVSECTION_H
