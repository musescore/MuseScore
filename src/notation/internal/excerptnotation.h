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

#ifndef MU_NOTATION_EXCERPTNOTATION_H
#define MU_NOTATION_EXCERPTNOTATION_H

#include "iexcerptnotation.h"
#include "notation.h"

namespace mu::notation {
class ExcerptNotation : public IExcerptNotation, public Notation, public std::enable_shared_from_this<ExcerptNotation>
{
public:
    explicit ExcerptNotation(mu::engraving::Excerpt* excerpt, const muse::modularity::ContextPtr& iocCtx);

    ~ExcerptNotation() override;

    void init();
    void reinit(engraving::Excerpt* newExcerpt);

    engraving::Excerpt* excerpt() const;

    bool isInited() const override;
    bool isCustom() const override;
    bool isEmpty() const override;

    QString name() const override;
    void setName(const QString& name) override;
    void undoSetName(const QString& name) override;
    muse::async::Notification nameChanged() const override;

    bool hasFileName() const override;
    const muse::String& fileName() const override;

    INotationPtr notation() override;
    IExcerptNotationPtr clone() const override;

private:

    mu::engraving::Excerpt* m_excerpt = nullptr;
    bool m_inited = false;
};
}

#endif // MU_NOTATION_EXCERPTNOTATION_H
