/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore Limited and others
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

#include "opensaveprojectscenario.h"

#include "cloud/clouderrors.h"
#include "cloud/qml/Muse/Cloud/enums.h"
#include "translation.h"

#include "inotationproject.h"
#include "projecterrors.h"

using namespace muse;
using namespace mu::project;

static std::string cloudStatusCodeErrorMessage(const Ret& ret, bool withHelp = false)
{
    std::string message;

    switch (ret.code()) {
    case int(cloud::Err::Status400_InvalidRequest):
        //: %1 will be replaced with the error code that MuseScore.com returned; this might contain english text
        //: that is deliberately not translated
        message = muse::qtrc("project/cloud", "MuseScore.com returned an error code: %1.")
                  .arg("400 Invalid request").toStdString();
        break;
    case int(cloud::Err::Status401_AuthorizationRequired):
        //: %1 will be replaced with the error code that MuseScore.com returned; this might contain english text
        //: that is deliberately not translated
        message = muse::qtrc("project/cloud", "MuseScore.com returned an error code: %1.")
                  .arg("401 Authorization required").toStdString();
        break;
    case int(cloud::Err::Status422_ValidationFailed):
        //: %1 will be replaced with the error code that MuseScore.com returned; this might contain english text
        //: that is deliberately not translated
        message = muse::qtrc("project/cloud", "MuseScore.com returned an error code: %1.")
                  .arg("422 Validation failed").toStdString();
        break;
    case int(cloud::Err::Status429_RateLimitExceeded):
        //: %1 will be replaced with the error code that MuseScore.com returned; this might contain english text
        //: that is deliberately not translated
        message = muse::qtrc("project/cloud", "MuseScore.com returned an error code: %1.")
                  .arg("429 Rate limit exceeded").toStdString();
        break;
    case int(cloud::Err::Status500_InternalServerError):
        //: %1 will be replaced with the error code that MuseScore.com returned; this might contain english text
        //: that is deliberately not translated
        message = muse::qtrc("project/cloud", "MuseScore.com returned an error code: %1.")
                  .arg("500 Internal server error").toStdString();
        break;
    case int(cloud::Err::UnknownStatusCode): {
        if (const auto status = ret.data<int>("status", -1); status != -1) {
            //: %1 will be replaced with the error code that MuseScore.com returned, which is a number.
            message = muse::qtrc("project/cloud", "MuseScore.com returned an unknown error code: %1.")
                      .arg(status).toStdString();
        } else {
            message = muse::trc("project/cloud", "MuseScore.com returned an unknown error code.");
        }
    } break;
    }

    if (withHelp) {
        message += "\n\n" + muse::trc("project/cloud", "Please try again later, or get help for this problem on MuseScore.com.");
    }

    return message;
}

void OpenSaveProjectScenario::showCloudOpenError(const Ret& ret) const
{
    std::string title = muse::trc("project", "Your score could not be opened");
    std::string message;

    switch (ret.code()) {
    case int(Err::InvalidCloudScoreId):
        message = muse::trc("project", "This score is invalid.");
        break;
    case int(Err::FileOpenError):
        message = muse::trc("project/cloud", "The file could not be downloaded to your disk.");
        break;
    case int(cloud::Err::Status403_AccountNotActivated):
        message = muse::trc("project/cloud", "Your MuseScore.com account needs to be verified first. "
                                             "Please activate your account via the link in the activation email.");
        break;
    case int(cloud::Err::Status403_NotOwner):
        message = muse::trc("project/cloud", "This score does not belong to this account. To access this score, make sure you are logged in "
                                             "to the desktop app with the account to which this score belongs.");
        break;
    case int(cloud::Err::Status404_NotFound):
        message = muse::trc("project/cloud", "The score could not be found, or cannot be accessed by your account.");
        break;

    case int(cloud::Err::Status400_InvalidRequest):
    case int(cloud::Err::Status401_AuthorizationRequired):
    case int(cloud::Err::Status422_ValidationFailed):
    case int(cloud::Err::Status429_RateLimitExceeded):
    case int(cloud::Err::Status500_InternalServerError):
    case int(cloud::Err::UnknownStatusCode):
        message = cloudStatusCodeErrorMessage(ret);
        break;

    case int(cloud::Err::NetworkError):
        message = muse::mtrc("project/cloud", "Could not connect to <a href=\"%1\">MuseScore.com</a>. "
                                              "Please check your internet connection or try again later.")
                  .arg(u"https://musescore.com").toStdString();
        break;
    default:
        message = muse::trc("project/cloud", "Please try again later.");
        break;
    }

    interactive()->warning(title, message);
}
