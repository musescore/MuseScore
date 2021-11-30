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
#ifndef MU_NOTATION_NOTATIONPAINTING_H
#define MU_NOTATION_NOTATIONPAINTING_H

#include "../inotationpainting.h"
#include "igetscore.h"

#include "modularity/ioc.h"
#include "../inotationconfiguration.h"
#include "engraving/iengravingconfiguration.h"

namespace Ms {
class Score;
}

namespace mu::notation {
class Notation;
class NotationPainting : public INotationPainting
{
    INJECT(notation, INotationConfiguration, configuration)
    INJECT(notation, engraving::IEngravingConfiguration, engravingConfiguration)

public:
    NotationPainting(Notation* notation);

    void setViewMode(const ViewMode& viewMode) override;
    ViewMode viewMode() const override;

    SizeF pageSizeInch() const override;

    void paintView(draw::Painter* painter, const RectF& frameRect) override;
    void paintPdf(draw::PagedPaintDevice* dev, draw::Painter* painter, const Options& opt) override;
    void paintPng(draw::Painter* painter, const Options& opt) override;

private:
    Ms::Score* score() const;

    void paintPages(mu::draw::Painter* painter, const RectF& frameRect, const QList<Ms::Page*>& pages, bool paintBorders) const;
    void paintPageBorder(mu::draw::Painter* painter, const Ms::Page* page) const;
    void paintForeground(mu::draw::Painter* painter, const RectF& pageRect) const;

    Notation* m_notation = nullptr;
};
}

#endif // MU_NOTATION_NOTATIONPAINTING_H
