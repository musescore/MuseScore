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

#include "log.h"

#include "libmscore/excerpt.h"

using namespace mu::notation;

ExcerptNotation::ExcerptNotation(Ms::Excerpt* excerpt)
    : Notation(), m_excerpt(excerpt)
{
    m_title = excerpt ? excerpt->name() : QString();
}

ExcerptNotation::~ExcerptNotation()
{
    if (!m_excerpt) {
        return;
    }

    Ms::MasterScore* master = m_excerpt->masterScore();
    if (master) {
        master->deleteExcerpt(m_excerpt);
    }

    delete m_excerpt;
    m_excerpt = nullptr;

    setScore(nullptr);
}

bool ExcerptNotation::isCreated() const
{
    return m_isCreated;
}

void ExcerptNotation::setIsCreated(bool created)
{
    m_isCreated = created;

    if (!created) {
        return;
    }

    setScore(m_excerpt->excerptScore());
    setTitle(m_title);

    if (isEmpty()) {
        fillWithDefaultInfo();
    }
}

void ExcerptNotation::fillWithDefaultInfo()
{
    TRACEFUNC;

    IF_ASSERT_FAILED(m_excerpt || m_excerpt->excerptScore()) {
        return;
    }

    Ms::Score* excerptScore = m_excerpt->excerptScore();

    auto setText = [&excerptScore](TextStyleType textType, const QString& text) {
        TextBase* textBox = excerptScore->getText(textType);

        if (!textBox) {
            textBox = excerptScore->addText(textType, false /*addToAllScores*/);
        }

        if (textBox) {
            textBox->unlink();
            textBox->setPlainText(text);
        }
    };

    setText(TextStyleType::TITLE, qtrc("notation", "Title"));
    setText(TextStyleType::COMPOSER, qtrc("notation", "Composer / arranger"));
    setText(TextStyleType::SUBTITLE, "");
    setText(TextStyleType::POET, "");

    excerptScore->doLayout();
}

Ms::Excerpt* ExcerptNotation::excerpt() const
{
    return m_excerpt;
}

bool ExcerptNotation::isEmpty() const
{
    return m_excerpt ? m_excerpt->parts().isEmpty() : true;
}

QString ExcerptNotation::title() const
{
    return m_excerpt ? m_excerpt->name() : m_title;
}

void ExcerptNotation::setTitle(const QString& title)
{
    m_title = title;

    if (!m_excerpt) {
        return;
    }

    m_excerpt->setName(title);

    if (!score()) {
        return;
    }

    Ms::Text* excerptTitle = score()->getText(Ms::TextStyleType::INSTRUMENT_EXCERPT);
    if (!excerptTitle) {
        return;
    }

    excerptTitle->setPlainText(title);
    score()->setMetaTag("partName", title);
    score()->doLayout();

    notifyAboutNotationChanged();
}

INotationPtr ExcerptNotation::notation()
{
    return shared_from_this();
}

IExcerptNotationPtr ExcerptNotation::clone() const
{
    if (!m_excerpt) {
        return nullptr;
    }

    Ms::Excerpt* copy = new Ms::Excerpt(*m_excerpt);
    return std::make_shared<ExcerptNotation>(copy);
}
