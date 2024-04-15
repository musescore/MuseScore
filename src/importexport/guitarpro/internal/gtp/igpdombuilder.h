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
#ifndef MU_IMPORTEXPORT_IGPDOMBUILDER_H
#define MU_IMPORTEXPORT_IGPDOMBUILDER_H

#include <memory>

#include "serialization/xmldom.h"
#include "gpdommodel.h"

namespace mu::iex::guitarpro {
class IGPDomBuilder
{
public:
    virtual ~IGPDomBuilder() = default;
    virtual void buildGPDomModel(muse::XmlDomElement* domElem) = 0;
    virtual std::unique_ptr<GPDomModel> getGPDomModel() = 0;
};
} // namespace mu::iex::guitarpro
#endif // MU_IMPORTEXPORT_IGPDOMBUILDER_H
