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

#ifndef MU_NOTATION_EXCERPTNOTATION_H
#define MU_NOTATION_EXCERPTNOTATION_H

#include "iexcerptnotation.h"
#include "notation.h"

namespace mu::notation {
class ExcerptNotation : public IExcerptNotation, public Notation, public std::enable_shared_from_this<ExcerptNotation>
{
public:
    explicit ExcerptNotation() = default;
    explicit ExcerptNotation(mu::engraving::Excerpt* excerpt);

    ~ExcerptNotation() override;

    void init();

    bool isCustom() const override;
    bool isEmpty() const override;

    mu::engraving::Excerpt* excerpt() const;

    QString name() const override;
    void setName(const QString& name) override;

    INotationPtr notation() override;
    IExcerptNotationPtr clone() const override;

private:
    void fillWithDefaultInfo();

    mu::engraving::Excerpt* m_excerpt = nullptr;
    QString m_name;
    bool m_inited = false;
};
}

#endif // MU_NOTATION_EXCERPTNOTATION_H
