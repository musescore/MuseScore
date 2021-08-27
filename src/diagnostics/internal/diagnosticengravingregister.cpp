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
#include "diagnosticengravingregister.h"

#include "engraving/libmscore/score.h"

using namespace mu::diagnostics;

void DiagnosticEngravingRegister::reg(Ms::ScoreElement* e)
{
    if (e->score()->isPaletteScore()) {
        return;
    }

    m_elements.push_back(e);
    m_registred.send(e);
}

void DiagnosticEngravingRegister::unreg(Ms::ScoreElement* e)
{
    m_elements.remove(e);
    m_unregistred.send(e);
}

std::list<Ms::ScoreElement*> DiagnosticEngravingRegister::elements() const
{
    return m_elements;
}

mu::async::Channel<Ms::ScoreElement*> DiagnosticEngravingRegister::registred() const
{
    return m_registred;
}

mu::async::Channel<Ms::ScoreElement*> DiagnosticEngravingRegister::unregistred() const
{
    return m_unregistred;
}
