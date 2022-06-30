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
#include "notation.h"

#include <QGuiApplication>
#include <QScreen>

#include "log.h"

#include "libmscore/masterscore.h"
#include "libmscore/scorefont.h"
#include "libmscore/page.h"
#include "libmscore/rendermidi.h"
#include "engraving/paint/paint.h"

#include "notationpainting.h"
#include "notationinteraction.h"
#include "notationplayback.h"
#include "notationundostack.h"
#include "notationstyle.h"
#include "notationelements.h"
#include "notationaccessibility.h"
#include "notationmidiinput.h"
#include "notationparts.h"
#include "notationtypes.h"
#include "draw/pen.h"

using namespace mu::notation;

Notation::Notation(mu::engraving::Score* score)
{
    m_painting = std::make_shared<NotationPainting>(this);
    m_undoStack = std::make_shared<NotationUndoStack>(this, m_notationChanged);
    m_interaction = std::make_shared<NotationInteraction>(this, m_undoStack);
    m_midiInput = std::make_shared<NotationMidiInput>(this, m_undoStack);
    m_accessibility = std::make_shared<NotationAccessibility>(this);
    m_parts = std::make_shared<NotationParts>(this, m_interaction, m_undoStack);
    m_style = std::make_shared<NotationStyle>(this, m_undoStack);
    m_elements = std::make_shared<NotationElements>(this);

    m_interaction->noteInput()->noteAdded().onNotify(this, [this]() {
        notifyAboutNotationChanged();
    });

    m_interaction->dragChanged().onNotify(this, [this]() {
        notifyAboutNotationChanged();
    });

    m_interaction->textEditingChanged().onNotify(this, [this]() {
        notifyAboutNotationChanged();
    });

    m_interaction->dropChanged().onNotify(this, [this]() {
        notifyAboutNotationChanged();
    });

    m_midiInput->noteChanged().onNotify(this, [this]() {
        notifyAboutNotationChanged();
    });

    m_style->styleChanged().onNotify(this, [this]() {
        notifyAboutNotationChanged();
    });

    m_parts->partsChanged().onNotify(this, [this]() {
        notifyAboutNotationChanged();
    });

    engravingConfiguration()->selectionColorChanged().onReceive(this, [this](int, const mu::draw::Color&) {
        notifyAboutNotationChanged();
    });

    configuration()->canvasOrientation().ch.onReceive(this, [this](framework::Orientation) {
        m_score->doLayout();
        for (mu::engraving::Score* score : m_score->scoreList()) {
            score->doLayout();
        }
    });

    setScore(score);
}

Notation::~Notation()
{
    //! Note Dereference internal pointers before the deallocation of mu::engraving::Score* in order to prevent access to dereferenced object
    //! Makes sense to use std::shared_ptr<mu::engraving::Score*> ubiquitous instead of the raw pointers
    m_parts = nullptr;
    m_undoStack = nullptr;
    m_interaction = nullptr;
    m_midiInput = nullptr;
    m_accessibility = nullptr;
    m_style = nullptr;
    m_elements = nullptr;
    m_painting = nullptr;

    //! NOTE: The master score will be deleted later from ~EngravingProject()
    //! Its excerpts will be deleted directly in ~MasterScore()
    m_score = nullptr;
}

void Notation::init()
{
    bool isVertical = configuration()->canvasOrientation().val == framework::Orientation::Vertical;
    mu::engraving::MScore::setVerticalOrientation(isVertical);

    mu::engraving::MScore::playRepeats = configuration()->isPlayRepeatsEnabled();
}

void Notation::setScore(mu::engraving::Score* score)
{
    if (m_score == score) {
        return;
    }

    m_score = score;
    m_scoreInited.notify();
}

mu::async::Notification Notation::scoreInited() const
{
    return m_scoreInited;
}

QString Notation::name() const
{
    return m_score ? m_score->name().toQString() : QString();
}

QString Notation::projectName() const
{
    return m_score ? m_score->masterScore()->name().toQString() : QString();
}

QString Notation::projectNameAndPartName() const
{
    if (!m_score) {
        return QString();
    }

    QString result = m_score->masterScore()->name();
    if (!m_score->isMaster()) {
        result += " - " + m_score->name().toQString();
    }

    return result;
}

QString Notation::workTitle() const
{
    if (!m_score) {
        return QString();
    }

    QString workTitle = m_score->metaTag(u"workTitle");
    if (workTitle.isEmpty()) {
        return m_score->masterScore()->name();
    }

    return workTitle;
}

QString Notation::projectWorkTitle() const
{
    if (!m_score) {
        return QString();
    }

    QString workTitle = m_score->masterScore()->metaTag(u"workTitle");
    if (workTitle.isEmpty()) {
        return m_score->masterScore()->name();
    }

    return workTitle;
}

QString Notation::projectWorkTitleAndPartName() const
{
    if (!m_score) {
        return QString();
    }

    QString result = projectWorkTitle();
    if (!m_score->isMaster()) {
        result += " - " + name();
    }

    return result;
}

bool Notation::isOpen() const
{
    return score()->isOpen();
}

void Notation::setIsOpen(bool open)
{
    if (isOpen() == open) {
        return;
    }

    score()->setIsOpen(open);
    m_openChanged.notify();
}

mu::async::Notification Notation::openChanged() const
{
    return m_openChanged;
}

void Notation::notifyAboutNotationChanged()
{
    m_notationChanged.notify();
}

void Notation::setViewMode(const ViewMode& viewMode)
{
    m_painting->setViewMode(viewMode);
}

ViewMode Notation::viewMode() const
{
    return m_painting->viewMode();
}

INotationPaintingPtr Notation::painting() const
{
    return m_painting;
}

INotationInteractionPtr Notation::interaction() const
{
    return m_interaction;
}

INotationMidiInputPtr Notation::midiInput() const
{
    return m_midiInput;
}

INotationUndoStackPtr Notation::undoStack() const
{
    return m_undoStack;
}

INotationElementsPtr Notation::elements() const
{
    return m_elements;
}

INotationStylePtr Notation::style() const
{
    return m_style;
}

mu::async::Notification Notation::notationChanged() const
{
    return m_notationChanged;
}

INotationAccessibilityPtr Notation::accessibility() const
{
    return m_accessibility;
}

INotationPartsPtr Notation::parts() const
{
    return m_parts;
}

mu::engraving::Score* Notation::score() const
{
    return m_score;
}
