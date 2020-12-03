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
#ifndef MU_FRAMEWORK_INETWORKMANAGER_H
#define MU_FRAMEWORK_INETWORKMANAGER_H

#include "ret.h"
#include "async/channel.h"
#include "global/progress.h"
#include "system/iodevice.h"

class QUrl;

namespace mu::framework {
class INetworkManager
{
public:
    virtual ~INetworkManager() = default;

    virtual Ret get(const QUrl& url, IODevice* incommingData) = 0;
    virtual Ret head(const QUrl& url) = 0;
    virtual Ret post(const QUrl& url, IODevice* outgoingData, IODevice* incommingData) = 0;
    virtual Ret put(const QUrl& url, IODevice* outgoingData, IODevice* incommingData) = 0;
    virtual Ret del(const QUrl& url, IODevice* incommingData) = 0;

    virtual ProgressChannel progressChannel() const = 0;

    virtual void abort() = 0;
};

using INetworkManagerPtr = std::shared_ptr<INetworkManager>;
}

#endif // MU_FRAMEWORK_INETWORKMANAGER_H
