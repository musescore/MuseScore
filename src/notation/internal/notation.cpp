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
#include "masternotationmididata.h"
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

Notation::Notation(Ms::Score* score)
{
    m_opened.val = false;

    m_painting = std::make_shared<NotationPainting>(this);
    m_undoStack = std::make_shared<NotationUndoStack>(this, m_notationChanged);
    m_interaction = std::make_shared<NotationInteraction>(this, m_undoStack);
    m_midiInput = std::make_shared<NotationMidiInput>(this, m_undoStack);
    m_accessibility = std::make_shared<NotationAccessibility>(this);
    m_parts = std::make_shared<NotationParts>(this, m_interaction, m_undoStack);
    m_playback = std::make_shared<NotationPlayback>(this, m_notationChanged);
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
        for (Ms::Score* score : m_score->scoreList()) {
            score->doLayout();
        }
    });

    setScore(score);
}

Notation::~Notation()
{
    //! Note Dereference internal pointers before the deallocation of Ms::Score* in order to prevent access to dereferenced object
    //! Makes sense to use std::shared_ptr<Ms::Score*> ubiquitous instead of the raw pointers
    m_parts = nullptr;
    m_playback = nullptr;
    m_undoStack = nullptr;
    m_interaction = nullptr;
    m_midiInput = nullptr;
    m_accessibility = nullptr;
    m_style = nullptr;
    m_elements = nullptr;
    m_painting = nullptr;

    delete m_score;
}

void Notation::init()
{
    bool isVertical = configuration()->canvasOrientation().val == framework::Orientation::Vertical;
    Ms::MScore::setVerticalOrientation(isVertical);

    Ms::MScore::playRepeats = configuration()->isPlayRepeatsEnabled();
}

void Notation::setScore(Ms::Score* score)
{
    m_score = score;

    if (score) {
        static_cast<NotationInteraction*>(m_interaction.get())->init();
        static_cast<NotationPlayback*>(m_playback.get())->init();
    }
}

QString Notation::title() const
{
    return m_score ? m_score->title() : QString();
}

QString Notation::completedTitle() const
{
    if (!m_score) {
        return QString();
    }

    QString title = m_score->metaTag("workTitle");
    if (title.isEmpty()) { // workTitle unset?
        title = m_score->masterScore()->title(); // fall back to (master)score's tab title
    }

    if (!m_score->isMaster()) { // excerpt?
        QString partName = m_score->metaTag("partName");
        if (partName.isEmpty()) { // partName unset?
            partName = m_score->title(); // fall back to excerpt's tab title
        }

        title += " - " + partName;
    }

    return title;
}

QString Notation::scoreTitle() const
{
    return m_score ? m_score->masterScore()->title() : QString();
}

mu::ValCh<bool> Notation::opened() const
{
    return m_opened;
}

void Notation::setOpened(bool opened)
{
    if (m_opened.val == opened) {
        return;
    }

    m_opened.set(opened);
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

INotationPlaybackPtr Notation::playback() const
{
    return m_playback;
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

Ms::Score* Notation::score() const
{
    return m_score;
}
