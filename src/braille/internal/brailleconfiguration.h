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
#ifndef MU_BRAILLE_BRAILLECONFIGURATION_H
#define MU_BRAILLE_BRAILLECONFIGURATION_H

#include "async/asyncable.h"

#include "ibrailleconfiguration.h"

namespace mu::engraving {
class BrailleConfiguration : public mu::braille::IBrailleConfiguration, public async::Asyncable
{
public:
    void init();

    async::Notification braillePanelEnabledChanged() const override;
    bool braillePanelEnabled() const override;
    void setBraillePanelEnabled(const bool enabled) override;

    async::Notification brailleTableChanged() const override;
    QString brailleTable() const override;
    void setBrailleTable(const QString table) override;

    QStringList brailleTableList() override;

private:
    async::Notification m_braillePanelEnabledChanged;
    async::Notification m_brailleTableChanged;
};
}

#endif // MU_BRAILLE_BRAILLECONFIGURATION_H
