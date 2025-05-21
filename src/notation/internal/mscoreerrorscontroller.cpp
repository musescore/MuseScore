/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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
#include "mscoreerrorscontroller.h"

#include "engraving/dom/mscore.h"

using namespace muse;
using namespace mu::engraving;
using namespace mu::notation;

MScoreErrorsController::MScoreErrorsController(const muse::modularity::ContextPtr& iocCtx)
    : muse::Injectable(iocCtx)
{
}

void MScoreErrorsController::checkAndShowMScoreError()
{
    TRACEFUNC;

    MsError err = MScore::_error;
    if (err == MsError::MS_NO_ERROR) {
        return;
    }

    MScore::setError(MsError::MS_NO_ERROR); // reset

    if (!configuration()->needToShowMScoreError(MScore::errorToString(err))) {
        return;
    }

    std::string title;
    std::string message;

    switch (err) {
    case MsError::MS_NO_ERROR:
        return;
    case MsError::NO_NOTE_SELECTED:
        title = muse::trc("notation", "No note selected");
        message = muse::trc("notation", "Please select a note and retry");
        break;
    case MsError::NO_CHORD_REST_SELECTED:
        title = muse::trc("notation", "No chord/rest selected");
        message = muse::trc("notation", "Please select a chord or rest and retry");
        break;
    case MsError::NO_LYRICS_SELECTED:
        title = muse::trc("notation", "No note or lyrics selected");
        message = muse::trc("notation", "Please select a note or lyrics and retry");
        break;
    case MsError::NO_NOTE_REST_SELECTED:
        title = muse::trc("notation", "No note or rest selected");
        message = muse::trc("notation", "Please select a note or rest and retry");
        break;
    case MsError::NO_FLIPPABLE_SELECTED:
        title = muse::trc("notation", "No flippable element selected");
        message = muse::trc("notation", "Please select an element that can be flipped and retry");
        break;
    case MsError::NO_STAFF_SELECTED:
        title = muse::trc("notation", "No staff selected");
        message = muse::trc("notation", "Please select one or more staves and retry");
        break;
    case MsError::NO_NOTE_FIGUREDBASS_SELECTED:
        title = muse::trc("notation", "No note or figured bass selected");
        message = muse::trc("notation", "Please select a note or figured bass and retry");
        break;
    case MsError::CANNOT_INSERT_TUPLET:
        title = muse::trc("notation", "Cannot insert chord/rest in tuplet");
        break;
    case MsError::CANNOT_SPLIT_TUPLET:
        title = muse::trc("notation", "Cannot split tuplet");
        break;
    case MsError::CANNOT_SPLIT_MEASURE_FIRST_BEAT:
        title = muse::trc("notation", "Cannot split measure at the first beat");
        break;
    case MsError::CANNOT_SPLIT_MEASURE_TUPLET:
        title = muse::trc("notation", "Cannot split measure here");
        message = muse::trc("notation", "Cannot split tuplet");
        break;
    case MsError::INSUFFICIENT_MEASURES:
        title = muse::trc("notation", "Cannot create measure repeat here");
        message = muse::trc("notation", "Insufficient or unequal measures");
        break;
    case MsError::CANNOT_SPLIT_MEASURE_REPEAT:
        title = muse::trc("notation", "Cannot split measure repeat");
        message = muse::trc("notation", "Please select a note or rest and retry");
        break;
    case MsError::CANNOT_SPLIT_MEASURE_TOO_SHORT:
        title = muse::trc("notation", "This measure is too short to be split");
        break;
    case MsError::CANNOT_REMOVE_TIME_TUPLET:
        title = muse::trc("notation", "Cannot remove time from tuplet");
        message = muse::trc("notation", "Please select the complete tuplet and retry");
        break;
    case MsError::CANNOT_REMOVE_TIME_MEASURE_REPEAT:
        title = muse::trc("notation", "Cannot remove time from measure repeat");
        message = muse::trc("notation", "Please select the complete measure repeat and retry");
        break;
    case MsError::NO_DEST:
        title = muse::trc("notation", "No destination to paste");
        break;
    case MsError::DEST_TUPLET:
        title = muse::trc("notation", "Cannot paste into tuplet");
        break;
    case MsError::TUPLET_CROSSES_BAR:
        title = muse::trc("notation", "Tuplet cannot cross barlines");
        break;
    case MsError::DEST_LOCAL_TIME_SIGNATURE:
        title = muse::trc("notation", "Cannot paste in local time signature");
        break;
    case MsError::DEST_TREMOLO:
        title = muse::trc("notation", "Cannot paste in tremolo");
        break;
    case MsError::NO_MIME:
        title = muse::trc("notation", "Nothing to paste");
        break;
    case MsError::DEST_NO_CR:
        title = muse::trc("notation", "Destination is not a chord or rest");
        break;
    case MsError::CANNOT_CHANGE_LOCAL_TIMESIG_MEASURE_NOT_EMPTY:
        title = muse::trc("notation", "Cannot change local time signature");
        message = muse::trc("notation", "Measure is not empty");
        break;
    case MsError::CANNOT_CHANGE_LOCAL_TIMESIG_HAS_EXCERPTS:
        title = muse::trc("notation", "Cannot change local time signature");
        message = muse::trc("notation", "This score already has part scores. Changing local time "
                                        "signatures while part scores are present is not yet supported.");
        break;
    case MsError::CORRUPTED_MEASURE:
        title = muse::trc("notation", "Cannot change time signature in front of a corrupted measure");
        break;
    case MsError::CANNOT_REMOVE_KEY_SIG:
        title = muse::trc("notation", "This key signature cannot be deleted");
        message = muse::trc("notation", "Please replace it with a key signature from the palettes instead.");
        break;
    case MsError::CANNOT_JOIN_MEASURE_STAFFTYPE_CHANGE:
        title = muse::trc("notation", "These measures cannot be joined");
        message = muse::trc("notation", "Please remove the staff type change and retry.");
        break;
    }

    interactive()->infoAsync(title, message, {}, 0,
                             IInteractive::Option::WithIcon | IInteractive::Option::WithDontShowAgainCheckBox)
    .onResolve(this, [this, err](const IInteractive::Result& res) {
        if (!res.showAgain()) {
            configuration()->setNeedToShowMScoreError(MScore::errorToString(err), false);
        }
    });
}
