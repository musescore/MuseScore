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

#include "async/promise.h"
#include "async/channel.h"
#include "io/path.h"
#include "types/retval.h"

#include "types/converttypes.h"

class QUrl;

namespace mu::project {
struct ConvertSelection {
    ConvertInput input;
    QString convertedFileName;
};

class IConvertFileToScoreScenario : MODULE_CONTEXT_INTERFACE
{
    INTERFACE_ID(IConvertFileToScoreScenario)

public:
    virtual ~IConvertFileToScoreScenario() = default;

    //! Server-provided limits (file size, formats, etc) for client-side validation
    virtual const ConvertConfig& config() const = 0;

    //! Checks cloud availability and, if needed, prompts the user to log in
    virtual muse::async::Promise<muse::Ret> checkConvertIsAllowed() = 0;

    //! Opens the file/link picker dialog and resolves with the user's selection
    virtual muse::async::Promise<ConvertSelection> selectFilesToConvert() = 0;

    //! Whether the given file can be converted
    virtual bool isFileSupported(const muse::io::path_t& path) const = 0;

    //! Checks the given files against the server's config limits and determines their convert type and category
    virtual muse::RetVal<ConvertFilesValidation> validateFiles(const muse::io::paths_t& paths) = 0;

    //! Checks that the given link is from a supported source
    virtual muse::Ret validateLink(const QUrl& link) = 0;

    //! Entry point for converting files picked outside the dialog, e.g. via drag-and-drop
    virtual void convertFiles(const muse::io::paths_t& paths) = 0;

    //! Sends the conversion request to the server and shows the processing dialog
    virtual muse::Ret startConvert(const ConvertInput& input, const QString& convertedFileName) = 0;

    //! Emits the result once the server-side conversion completes
    virtual muse::async::Channel<muse::Ret, muse::io::path_t> convertFinished() const = 0;
};

using IConvertFileToScoreScenarioPtr = std::shared_ptr<IConvertFileToScoreScenario>;
}
