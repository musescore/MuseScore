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
#ifndef MU_MIDI_IMIDICONFIGURATION_H
#define MU_MIDI_IMIDICONFIGURATION_H

#include "modularity/imoduleexport.h"

//! NOTE There used to be synthesizer settings here, now nothing
//! but we will keep the interface, maybe something will appear,
//! for example, midi port settings

namespace mu::midi {
class IMidiConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IMidiConfiguration)

public:
    virtual ~IMidiConfiguration() = default;

    virtual bool useRemoteControl() const = 0;
    virtual void setUseRemoteControl(bool value) = 0;
};
}

#endif // MU_MIDI_IMIDICONFIGURATION_H
