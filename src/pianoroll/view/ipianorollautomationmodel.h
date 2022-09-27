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

#ifndef MU_PIANOROLL_IPIANOROLLAUTOMATIONMODEL_H
#define MU_PIANOROLL_IPIANOROLLAUTOMATIONMODEL_H

#include "pianorollautomationeditor.h"

#include <vector>

namespace mu::pianoroll {
struct NoteEventBlock
{
    Ms::Note* note;
    int voice;
    int staffIdx;
};

class IPianorollAutomationModel
{
public:
    virtual ~IPianorollAutomationModel() = default;

    virtual PianorollAutomationEditor::AutomationType type() = 0;

    virtual double maxValue() = 0;
    virtual double minValue() = 0;
    virtual double value(Ms::Staff* staff, NoteEventBlock& block) = 0;
    virtual void setValue(Ms::Staff* staff, NoteEventBlock& block, double value) = 0;
};
}

#endif // MU_PIANOROLL_IPIANOROLLAUTOMATIONMODEL_H
