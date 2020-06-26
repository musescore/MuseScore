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
#include "launchertestsmodel.h"
#include "log.h"

using namespace mu::framework;

LauncherTestsModel::LauncherTestsModel(QObject* parent)
    : QObject(parent)
{
}

void LauncherTestsModel::openSampleDialog()
{
    LOGI() << "cpp: before open";
    launcher()->open("musescore://devtools/launcher/sample");
    LOGI() << "cpp: after open";
}

void LauncherTestsModel::openSampleDialogSync()
{
    LOGI() << "cpp: before open";
    launcher()->open("musescore://devtools/launcher/sample?sync=true");
    LOGI() << "cpp: after open";
}
