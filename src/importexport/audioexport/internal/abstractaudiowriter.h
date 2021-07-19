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
#ifndef MU_IMPORTEXPORT_ABSTRACTAUDIOWRITER_H
#define MU_IMPORTEXPORT_ABSTRACTAUDIOWRITER_H

#include "project/inotationwriter.h"

namespace mu::iex::audioexport {
class AbstractAudioWriter : public project::INotationWriter
{
public:
    AbstractAudioWriter() = default;
    virtual ~AbstractAudioWriter() = default;

    std::vector<UnitType> supportedUnitTypes() const override;
    bool supportsUnitType(UnitType unitType) const override;

    Ret write(notation::INotationPtr notation, io::Device& destinationDevice, const Options& options = Options()) override;
    Ret writeList(const notation::INotationPtrList& notations, io::Device& destinationDevice, const Options& options = Options()) override;

    void abort() override;
    framework::ProgressChannel progress() const override;

protected:
    UnitType unitTypeFromOptions(const Options& options) const;
    framework::ProgressChannel m_progress;
};
}

#endif // MU_IMPORTEXPORT_ABSTRACTAUDIOWRITER_H
