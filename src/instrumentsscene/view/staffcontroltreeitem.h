/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#ifndef MU_INSTRUMENTSSCENE_STAFFCONTROLTREEITEM_H
#define MU_INSTRUMENTSSCENE_STAFFCONTROLTREEITEM_H

#include "abstractinstrumentspaneltreeitem.h"

#include "notation/inotationparts.h"

namespace mu::instrumentsscene {
class StaffControlTreeItem : public AbstractInstrumentsPanelTreeItem
{
    Q_OBJECT

public:
    StaffControlTreeItem(notation::IMasterNotationPtr masterNotation, notation::INotationPtr notation, QObject* parent);

    void init(const muse::ID& partId);

    Q_INVOKABLE void appendNewItem() override;

private:
    muse::ID m_partId;
};
}

#endif // MU_INSTRUMENTSSCENE_STAFFCONTROLTREEITEM_H
