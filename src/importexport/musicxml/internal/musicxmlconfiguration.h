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

#include "../imusicxmlconfiguration.h"
#include "async/asyncable.h"

namespace mu::iex::musicxml {
class MusicXmlConfiguration : public IMusicXmlConfiguration, public muse::async::Asyncable
{
public:
    void init();

    bool importBreaks() const override;
    void setImportBreaks(bool value) override;
    muse::async::Channel<bool> importBreaksChanged() const override;

    bool importLayout() const override;
    void setImportLayout(bool value) override;
    muse::async::Channel<bool> importLayoutChanged() const override;

    bool exportLayout() const override;
    void setExportLayout(bool value) override;

    bool exportMu3Compat() const override;
    void setExportMu3Compat(bool value) override;

    MusicXmlExportBreaksType exportBreaksType() const override;
    void setExportBreaksType(MusicXmlExportBreaksType breaksType) override;

    bool exportInvisibleElements() const override;
    void setExportInvisibleElements(bool value) override;

    bool needUseDefaultFont() const override;
    void setNeedUseDefaultFont(bool value) override;
    muse::async::Channel<bool> needUseDefaultFontChanged() const override;
    void setNeedUseDefaultFontOverride(std::optional<bool> value) override;

    bool needAskAboutApplyingNewStyle() const override;
    void setNeedAskAboutApplyingNewStyle(bool value) override;
    muse::async::Channel<bool> needAskAboutApplyingNewStyleChanged() const override;

    bool inferTextType() const override;
    void setInferTextType(bool value) override;
    muse::async::Channel<bool> inferTextTypeChanged() const override;
    void setInferTextTypeOverride(std::optional<bool> value) override;

private:
    std::optional<bool> m_needUseDefaultFontOverride;
    std::optional<bool> m_inferTextTypeOverride;

    muse::async::Channel<bool> m_importBreaksChanged;
    muse::async::Channel<bool> m_importLayoutChanged;
    muse::async::Channel<bool> m_needUseDefaultFontChanged;
    muse::async::Channel<bool> m_needAskAboutApplyingNewStyleChanged;
    muse::async::Channel<bool> m_inferTextTypeChanged;
};
}
