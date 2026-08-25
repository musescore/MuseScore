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
/// Expected call order for an OMR conversion:
/// 1. upload() to submit the file(s) and start processing
/// 2. Poll fetchQueue() and watch the item's status
/// 3. As soon as the status is AwaitingReview or Done, the MSCZ is already
///    available: call fetchMsczUrl() then downloadConvertedScore() to get the score
/// 4. Rating the recognition quality (submitOmrReview(), once AwaitingReview) is optional
///    and does not gate the download above
/// 5. Keep polling fetchQueue() until the status is Failed, or the item disappears
///    from the queue (which should be treated the same as Done)
///
/// Expected call order for an Audio2Score conversion (no meta/review steps):
/// 1. upload() to submit the file and start processing
/// 2. Poll fetchQueue() and watch the item's status
/// 3. Once Done, call fetchMsczUrl() to get the final score, then downloadConvertedScore()
class IMuseScoreComConvertService : MODULE_CONTEXT_INTERFACE
{
    INTERFACE_ID(IMuseScoreComConvertService)

public:
    virtual ~IMuseScoreComConvertService() = default;

    virtual async::Promise<RetVal<ConvertConfig> > fetchConfig() = 0;

    virtual ProgressPtr upload(ConvertType type, const ConvertFileList& files) = 0;
    virtual ProgressPtr downloadConvertedScore(const SignedMsczUrl& urlInfo, DevicePtr scoreData) = 0;

    virtual async::Promise<RetVal<ConvertQueueList> > fetchQueue() = 0;
    virtual async::Promise<RetVal<SignedMsczUrl> > fetchMsczUrl(ConvertType type, int id) = 0;

    virtual async::Promise<RetVal<ConvertResult> > submitOmrReview(int id, OmrReviewRating review, const QString& reason = QString()) = 0;
};
using IMuseScoreComConvertServicePtr = std::shared_ptr<IMuseScoreComConvertService>;
}
