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

#include "tie.h"

using namespace mu::engraving::apiv1;

Note* Tie::startNote() { return wrap<Note>(toTie(e)->startNote()); }

Note* Tie::endNote() { return wrap<Note>(toTie(e)->endNote()); }

Tie* mu::engraving::apiv1::tieWrap(mu::engraving::Tie* tie) { return wrap<Tie>(tie); }
