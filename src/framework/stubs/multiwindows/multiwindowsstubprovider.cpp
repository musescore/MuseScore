/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#include "multiwindowsstubprovider.h"

using namespace muse;
using namespace muse::mi;

// Project opening

bool MultiWindowsStubProvider::isProjectAlreadyOpened(const io::path_t&) const
{
    return false;
}

void MultiWindowsStubProvider::activateWindowWithProject(const io::path_t&)
{
}

bool MultiWindowsStubProvider::isHasWindowWithoutProject() const
{
    return false;
}

void MultiWindowsStubProvider::activateWindowWithoutProject(const QStringList&)
{
}

bool MultiWindowsStubProvider::openNewWindow(const QStringList&)
{
    return false;
}

// Settings
bool MultiWindowsStubProvider::isPreferencesAlreadyOpened() const
{
    return false;
}

void MultiWindowsStubProvider::activateWindowWithOpenedPreferences() const
{
}

void MultiWindowsStubProvider::settingsBeginTransaction()
{
}

void MultiWindowsStubProvider::settingsCommitTransaction()
{
}

void MultiWindowsStubProvider::settingsRollbackTransaction()
{
}

void MultiWindowsStubProvider::settingsReset()
{
}

void MultiWindowsStubProvider::settingsSetValue(const std::string&, const Val&)
{
}

// Resources (files)
bool MultiWindowsStubProvider::lockResource(const std::string&)
{
    return true;
}

bool MultiWindowsStubProvider::unlockResource(const std::string&)
{
    return true;
}

void MultiWindowsStubProvider::notifyAboutResourceChanged(const std::string&)
{
}

async::Channel<std::string> MultiWindowsStubProvider::resourceChanged()
{
    return async::Channel<std::string>();
}

// Quit for all
void MultiWindowsStubProvider::notifyAboutWindowWasQuited()
{
}

void MultiWindowsStubProvider::quitForAll()
{
}

void MultiWindowsStubProvider::quitAllAndRestartLast()
{
}

void MultiWindowsStubProvider::quitAllAndRunInstallation(const io::path_t&)
{
}
