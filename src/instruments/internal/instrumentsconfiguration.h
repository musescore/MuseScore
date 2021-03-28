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
#ifndef MU_INSTRUMENTS_INSTRUMENTSCONFIGURATION_H
#define MU_INSTRUMENTS_INSTRUMENTSCONFIGURATION_H

#include "modularity/ioc.h"
#include "iglobalconfiguration.h"
#include "extensions/iextensionsconfiguration.h"
#include "system/ifilesystem.h"

#include "iinstrumentsconfiguration.h"

namespace mu::instruments {
class InstrumentsConfiguration : public IInstrumentsConfiguration
{
    INJECT(instruments, framework::IGlobalConfiguration, globalConfiguration)
    INJECT(instruments, extensions::IExtensionsConfiguration, extensionsConfigurator)
    INJECT(instruments, system::IFileSystem, fileSystem)

public:
    void init();

    io::paths instrumentListPaths() const override;
    async::Notification instrumentListPathsChanged() const override;

    io::paths userInstrumentListPaths() const override;
    void setUserInstrumentListPaths(const io::paths& paths) override;

    io::paths scoreOrderListPaths() const override;
    async::Notification scoreOrderListPathsChanged() const override;

    io::paths userScoreOrderListPaths() const override;
    void setUserScoreOrderListPaths(const io::paths& paths) override;

private:
    io::paths extensionsPaths() const;

    io::path firstInstrumentListPath() const;
    void setFirstInstrumentListPath(const io::path& path);

    io::path secondInstrumentListPath() const;
    void setSecondInstrumentListPath(const io::path& path);

    io::path firstScoreOrderListPath() const;
    void setFirstScoreOrderListPath(const io::path& path);

    io::path secondScoreOrderListPath() const;
    void setSecondScoreOrderListPath(const io::path& path);

    async::Notification m_instrumentListPathsChanged;
    async::Notification m_scoreOrderListPathsChanged;
};
}

#endif // MU_INSTRUMENTS_INSTRUMENTSCONFIGURATION_H
