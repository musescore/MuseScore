//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "audiosanitizer.h"

#include <thread>

using namespace mu::audio;

static std::thread::id s_as_mainThreadID;
static std::thread::id s_as_workerThreadID;

void AudioSanitizer::setupMainThread()
{
    s_as_mainThreadID = std::this_thread::get_id();
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
    return std::this_thread::get_id() == s_as_workerThreadID;
}
