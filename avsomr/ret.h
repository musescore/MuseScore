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

#ifndef AVS_ERR_H
#define AVS_ERR_H

#include <QString>

namespace Ms {
namespace Avs {
class Ret
{
public:

    enum Code {
        Undefined = -1,
        Ok = 0,
        AvsOmrFirst       = 9000,     // reserved codes less than 9000, avs codes will be 90xx
        // common
        UnknownError,
        FailedReadFile,
        FailedClearDir,
        FileNotSupported,
        // network
        NetworkError      = 9020,
        ServerError,
        // local
        LocalNotInstalled = 9040,
        LocalInstaling,
        LocalFailedExec,
        LocalAlreadyBuilding,

        AvsOmrLast
    };

    Ret();
    Ret(Code c);

    bool valid() const;
    bool success() const;
    Code code() const;

    inline Ret& operator=(Code c) { _code = c; return *this; }
    inline operator bool() const { return success(); }
    inline bool operator!() const { return !success(); }

    QString text() const;
    QString formatedText() const;
    QString supportHint() const;

    static QString text(Code c);
    static QString formatedText(Code c);
    static QString supportHint(Code c);

private:
    Code _code{ Undefined };
};
} // Avs
} // Ms

#endif // AVS_ERR_H
