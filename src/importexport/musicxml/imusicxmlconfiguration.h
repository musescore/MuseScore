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
#pragma once

#include "modularity/imoduleinterface.h"
#include "io/path.h"

namespace mu::iex::musicxml {
class IMusicXmlConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IMusicXmlConfiguration)

public:
    virtual ~IMusicXmlConfiguration() = default;

    virtual bool importBreaks() const = 0;
    virtual void setImportBreaks(bool value) = 0;

    virtual bool importLayout() const = 0;
    virtual void setImportLayout(bool value) = 0;

    virtual bool exportLayout() const = 0;
    virtual void setExportLayout(bool value) = 0;

    virtual bool exportMu3Compat() const = 0;
    virtual void setExportMu3Compat(bool value) = 0;

    enum class MusicXmlExportBreaksType {
        All, Manual, No
    };

    virtual MusicXmlExportBreaksType exportBreaksType() const = 0;
    virtual void setExportBreaksType(MusicXmlExportBreaksType breaksType) = 0;

    virtual bool exportInvisibleElements() const = 0;
    virtual void setExportInvisibleElements(bool value) = 0;

    virtual bool needUseDefaultFont() const = 0;
    virtual void setNeedUseDefaultFont(bool value) = 0;
    virtual void setNeedUseDefaultFontOverride(std::optional<bool> value) = 0;

    virtual bool needAskAboutApplyingNewStyle() const = 0;
    virtual void setNeedAskAboutApplyingNewStyle(bool value) = 0;

    virtual bool inferTextType() const = 0;
    virtual void setInferTextType(bool value) = 0;
    virtual void setInferTextTypeOverride(std::optional<bool> value) = 0;
};
}
