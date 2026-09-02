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

#include "converttypes.h"

class QIODevice;
using DevicePtr = std::shared_ptr<QIODevice>;

namespace muse::cloud {
/// fetchConfig() can be called at any time (no authenticated user required) to get the
/// upload limits (max file size, page/image counts, allowed types) for client-side validation
/// before upload() is called.
///
/// Expected call order for a conversion (OMR or Audio2Score):
/// 1. upload() to submit the file(s) and start processing
/// 2. Poll fetchQueue() and watch the item's status
/// 3. As soon as the status is AwaitingReview or Done, the MSCZ is already
///    available: call fetchMsczUrl() then downloadConvertedScore() to get the score
/// 4. Rating the recognition quality (submitReview(), once AwaitingReview) is optional
///    and does not gate the download above; submitReviewComment() may attach a
///    comment afterwards, once the review has been submitted
/// 5. Keep polling fetchQueue() until the status is Failed, or the item disappears
///    from the queue (which should be treated the same as Done)
class IMuseScoreComConvertService : MODULE_CONTEXT_INTERFACE
{
    INTERFACE_ID(IMuseScoreComConvertService)

public:
    virtual ~IMuseScoreComConvertService() = default;

    virtual async::Promise<RetVal<ConvertConfig> > fetchConfig() = 0;

    virtual ProgressPtr upload(const ConvertInput& input) = 0;
    virtual ProgressPtr downloadConvertedScore(const SignedMsczUrl& urlInfo, DevicePtr scoreData) = 0;

    virtual async::Promise<RetVal<ConvertQueueList> > fetchQueue() = 0;
    virtual async::Promise<RetVal<SignedMsczUrl> > fetchMsczUrl(ConvertType type, int id) = 0;

    virtual async::Promise<RetVal<ConvertResult> > submitReview(ConvertType type, int id, ReviewRating review,
                                                                const QString& comment = QString()) = 0;
    virtual async::Promise<RetVal<ConvertResult> > submitReviewComment(ConvertType type, int id, const QString& comment) = 0;
};
using IMuseScoreComConvertServicePtr = std::shared_ptr<IMuseScoreComConvertService>;
}
