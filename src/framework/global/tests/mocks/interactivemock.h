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
#ifndef MUSE_GLOBAL_INTERACTIVEMOCK_H
#define MUSE_GLOBAL_INTERACTIVEMOCK_H

#include <gmock/gmock.h>

#include "iinteractive.h"

namespace muse {
class InteractiveMock : public IInteractive
{
public:

    MOCK_METHOD(Result, questionSync, (const std::string&, const Text&, const ButtonDatas&, int, const Options&,
                                       const std::string&), (override));

    MOCK_METHOD(async::Promise<Result>, question, (const std::string&, const Text&, const ButtonDatas&, int, const Options&,
                                                   const std::string&), (override));

    MOCK_METHOD(ButtonData, buttonData, (Button), (const, override));

    MOCK_METHOD(Result, infoSync, (const std::string&, const Text&, const ButtonDatas&, int, const Options&,
                                   const std::string&), (override));
    MOCK_METHOD(async::Promise<Result>, info, (const std::string&, const Text&, const ButtonDatas&, int, const Options&,
                                               const std::string&), (override));

    MOCK_METHOD(Result, warningSync, (const std::string&, const Text&, const ButtonDatas&, int, const Options&,
                                      const std::string&), (override));
    MOCK_METHOD(async::Promise<Result>, warning, (const std::string&, const Text&, const ButtonDatas&, int, const Options&,
                                                  const std::string&), (override));

    MOCK_METHOD(Result, errorSync, (const std::string&, const Text&, const ButtonDatas&, int, const Options&,
                                    const std::string&), (override));
    MOCK_METHOD(async::Promise<Result>, error, (const std::string&, const Text&, const ButtonDatas&, int, const Options&,
                                                const std::string&), (override));

    MOCK_METHOD(void, showProgress, (const std::string&, Progress*), (override));

    MOCK_METHOD(async::Promise<io::path_t>, selectOpeningFile, (const std::string& title, const io::path_t& dir,
                                                                const std::vector<std::string>& filter), (override));
    MOCK_METHOD(io::path_t, selectOpeningFileSync, (const std::string&, const io::path_t&, const std::vector<std::string>&), (override));
    MOCK_METHOD(io::path_t, selectSavingFileSync, (const std::string&, const io::path_t&, const std::vector<std::string>&, bool),
                (override));
    MOCK_METHOD(io::path_t, selectDirectory, (const std::string&, const io::path_t&), (override));
    MOCK_METHOD(io::paths_t, selectMultipleDirectories, (const std::string&, const io::path_t&, const io::paths_t&), (override));

    MOCK_METHOD(QColor, selectColor, (const QColor&, const std::string&), (override));
    MOCK_METHOD(bool, isSelectColorOpened, (), (const, override));

    MOCK_METHOD(RetVal<Val>, openSync, (const UriQuery&), (override));
    MOCK_METHOD(async::Promise<Val>, open, (const UriQuery&), (override));
    MOCK_METHOD(RetVal<bool>, isOpened, (const UriQuery&), (const, override));
    MOCK_METHOD(RetVal<bool>, isOpened, (const Uri&), (const, override));
    MOCK_METHOD(async::Channel<Uri>, opened, (), (const, override));

    MOCK_METHOD(void, raise, (const UriQuery&), (override));

    MOCK_METHOD(void, close, (const UriQuery&), (override));
    MOCK_METHOD(void, close, (const Uri&), (override));
    MOCK_METHOD(void, closeAllDialogs, (), (override));

    MOCK_METHOD(ValCh<Uri>, currentUri, (), (const, override));
    MOCK_METHOD(RetVal<bool>, isCurrentUriDialog, (), (const, override));
    MOCK_METHOD(std::vector<Uri>, stack, (), (const, override));

    MOCK_METHOD(Ret, openUrl, (const std::string&), (const, override));
    MOCK_METHOD(Ret, openUrl, (const QUrl&), (const, override));

    MOCK_METHOD(Ret, isAppExists, (const std::string&), (const, override));
    MOCK_METHOD(Ret, canOpenApp, (const Uri&), (const, override));
    MOCK_METHOD(async::Promise<Ret>, openApp, (const Uri&), (const, override));

    MOCK_METHOD(Ret, revealInFileBrowser, (const io::path_t&), (const, override));
};
}

#endif // MUSE_GLOBAL_INTERACTIVEMOCK_H
