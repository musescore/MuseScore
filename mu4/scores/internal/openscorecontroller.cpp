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
#include "openscorecontroller.h"

#include "log.h"

using namespace mu::scores;

void OpenScoreController::init()
{
    dispatcher()->reg("domain/notation/file-open", this, &OpenScoreController::openScore);
}

void OpenScoreController::openScore()
{
    QString filePath = interactive()->selectOpeningFile("Score", "", "");
    if (filePath.isEmpty()) {
        return;
    }

    auto notation = notationCreator()->newNotation();
    IF_ASSERT_FAILED(notation) {
        return;
    }

    bool ok = notation->load(filePath.toStdString());
    if (!ok) {
        LOGE() << "failed load: " << filePath;
        //! TODO Show dialog about error
        return;
    }

    m_openedNotations.push_back(notation);

    globalContext()->setCurrentNotation(notation);
}
