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

#include "notationpaintview.h"

#include "notation/imasternotation.h"
#include "notation/notationcommands.h"
#include "notation/inotationplayback.h"
#include "notation/inotationselection.h"
#include "notation/inotationviewstate.h"

#include "engraving/dom/note.h"
#include "engraving/playback/noteplaybackinfo.h"

using namespace mu;
using namespace muse::draw;
using namespace mu::notation;

NotationPaintView::NotationPaintView(QQuickItem* parent)
    : AbstractNotationPaintView(parent)
{
    m_influenceKeys.setModifiers(playbackInfluenceModifiers());

    commandShortcutsRegister()->shortcutsChanged().onNotify(this, [this]() {
        m_influenceKeys.setModifiers(playbackInfluenceModifiers());
    });

    connect(&m_influenceKeys, &ModifierHoldWatcher::heldChanged, this, [this](bool) {
        updateHeldPlaybackInfluence();
    });
}

void NotationPaintView::onLoadNotation(INotationPtr notation)
{
    m_isLocalMatrixUpdate = true;
    setMatrix(notation->viewState()->matrix());
    m_isLocalMatrixUpdate = false;

    notation->viewState()->matrixChanged().onReceive(this, [this](const Transform& matrix, NotationPaintView* sender) {
        if (sender != this) {
            m_isLocalMatrixUpdate = true;
            setMatrix(matrix);
            m_isLocalMatrixUpdate = false;
        }
    });

    AbstractNotationPaintView::onLoadNotation(notation);
}

void NotationPaintView::onUnloadNotation(INotationPtr notation)
{
    if (notation == m_heldInfluenceNotation) {
        m_heldInfluenceNotation->interaction()->setHeldPlaybackInfluenceHighlight({});
        m_heldInfluenceNotation = nullptr;
    }

    AbstractNotationPaintView::onUnloadNotation(notation);

    notation->viewState()->matrixChanged().disconnect(this);
}

void NotationPaintView::initZoomAndPosition()
{
    if (notation() && !notation()->viewState()->isMatrixInited()) {
        inputController()->initZoom();
        inputController()->initCanvasPos();
    }
}

void NotationPaintView::onMatrixChanged(const Transform& oldMatrix, const Transform& newMatrix, bool overrideZoomType)
{
    AbstractNotationPaintView::onMatrixChanged(oldMatrix, newMatrix, overrideZoomType);

    if (!m_isLocalMatrixUpdate && notation()) {
        notation()->viewState()->setMatrix(newMatrix, this);

        if (overrideZoomType) {
            notation()->viewState()->setZoomType(ZoomType::Percentage);
        }
    }
}

void NotationPaintView::updateHeldPlaybackInfluence()
{
    if (m_heldInfluenceNotation) {
        m_heldInfluenceNotation->interaction()->setHeldPlaybackInfluenceHighlight({});
        m_heldInfluenceNotation = nullptr;
    }

    const INotationPtr shown = notation();

    if (!m_influenceKeys.isHeld() || !shown || !isVisible()) {
        return;
    }

    const INotationSelectionPtr selection = shown->interaction()->selection();

    if (!selection->isSingle() || !selection->element() || !selection->element()->isNote()) {
        return;
    }

    // Playing hides the highlight when painting, so there's no need to check for it here
    const engraving::Note* note = engraving::toNote(selection->element());
    const engraving::NotePlaybackInfo info = shown->masterNotation()->playback()->notePlaybackInfo(note);

    if (!info.isValid()) {
        return;
    }

    shown->interaction()->setHeldPlaybackInfluenceHighlight(engraving::listedItems(info));
    m_heldInfluenceNotation = shown;
}

Qt::KeyboardModifiers NotationPaintView::playbackInfluenceModifiers() const
{
    const muse::shortcuts::Shortcut& shortcut = commandShortcutsRegister()->shortcut(HIGHLIGHT_PLAYBACK_INFLUENCES_COMMAND);

    for (const std::string& sequence : shortcut.sequences) {
        if (const Qt::KeyboardModifiers modifiers = muse::shortcuts::modifiersFromSequence(sequence); modifiers != Qt::NoModifier) {
            return modifiers;
        }
    }

    return Qt::NoModifier;
}
