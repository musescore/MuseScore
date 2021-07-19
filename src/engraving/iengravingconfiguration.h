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
#ifndef MU_ENGRAVING_IENGRAVINGCONFIGURATION_H
#define MU_ENGRAVING_IENGRAVINGCONFIGURATION_H

#include <QString>

#include "modularity/imoduleexport.h"

namespace mu::engraving {
class IEngravingConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IEngravingConfiguration)
public:
    virtual ~IEngravingConfiguration() = default;

    virtual QString defaultStyleFilePath() const = 0;
    virtual void setDefaultStyleFilePath(const QString& path) = 0;

    virtual QString partStyleFilePath() const = 0;
    virtual void setPartStyleFilePath(const QString& path) = 0;
};
}

#endif // MU_ENGRAVING_IENGRAVINGCONFIGURATION_H
