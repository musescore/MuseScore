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

    //! Sends the conversion request to the server
    virtual muse::Ret startConvert(const ConvertInput& input, const muse::String& convertedFileName) = 0;

    //! Emits the result once the server-side conversion completes
    virtual muse::async::Channel<muse::Ret, muse::io::path_t> convertFinished() const = 0;

    //! Names of the files currently being converted server-side (queued, processing, or downloading)
    virtual muse::StringList fileNamesBeingConverted() const = 0;
    virtual muse::async::Notification fileNamesBeingConvertedChanged() const = 0;

    //! Emitted once a converted score is ready and awaiting a quality review
    virtual muse::async::Channel<ConvertType, int /*queueId*/> reviewRequested() const = 0;
    virtual void submitReview(ConvertType type, int queueId, ReviewRating rating, const QString& comment = QString()) = 0;
    virtual void submitReviewComment(ConvertType type, int queueId, const QString& comment) = 0;
};

using IConvertFileToScoreServicePtr = std::shared_ptr<IConvertFileToScoreService>;
}
