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

#ifndef __PLUGIN_API_TIE_H__
#define __PLUGIN_API_TIE_H__

#include "elements.h"
#include "libmscore/tie.h"

namespace Ms {
namespace PluginAPI {
//---------------------------------------------------------
//   Tie
///  Provides access to internal Ms::Tie objects.
///  \since MuseScore 3.3
//---------------------------------------------------------

class Tie : public Element
{
    Q_OBJECT
    /// The starting note of the tie.
    /// \since MuseScore 3.3
    Q_PROPERTY(Ms::PluginAPI::Note* startNote READ startNote)
    /// The ending note of the tie.
    /// \since MuseScore 3.3
    Q_PROPERTY(Ms::PluginAPI::Note* endNote READ endNote)

    /// \cond MS_INTERNAL

public:
    Tie(Ms::Tie* tie, Ownership own = Ownership::PLUGIN)
        : Element(tie, own) {}

    Note* startNote();
    Note* endNote();

    /// \endcond
};

extern Tie* tieWrap(Ms::Tie* tie);
} // namespace PluginAPI
} // namespace Ms
#endif
