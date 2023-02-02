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

#include "voicingSelect.h"

using namespace mu::notation;

VoicingSelect::VoicingSelect(QWidget* parent)
    : QWidget(parent)
{
    setupUi(this);

    //setup changed signals
    connect(interpretBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &VoicingSelect::_voicingChanged);
    connect(voicingBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &VoicingSelect::_voicingChanged);
    connect(durationBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &VoicingSelect::_voicingChanged);
}

void VoicingSelect::_voicingChanged()
{
    emit voicingChanged(interpretBox->currentIndex(), voicingBox->currentIndex(), durationBox->currentIndex());
}

void VoicingSelect::blockVoicingSignals(bool val)
{
    interpretBox->blockSignals(val);
    voicingBox->blockSignals(val);
    durationBox->blockSignals(val);
}

void VoicingSelect::setVoicing(int idx)
{
    blockVoicingSignals(true);
    voicingBox->setCurrentIndex(idx);
    blockVoicingSignals(false);
}

void VoicingSelect::setLiteral(bool literal)
{
    blockVoicingSignals(true);
    interpretBox->setCurrentIndex(literal);
    blockVoicingSignals(false);
}

void VoicingSelect::setDuration(int idx)
{
    blockVoicingSignals(true);
    durationBox->setCurrentIndex(idx);
    blockVoicingSignals(false);
}
