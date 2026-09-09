/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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
#pragma once

#include <string>

#include "ui/view/iconcodes.h"

namespace muse::toast {
enum class ToastActionCode {
    None = 0,

    Dismiss,
    TryAgain,

    Custom,
};

struct ToastAction {
    std::string text;
    int code = int(ToastActionCode::None);
    bool accent = false;
    muse::ui::IconCode::Code icon = muse::ui::IconCode::Code::NONE;

    ToastAction() = default;
    ToastAction(const std::string& text, int code, bool accent = false,
                muse::ui::IconCode::Code icon = muse::ui::IconCode::Code::NONE)
        : text(text), code(code), accent(accent), icon(icon) {}
    ToastAction(const std::string& text, ToastActionCode code, bool accent = false,
                muse::ui::IconCode::Code icon = muse::ui::IconCode::Code::NONE)
        : text(text), code(int(code)), accent(accent), icon(icon) {}
};

struct ToastResult {
    ToastResult() = default;
    ToastResult(int actionCode)
        : m_code(actionCode) {}

    ToastActionCode standardCode() const { return static_cast<ToastActionCode>(m_code); }
    int actionCode() const { return m_code; }
    bool isCode(int code) const { return code == m_code; }
    bool isCode(ToastActionCode code) const { return int(code) == m_code; }

private:
    int m_code = int(ToastActionCode::None);
};
}
