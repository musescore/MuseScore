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
#include "musesampleruiactions.h"

#include "context/uicontext.h"
#include "types/translatablestring.h"

using namespace mu::ui;
using namespace mu::actions;
using namespace mu::musesampler;

const UiActionList MuseSamplerUiActions::m_actions = {
    UiAction("musesampler-check",
             ActionCategory::Undefined,
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "Check Muse Sampler")
             )
};

const UiActionList& MuseSamplerUiActions::actionsList() const
{
    return m_actions;
}

bool MuseSamplerUiActions::actionEnabled(const UiAction&) const
{
    return true;
}

mu::async::Channel<ActionCodeList> MuseSamplerUiActions::actionEnabledChanged() const
{
    static async::Channel<ActionCodeList> ch;
    return ch;
}

bool MuseSamplerUiActions::actionChecked(const UiAction&) const
{
    return false;
}

mu::async::Channel<ActionCodeList> MuseSamplerUiActions::actionCheckedChanged() const
{
    static async::Channel<ActionCodeList> ch;
    return ch;
}
