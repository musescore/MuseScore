/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include <algorithm>

#include "engraving/dom/excerpt.h"
#include "engraving/dom/masterscore.h"
#include "engraving/editing/editexcerpt.h"
#include "engraving/editing/transaction/transaction.h"

#include "inotationundostack.h"
#include "masternotation.h"
#include "project/inotationproject.h"

using namespace mu::notation;

ExcerptNotation::ExcerptNotation(MasterNotation* master, engraving::Excerpt* excerpt, const muse::modularity::ContextPtr& iocCtx)
    : Notation(master, iocCtx), m_excerpt(excerpt)
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
    if (score()) {
        return;
    }

    setScore(m_excerpt->excerptScore());
}

void ExcerptNotation::deinit()
{
    if (!m_excerpt->inited()) {
        return;
    }

    // Delete the excerptScore and reset to uninitialised state
    mu::engraving::Score* excerptScore = m_excerpt->excerptScore();
    setScore(nullptr);
    m_excerpt->setExcerptScore(nullptr);
    m_excerpt->setInited(false);
    delete excerptScore;
}

void ExcerptNotation::reinit(engraving::Excerpt* newExcerpt)
{
    setScore(nullptr);
    m_excerpt = newExcerpt;

    init();

    notifyAboutNotationChanged();
}

bool ExcerptNotation::isInited() const
{
    return m_excerpt->inited();
}

bool ExcerptNotation::isCustom() const
{
    return m_excerpt->custom();
}

bool ExcerptNotation::isEmpty() const
{
    return m_excerpt->parts().empty();
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
        // Mark project as unsaved so asterisk appears and save confirmation is shown
        if (m_masterNotation && m_masterNotation->project()) {
            m_masterNotation->project()->markAsUnsaved();
        }
        notifyAboutNotationChanged();
    }
}

void ExcerptNotation::undoSetName(const QString& name)
{
    if (name == this->name()) {
        return;
    }

    if (!score()) {
        if (!m_masterNotation) {
            setName(name);
            return;
        }
        //: Means: "edit the name of a part score"
        m_masterNotation->undoStack()->transaction(muse::TranslatableString("undoableAction", "Rename part"),
                                                   [&](engraving::Transaction& tx) {
            tx.push(new engraving::ChangeExcerptTitle(m_excerpt, name));
        });
        notifyAboutNotationChanged();
        return;
    }

    //: Means: "edit the name of a part score"
    undoStack()->transaction(muse::TranslatableString("undoableAction", "Rename part"), [&](engraving::Transaction& tx) {
        tx.push(new engraving::ChangeExcerptTitle(m_excerpt, name));
    });

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

    return std::make_shared<ExcerptNotation>(m_masterNotation, copy, iocContext());
}
