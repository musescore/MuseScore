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

#include "../inotationpainting.h"

#include "modularity/ioc.h"
#include "../inotationconfiguration.h"
#include "engraving/iengravingconfiguration.h"
#include "engraving/rendering/iscorerenderer.h"
#include "ui/iuiconfiguration.h"

namespace mu::engraving {
class Score;
class Page;
}

namespace mu::notation {
class Notation;
class NotationPainting : public INotationPainting
{
    muse::GlobalInject<INotationConfiguration> configuration;
    muse::GlobalInject<engraving::IEngravingConfiguration> engravingConfiguration;
    muse::GlobalInject<muse::ui::IUiConfiguration> uiConfiguration;
    muse::GlobalInject<engraving::rendering::IScoreRenderer> scoreRenderer;

public:
    NotationPainting(Notation* notation);

    void setViewMode(const ViewMode& viewMode) override;
    ViewMode viewMode() const override;
    muse::async::Notification viewModeChanged() const override;

    int pageCount() const override;
    muse::SizeF pageSizeInch() const override;
    muse::SizeF pageSizeInch(const Options& opt) const override;

    void paintView(muse::draw::Painter* painter, const muse::RectF& frameRect, bool isPrinting, bool isAutomation) override;
    void paintPdf(muse::draw::Painter* painter, const Options& opt) override;
    void paintPrint(muse::draw::Painter* painter, const Options& opt) override;
    void paintPng(muse::draw::Painter* painter, const Options& opt) override;

private:
    mu::engraving::Score* score() const;

    bool isPaintPageBorder() const;
    void doPaint(muse::draw::Painter* painter, const Options& opt);
    void paintPageBorder(muse::draw::Painter* painter, const mu::engraving::Page* page) const;
    void paintPageSheet(muse::draw::Painter* painter, const engraving::Page* page, const muse::RectF& pageRect,
                        bool printPageBackground) const;

    Notation* m_notation = nullptr;

    muse::async::Notification m_viewModeChanged;
};
}
