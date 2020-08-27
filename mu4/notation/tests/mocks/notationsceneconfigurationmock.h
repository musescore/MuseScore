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
#ifndef MU_NOTATION_NOTATIONSCENECONFIGURATIONMOCK_H
#define MU_NOTATION_NOTATIONSCENECONFIGURATIONMOCK_H

#include <gmock/gmock.h>
#include "notation/iscenenotationconfiguration.h"

namespace mu {
namespace notation {
class NotationSceneConfigurationMock : public ISceneNotationConfiguration
{
public:
    MOCK_METHOD(QColor, backgroundColor, (), (const, override));
    MOCK_METHOD(async::Channel<QColor>, backgroundColorChanged, (), (const, override));

    MOCK_METHOD(QColor, defaultForegroundColor, (), (const, override));
    MOCK_METHOD(QColor, foregroundColor, (), (const, override));
    MOCK_METHOD(async::Channel<QColor>, foregroundColorChanged, (), (const, override));

    MOCK_METHOD(QColor, playbackCursorColor, (), (const, override));

    MOCK_METHOD(int, selectionProximity, (), (const, override));

    MOCK_METHOD(ValCh<int>, currentZoom, (), (const, override));
    MOCK_METHOD(void, setCurrentZoom, (int), (override));

    MOCK_METHOD(int, fontSize, (), (const, override));
    MOCK_METHOD(QString, stylesDirPath, (), (const, override));
};
}
}

#endif // MU_NOTATION_NOTATIONSCENECONFIGURATIONMOCK_H
