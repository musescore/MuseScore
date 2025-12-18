/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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
#include "appupdateservicestub.h"

using namespace muse;
using namespace muse::update;
using namespace muse::async;

Promise<RetVal<ReleaseInfo> > AppUpdateServiceStub::checkForUpdate()
{
    return Promise<RetVal<ReleaseInfo> >([](auto resolve, auto) {
        return resolve(make_ret(Ret::Code::NotSupported));
    });
}

const RetVal<ReleaseInfo>& AppUpdateServiceStub::lastCheckResult() const
{
    static const auto dummyInfo = RetVal<ReleaseInfo>::make_ret(Ret::Code::NotSupported);
    return dummyInfo;
}

RetVal<Progress> AppUpdateServiceStub::downloadRelease()
{
    return RetVal<Progress>::make_ret(Ret::Code::NotSupported);
}
