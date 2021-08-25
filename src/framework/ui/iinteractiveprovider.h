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
#ifndef MU_UI_IINTERACTIVEPROVIDER_H
#define MU_UI_IINTERACTIVEPROVIDER_H

#include "modularity/imoduleexport.h"
#include "uri.h"
#include "retval.h"

#include "iinteractive.h"

namespace mu::ui {
class IInteractiveProvider : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(ILaunchProvider)

public:
    virtual ~IInteractiveProvider() = default;

    virtual RetVal<Val> question(const std::string& title, const framework::IInteractive::Text& text,
                                 const framework::IInteractive::ButtonDatas& buttons,
                                 int defBtn = int(framework::IInteractive::Button::NoButton),
                                 const framework::IInteractive::Options& options = {}) = 0;

    virtual RetVal<Val> info(const std::string& title, const std::string& text, const framework::IInteractive::ButtonDatas& buttons,
                             int defBtn = int(framework::IInteractive::Button::NoButton),
                             const framework::IInteractive::Options& options = {}) = 0;

    virtual RetVal<Val> warning(const std::string& title, const std::string& text, const framework::IInteractive::ButtonDatas& buttons,
                                int defBtn = int(framework::IInteractive::Button::NoButton),
                                const framework::IInteractive::Options& options = {}) = 0;

    virtual RetVal<Val> error(const std::string& title, const std::string& text, const framework::IInteractive::ButtonDatas& buttons,
                              int defBtn = int(framework::IInteractive::Button::NoButton),
                              const framework::IInteractive::Options& options = {}) = 0;

    virtual RetVal<Val> open(const UriQuery& uri) = 0;
    virtual RetVal<bool> isOpened(const Uri& uri) const = 0;

    virtual void close(const Uri& uri) = 0;

    virtual ValCh<Uri> currentUri() const = 0;
    virtual std::vector<Uri> stack() const = 0;
};
}

#endif // MU_UI_IINTERACTIVEPROVIDER_H
