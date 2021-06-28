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
#include "multiinstancesstubprovider.h"

using namespace mu::mi;

// Score opening
bool MultiInstancesStubProvider::isScoreAlreadyOpened(const io::path&) const
{
    return false;
}

void MultiInstancesStubProvider::activateWindowWithScore(const io::path&)
{
}

// Settings
bool MultiInstancesStubProvider::isPreferencesAlreadyOpened() const
{
    return false;
}

void MultiInstancesStubProvider::activateWindowWithOpenedPreferences() const
{
}

void MultiInstancesStubProvider::settingsBeginTransaction()
{
}

void MultiInstancesStubProvider::settingsCommitTransaction()
{
}

void MultiInstancesStubProvider::settingsRollbackTransaction()
{
}

void MultiInstancesStubProvider::settingsSetValue(const std::string&, const Val&)
{
}

// Resources (files)
bool MultiInstancesStubProvider::lockResource(const std::string&)
{
    return true;
}

bool MultiInstancesStubProvider::unlockResource(const std::string&)
{
    return true;
}

// Instances info
const std::string& MultiInstancesStubProvider::selfID() const
{
    static std::string id("stub");
    return id;
}

std::vector<InstanceMeta> MultiInstancesStubProvider::instances() const
{
    static std::vector<InstanceMeta> v;
    return v;
}

mu::async::Notification MultiInstancesStubProvider::instancesChanged() const
{
    static mu::async::Notification n;
    return n;
}
