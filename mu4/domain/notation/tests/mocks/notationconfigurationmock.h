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
#ifndef MU_DOMAIN_NOTATIONCONFIGURATIONMOCK_H
#define MU_DOMAIN_NOTATIONCONFIGURATIONMOCK_H

#include <gmock/gmock.h>

#include "domain/notation/inotationconfiguration.h"

namespace mu {
namespace domain {
namespace notation {
class NotationConfigurationMock : public INotationConfiguration
{
public:
    MOCK_METHOD(QColor, anchorLineColor, (), (const, override));
    MOCK_METHOD(QString, templatesPath, (), (const, override));
    MOCK_METHOD(QString, userTemplatesPath, (), (const, override));
    MOCK_METHOD(QStringList, extensionsTemplatesPaths, (), (const, override));
};
}
}
}

#endif // MU_DOMAIN_NOTATIONCONFIGURATIONMOCK_H
