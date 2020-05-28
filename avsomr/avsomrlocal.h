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

#ifndef AVS_AVSOMRLOCAL_H
#define AVS_AVSOMRLOCAL_H

#include <functional>
#include <QString>
#include "ret.h"

namespace Ms {
namespace Avs {
class AvsOmrLocalInstaller;
class AvsOmrLocal
{
public:

    static AvsOmrLocal* instance()
    {
        static AvsOmrLocal l;
        return &l;
    }

    enum class State {
        Undefined = 0,
        NotInstalled,
        Instaling,
        Ready,
        Building
    };

    QString version() const;
    const State& state() const;
    QString stateString() const;
    Ret stateToRet(const State& st) const;

    bool isUseLocal() const;
    bool isInstalled() const;
    void isInstalledAsync(const std::function<void(bool)>& callback) const;
    Ret checkInstallOrUpdate(bool isWait);

    QString avsHomePath() const;
    QString makeAvsFilePath(const QString& buildDir, const QString& baseName) const;

    Ret build(const QString& filePath, const QString& buildDir);

private:

    AvsOmrLocal();
    ~AvsOmrLocal();

    AvsOmrLocalInstaller* installer() const;

    bool isNeedUpdate() const;
    void installBackground();
    void waitInstallOrUpdate();

    void setState(State st);
    Ret execAvs(const QString& cmd, const QString& filePath, const QString& buildDir) const;

    mutable State _state{ State::Undefined };
    mutable AvsOmrLocalInstaller* _installer{ nullptr };
};
} // Avs
} // Ms

#endif // AVS_AVSOMRLOCAL_H
