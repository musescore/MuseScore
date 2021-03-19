//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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
#include "userscoresconfigurationstub.h"

using namespace mu::userscores;
using namespace mu;

ValCh<QStringList> UserScoresConfigurationStub::recentScorePaths() const
{
    return ValCh<QStringList>();
}

void UserScoresConfigurationStub::setRecentScorePaths(const QStringList&)
{
}

io::paths UserScoresConfigurationStub::templatesDirPaths() const
{
    return {};
}

io::path UserScoresConfigurationStub::scoresPath() const
{
    return io::path();
}

io::path UserScoresConfigurationStub::defaultSavingFilePath(const std::string&) const
{
    return io::path();
}

QColor UserScoresConfigurationStub::templatePreviewBackgroundColor() const
{
    return QColor();
}

async::Channel<QColor> UserScoresConfigurationStub::templatePreviewBackgroundColorChanged() const
{
    return async::Channel<QColor>();
}
