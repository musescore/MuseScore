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

#include "excerptnotation.h"

#include "engraving/dom/excerpt.h"
#include "engraving/dom/text.h"
#include "engraving/dom/undo.h"

#include "log.h"

using namespace mu::notation;

ExcerptNotation::ExcerptNotation(mu::engraving::Excerpt* excerpt, const muse::modularity::ContextPtr& iocCtx)
    : Notation(iocCtx), m_excerpt(excerpt)
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

void ExcerptNotation::reinit(engraving::Excerpt* newExcerpt)
{
    m_inited = false;
    m_excerpt = newExcerpt;

    init();

    notifyAboutNotationChanged();
}

bool ExcerptNotation::isInited() const
{
    return m_inited;
}

bool ExcerptNotation::isCustom() const
{
    return m_excerpt->custom();
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

    auto unlinkText = [&score](TextStyleType textType) {
        engraving::Text* textItem = score->getText(textType);
        if (textItem) {
            textItem->undoUnlink();
        }
    };

    unlinkText(TextStyleType::TITLE);
    unlinkText(TextStyleType::SUBTITLE);
    unlinkText(TextStyleType::COMPOSER);
    unlinkText(TextStyleType::LYRICIST);
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

void ExcerptNotation::undoSetName(const QString& name)
{
    if (name == this->name()) {
        return;
    }

    if (!score()) {
        setName(name);
        return;
    }

    //: Means: "edit the name of a part score"
    undoStack()->prepareChanges(muse::TranslatableString("undoableAction", "Rename part"));

    score()->undo(new engraving::ChangeExcerptTitle(m_excerpt, name));

    undoStack()->commitChanges();
    notifyAboutNotationChanged();
}

muse::async::Notification ExcerptNotation::nameChanged() const
{
    return m_excerpt->nameChanged();
}

bool ExcerptNotation::hasFileName() const
{
    return m_excerpt->hasFileName();
}

const muse::String& ExcerptNotation::fileName() const
{
    return m_excerpt->fileName();
}

INotationPtr ExcerptNotation::notation()
{
    return shared_from_this();
}

IExcerptNotationPtr ExcerptNotation::clone() const
{
    mu::engraving::Excerpt* copy = new mu::engraving::Excerpt(*m_excerpt);
    copy->markAsCustom();

    return std::make_shared<ExcerptNotation>(copy, iocContext());
}
