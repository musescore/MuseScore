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
    //! NOTE: do not destroy the score here, because it may be stored in UndoStack
    //! (after opening an excerpt via the Parts dialog and pressing ctrl + z)
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

    auto getText = [&score](TextStyleType textType, const QString& defaultText) {
        if (mu::engraving::Text* t = score->getText(textType)) {
            return t->plainText().toQString();
        } else {
            return defaultText;
        }
    };

    setText(TextStyleType::TITLE, getText(TextStyleType::TITLE, ""));
    setText(TextStyleType::COMPOSER, getText(TextStyleType::COMPOSER, ""));
    setText(TextStyleType::SUBTITLE, getText(TextStyleType::SUBTITLE, ""));
    setText(TextStyleType::POET, getText(TextStyleType::POET, ""));
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
