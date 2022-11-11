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

#include "excerptnotation.h"

#include "libmscore/excerpt.h"
#include "libmscore/text.h"

#include "log.h"

using namespace mu::notation;

ExcerptNotation::ExcerptNotation(mu::engraving::Excerpt* excerpt)
    : Notation(), m_excerpt(excerpt)
{
}

ExcerptNotation::~ExcerptNotation()
{
    setScore(nullptr);
}

void ExcerptNotation::init()
{
    if (m_inited) {
        return;
    }

    setScore(m_excerpt->excerptScore());

    if (isEmpty()) {
        fillWithDefaultInfo();
    }

    m_inited = true;
}

bool ExcerptNotation::isCustom() const
{
    return !m_excerpt->initialPartId().isValid();
}

bool ExcerptNotation::isEmpty() const
{
    return m_excerpt->parts().empty();
}

void ExcerptNotation::fillWithDefaultInfo()
{
    TRACEFUNC;

    IF_ASSERT_FAILED(m_excerpt || m_excerpt->excerptScore()) {
        return;
    }

    mu::engraving::Score* score = m_excerpt->excerptScore();
    mu::engraving::MeasureBase* topVerticalFrame = score->first();

    if (topVerticalFrame && topVerticalFrame->isVBox()) {
        topVerticalFrame->undoUnlink();
    }

    auto setText = [&score](TextStyleType textType, const QString& text) {
        TextBase* textItem = score->getText(textType);

        if (!textItem) {
            textItem = score->addText(textType, nullptr /*destinationElement*/, false /*addToAllScores*/);
        }

        if (textItem) {
            textItem->undoUnlink();
            textItem->setPlainText(text);
        }
    };

    setText(TextStyleType::TITLE, qtrc("notation", "Title"));
    setText(TextStyleType::COMPOSER, qtrc("notation", "Composer / arranger"));
    setText(TextStyleType::SUBTITLE, "");
    setText(TextStyleType::POET, "");
}

mu::engraving::Excerpt* ExcerptNotation::excerpt() const
{
    return m_excerpt;
}

QString ExcerptNotation::name() const
{
    return m_excerpt->name().toQString();
}

void ExcerptNotation::setName(const QString& name)
{
    bool changed = name != this->name();
    m_excerpt->setName(name);

    if (changed) {
        notifyAboutNotationChanged();
    }
}

mu::async::Notification ExcerptNotation::nameChanged() const
{
    return m_excerpt->nameChanged();
}

INotationPtr ExcerptNotation::notation()
{
    return shared_from_this();
}

IExcerptNotationPtr ExcerptNotation::clone() const
{
    mu::engraving::Excerpt* copy = new mu::engraving::Excerpt(*m_excerpt);
    return std::make_shared<ExcerptNotation>(copy);
}
