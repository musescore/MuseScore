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
#include "audiosanitizer.h"

#include <thread>

#include "concurrency/taskscheduler.h"

using namespace mu::audio;

static std::thread::id s_as_mainThreadID;
static std::thread::id s_as_workerThreadID;

void AudioSanitizer::setupMainThread()
{
    s_as_mainThreadID = std::this_thread::get_id();
}

std::thread::id AudioSanitizer::mainThread()
{
    return s_as_mainThreadID;
}

bool AudioSanitizer::isMainThread()
{
    return std::this_thread::get_id() == s_as_mainThreadID;
}

void AudioSanitizer::setupWorkerThread()
{
    s_as_workerThreadID = std::this_thread::get_id();
}

std::thread::id AudioSanitizer::workerThread()
{
    return s_as_workerThreadID;
}

bool AudioSanitizer::isWorkerThread()
{
    std::thread::id id = std::this_thread::get_id();

    return TaskScheduler::instance()->containsThread(id) || id == s_as_workerThreadID;
}
