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
#include "mu4inspectoradapter.h"

#include "log.h"

using namespace mu::inspector;
using namespace mu::notation;

bool MU4InspectorAdapter::isNotationExisting() const
{
    return !context()->masterNotations().empty();
}

bool MU4InspectorAdapter::isTextEditingStarted() const
{
    IF_ASSERT_FAILED(context() && context()->currentNotation()) {
        return false;
    }

    return context()->currentNotation()->interaction()->isTextEditingStarted();
}

mu::async::Notification MU4InspectorAdapter::isTextEditingChanged() const
{
    IF_ASSERT_FAILED(context() && context()->currentNotation()) {
        return mu::async::Notification();
    }

    return context()->currentNotation()->interaction()->textEditingChanged();
}

void MU4InspectorAdapter::beginCommand()
{
    if (undoStack()) {
        undoStack()->prepareChanges();
    }
}

void MU4InspectorAdapter::endCommand()
{
    if (undoStack()) {
        undoStack()->commitChanges();
    }
}

void MU4InspectorAdapter::updateStyleValue(const Ms::Sid& styleId, const QVariant& newValue)
{
    if (style()) {
        style()->setStyleValue(styleId, newValue);
    }
}

QVariant MU4InspectorAdapter::styleValue(const Ms::Sid& styleId)
{
    return style() ? style()->styleValue(styleId) : QVariant();
}

void MU4InspectorAdapter::showSpecialCharactersDialog()
{
    NOT_IMPLEMENTED;
}

void MU4InspectorAdapter::showStaffTextPropertiesDialog()
{
    NOT_IMPLEMENTED;
}

void MU4InspectorAdapter::showPageSettingsDialog()
{
    NOT_IMPLEMENTED;
}

void MU4InspectorAdapter::showStyleSettingsDialog()
{
    interactive()->open("musescore://notation/style");
}

void MU4InspectorAdapter::showTimeSignaturePropertiesDialog()
{
    NOT_IMPLEMENTED;
}

void MU4InspectorAdapter::showArticulationPropertiesDialog()
{
    NOT_IMPLEMENTED;
}

void MU4InspectorAdapter::showGridConfigurationDialog()
{
    NOT_IMPLEMENTED;
}

void MU4InspectorAdapter::updatePageMarginsVisibility(const bool /*isVisible*/)
{
    NOT_IMPLEMENTED;
}

void MU4InspectorAdapter::updateFramesVisibility(const bool /*isVisible*/)
{
    NOT_IMPLEMENTED;
}

void MU4InspectorAdapter::updateHorizontalGridSnapping(const bool /*isSnapped*/)
{
    NOT_IMPLEMENTED;
}

void MU4InspectorAdapter::updateVerticalGridSnapping(const bool /*isSnapped*/)
{
    NOT_IMPLEMENTED;
}

void MU4InspectorAdapter::updateUnprintableElementsVisibility(const bool /*isVisible*/)
{
    NOT_IMPLEMENTED;
}

void MU4InspectorAdapter::updateInvisibleElementsDisplaying(const bool /*isVisible*/)
{
    NOT_IMPLEMENTED;
}

void MU4InspectorAdapter::updateNotation()
{
    IF_ASSERT_FAILED(context() && context()->currentNotation()) {
        return;
    }

    return context()->currentNotation()->notationChanged().notify();
}

INotationUndoStackPtr MU4InspectorAdapter::undoStack() const
{
    if (!context() || !context()->currentNotation()) {
        return nullptr;
    }

    return context()->currentNotation()->undoStack();
}

INotationStylePtr MU4InspectorAdapter::style() const
{
    if (!context() || !context()->currentNotation()) {
        return nullptr;
    }

    return context()->currentNotation()->style();
}
