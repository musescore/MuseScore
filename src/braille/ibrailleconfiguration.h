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
#ifndef MU_BRAILLE_IBRAILLECONFIGURATION_H
#define MU_BRAILLE_IBRAILLECONFIGURATION_H

#include "modularity/imoduleinterface.h"
#include "async/notification.h"

namespace mu::braille {
class IBrailleConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IBrailleConfiguration)

public:
    virtual ~IBrailleConfiguration() = default;

    virtual async::Notification braillePanelEnabledChanged() const = 0;
    virtual bool braillePanelEnabled() const = 0;
    virtual void setBraillePanelEnabled(const bool enabled) = 0;

    virtual async::Notification brailleTableChanged() const = 0;
    virtual QString brailleTable() const = 0;
    virtual void setBrailleTable(const QString table) = 0;

    virtual QStringList brailleTableList() = 0;
};
}

#endif // MU_BRAILLE_IBRAILLECONFIGURATION_H
