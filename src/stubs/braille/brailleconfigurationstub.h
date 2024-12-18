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
#ifndef MU_BRAILLE_BRAILLECONFIGURATIONSTUB_H
#define MU_BRAILLE_BRAILLECONFIGURATIONSTUB_H

#include "braille/ibrailleconfiguration.h"

namespace mu::braille {
class BrailleConfigurationStub : public IBrailleConfiguration
{
public:
    muse::async::Notification braillePanelEnabledChanged() const override;
    bool braillePanelEnabled() const override;
    void setBraillePanelEnabled(const bool enabled) override;

    muse::async::Notification intervalDirectionChanged() const override;
    BrailleIntervalDirection intervalDirection() const override;
    void setIntervalDirection(const BrailleIntervalDirection direction) override;

    muse::async::Notification brailleTableChanged() const override;
    QString brailleTable() const override;
    void setBrailleTable(const QString& table) override;
    QStringList brailleTableList() const override;
};
}

#endif // MU_BRAILLE_BRAILLECONFIGURATIONSTUB_H
