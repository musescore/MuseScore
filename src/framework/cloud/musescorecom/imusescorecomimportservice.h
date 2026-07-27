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
/// fetchImportConfig() can be called at any time (no authenticated user required) to get the
/// upload limits (max file size, page/image counts, allowed types) for client-side validation
/// before uploadImport() is called.
///
/// Expected call order for an OMR import:
/// 1. uploadImport() to submit the file(s) and start processing
/// 2. Poll fetchImportQueue() and watch the item's status
/// 3. As soon as the status is AwaitingMeta, AwaitingReview, or Done, the MSCZ is already
///    available: call fetchMsczUrl() then downloadImportedScore() to get the score
/// 4. Submitting meta (submitOmrMeta(), once AwaitingMeta, options via
///    fetchSongAutocomplete()/fetchGenres()) and rating the recognition quality
///    (submitOmrReview(), once AwaitingReview) are both optional and do not gate the
///    download above — they only affect the score's state on musescore.com
/// 5. Keep polling fetchImportQueue() until the status is Failed, or the item disappears
///    from the queue (which should be treated the same as Done)
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

    virtual async::Promise<RetVal<ImportConfig> > fetchImportConfig() = 0;

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
