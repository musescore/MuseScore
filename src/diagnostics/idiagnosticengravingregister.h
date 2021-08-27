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
#ifndef MU_DIAGNOSTICS_IDIAGNOSTICENGRAVINGREGISTER_H
#define MU_DIAGNOSTICS_IDIAGNOSTICENGRAVINGREGISTER_H

#include <list>
#include "modularity/imoduleexport.h"
#include "async/channel.h"

namespace Ms {
class ScoreElement;
}

namespace mu::diagnostics {
class IDiagnosticEngravingRegister : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IDiagnosticEngravingRegister)
public:
    virtual ~IDiagnosticEngravingRegister() = default;

    virtual void reg(const Ms::ScoreElement* e) = 0;
    virtual void unreg(const Ms::ScoreElement* e) = 0;
    virtual std::list<const Ms::ScoreElement*> elements() const = 0;
    virtual async::Channel<const Ms::ScoreElement*> registred() const = 0;
    virtual async::Channel<const Ms::ScoreElement*> unregistred() const = 0;
};
}

#endif // MU_DIAGNOSTICS_IDIAGNOSTICENGRAVINGREGISTER_H
