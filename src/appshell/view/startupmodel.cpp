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

#include "startupmodel.h"

using namespace mu::appshell;
using namespace mu::actions;

static const std::string HOME_URI("musescore://home");
static const std::string NOTATION_URI("musescore://notation");
static const std::string NEW_SCORE_URI("musescore://userscores/newscore");

StartupModel::StartupModel(QObject* parent)
    : QObject(parent)
{
}

std::string StartupModel::startupPageUri() const
{
    switch (configuration()->startupSessionType()) {
    case StartupSessionType::StartEmpty:
    case StartupSessionType::StartWithNewScore:
        return HOME_URI;
    case StartupSessionType::ContinueLastSession:
    case StartupSessionType::StartWithScore:
        return NOTATION_URI;
    }

    return HOME_URI;
}

void StartupModel::load()
{
    interactive()->open(startupPageUri());

    switch (configuration()->startupSessionType()) {
    case StartupSessionType::StartEmpty:
        break;
    case StartupSessionType::StartWithNewScore:
        interactive()->open(NEW_SCORE_URI);
        break;
    case StartupSessionType::ContinueLastSession:
        dispatcher()->dispatch("continue-last-session");
        break;
    case StartupSessionType::StartWithScore: {
        io::path scorePath = configuration()->startupScorePath();
        dispatcher()->dispatch("file-open", ActionData::make_arg1<io::path>(scorePath));
    } break;
    }
}
