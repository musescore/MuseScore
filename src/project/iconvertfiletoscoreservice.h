/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
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

#include "async/channel.h"
#include "async/notification.h"
#include "io/path.h"
#include "progress.h"
#include "types/retval.h"

#include "types/converttypes.h"

class QUrl;

namespace mu::project {
class IConvertFileToScoreService : MODULE_CONTEXT_INTERFACE
{
    INTERFACE_ID(IConvertFileToScoreService)

public:
    virtual ~IConvertFileToScoreService() = default;

    //! Server-provided limits (file size, formats, etc) for client-side validation
    virtual const ConvertConfig& config() const = 0;

    //! Whether the given file can be converted
    virtual bool isFileSupported(const muse::io::path_t& path) const = 0;

    //! Checks the given files against the server's config limits and determines their convert type and category
    virtual muse::RetVal<ConvertFilesValidation> validateFiles(const muse::io::paths_t& paths) const = 0;

    //! Checks that the given link is from a supported source
    virtual muse::Ret validateLink(const QUrl& link) const = 0;

    //! Starts uploading the input to the server, returning the upload's progress
    //! (or an error if the upload couldn't be started, e.g. the files couldn't be read).
    //! The conversion's final result is reported via convertFinished()
    virtual muse::RetVal<muse::Progress> uploadToConvert(const ConvertInput& input, const muse::String& convertedScoreName) = 0;

    //! Emits the final result of a processed conversion (failure, or success with the WatchedScore)
    virtual muse::async::Channel<muse::Ret, WatchedScore> convertFinished() const = 0;

    //! Uploads in progress (not saved, not polled)
    virtual muse::ValNt<UploadingScoreList> uploadingScores() const = 0;

    //! All pending/reviewable conversions from the server's convert queue
    virtual muse::ValNt<WatchedScoreList> watchedScores() const = 0;

    //! Returns a previously converted/watched score by its cloud score id
    virtual const WatchedScore* watchedScoreById(int scoreId) const = 0;

    //! Emitted after every conversion status check, whether it succeeded or failed
    virtual muse::async::Channel<PollingStatus> pollingStatusChanged() const = 0;

    //! Resumes polling for any still-pending items - e.g. in response to the user pressing "Retry"
    virtual void retryPolling() = 0;

    virtual void submitReview(int scoreId, ReviewRating rating, const QString& comment = QString()) = 0;
    virtual void submitReviewComment(int scoreId, const QString& comment) = 0;

    //! Aborts an upload in progress
    virtual void cancelUpload(int uploadId) = 0;

    //! Cancels a watched conversion, removing it both server-side and from watchedScores()
    virtual void cancelConversion(ConvertType type, int convertId) = 0;
};

using IConvertFileToScoreServicePtr = std::shared_ptr<IConvertFileToScoreService>;
}
