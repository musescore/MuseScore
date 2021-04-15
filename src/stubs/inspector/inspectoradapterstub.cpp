/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
#include "inspectoradapterstub.h"

using namespace mu::inspector;

bool InspectorAdapterStub::isNotationExisting() const
{
    return false;
}

bool InspectorAdapterStub::isTextEditingStarted() const
{
    return false;
}

mu::async::Notification InspectorAdapterStub::isTextEditingChanged() const
{
    return mu::async::Notification();
}

void InspectorAdapterStub::beginCommand()
{
}

void InspectorAdapterStub::endCommand()
{
}

void InspectorAdapterStub::updateStyleValue(const Ms::Sid&, const QVariant&)
{
}

QVariant InspectorAdapterStub::styleValue(const Ms::Sid&)
{
    return QVariant();
}

void InspectorAdapterStub::showSpecialCharactersDialog()
{
}

void InspectorAdapterStub::showStaffTextPropertiesDialog()
{
}

void InspectorAdapterStub::showPageSettingsDialog()
{
}

void InspectorAdapterStub::showStyleSettingsDialog()
{
}

void InspectorAdapterStub::showTimeSignaturePropertiesDialog()
{
}

void InspectorAdapterStub::showArticulationPropertiesDialog()
{
}

void InspectorAdapterStub::showGridConfigurationDialog()
{
}

void InspectorAdapterStub::updatePageMarginsVisibility(const bool)
{
}

void InspectorAdapterStub::updateFramesVisibility(const bool)
{
}

void InspectorAdapterStub::updateHorizontalGridSnapping(const bool)
{
}

void InspectorAdapterStub::updateVerticalGridSnapping(const bool)
{
}

void InspectorAdapterStub::updateUnprintableElementsVisibility(const bool)
{
}

void InspectorAdapterStub::updateInvisibleElementsDisplaying(const bool)
{
}

void InspectorAdapterStub::updateNotation()
{
}
