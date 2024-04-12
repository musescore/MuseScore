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

#include "excerpt.h"

#include "engraving/dom/score.h"

// api
#include "score.h"

using namespace mu::engraving::apiv1;
Score* Excerpt::partScore()
{
    return wrap<mu::engraving::apiv1::Score>(e->excerptScore(), Ownership::SCORE);
}

//---------------------------------------------------------
//   wrap
///   \cond PLUGIN_API \private \endcond
//---------------------------------------------------------

mu::engraving::apiv1::Excerpt* mu::engraving::apiv1::excerptWrap(mu::engraving::Excerpt* e)
{
    return excerptWrap<mu::engraving::apiv1::Excerpt>(e);
}
