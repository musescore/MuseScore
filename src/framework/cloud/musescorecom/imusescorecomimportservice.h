/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#pragma once

#include "modularity/imoduleinterface.h"
#include "async/promise.h"
#include "progress.h"
#include "types/retval.h"

#include "importtypes.h"

class QIODevice;
using DevicePtr = std::shared_ptr<QIODevice>;

namespace muse::cloud {
/// Expected call order for an OMR import:
/// 1. uploadImport() to submit the file(s) and start processing
/// 2. Poll fetchImportQueue() and watch the item's status
/// 3. If the status is AwaitingMeta, get options via fetchSongAutocomplete()/fetchGenres()
///    and submit the choice with submitOmrMeta()
/// 4. Keep polling fetchImportQueue(). If the status is AwaitingReview, use fetchMsczUrl()
///    to let the user review the result, then submit their verdict with submitOmrReview()
/// 5. Keep polling fetchImportQueue() until the status is Done or Failed (an item may also
///    disappear from the queue instead of reporting Done, which should be treated the same way)
/// 6. Once Done, call fetchMsczUrl() to get the final score, then downloadImportedScore()
///
/// Expected call order for an Audio2Score import (no meta/review steps):
/// 1. uploadImport() to submit the file and start processing
/// 2. Poll fetchImportQueue() and watch the item's status
/// 3. Once Done, call fetchMsczUrl() to get the final score, then downloadImportedScore()
class IMuseScoreComImportService : MODULE_CONTEXT_INTERFACE
{
    INTERFACE_ID(IMuseScoreComImportService)

public:
    virtual ~IMuseScoreComImportService() = default;

    virtual ProgressPtr uploadImport(ImportType type, const ImportFileList& files) = 0;
    virtual ProgressPtr downloadImportedScore(const SignedMsczUrl& urlInfo, DevicePtr scoreData) = 0;

    virtual async::Promise<RetVal<ImportQueueList> > fetchImportQueue() = 0;
    virtual async::Promise<RetVal<SignedMsczUrl> > fetchMsczUrl(ImportType type, int id) = 0;

    virtual async::Promise<RetVal<SongAutocompleteList> > fetchSongAutocomplete(const QString& searchText) = 0;
    virtual async::Promise<RetVal<GenreList> > fetchGenres() = 0;

    virtual async::Promise<RetVal<ImportResult> > submitOmrMeta(const OmrMeta& meta) = 0;
    virtual async::Promise<RetVal<ImportResult> > submitOmrReview(int id, OmrReviewRating review, const QString& reason = QString()) = 0;
};
using IMuseScoreComImportServicePtr = std::shared_ptr<IMuseScoreComImportService>;
}
