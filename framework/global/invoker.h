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
#ifndef MU_FRAMEWORK_INVOKER_H
#define MU_FRAMEWORK_INVOKER_H

#include <QObject>
#include <thread>
#include <functional>

namespace mu {
namespace framework {
class Invoker : public QObject
{
    Q_OBJECT
public:
    Invoker() = default;

    using Call = std::function<void ()>;

    static void setup();

    void invoke(const Call& func = nullptr);

public slots:
    void doInvoke(void* ptr);

private:

    struct Functor {
        Call call;
        Functor(const Call& c)
            : call(c) {}
    };

    static std::thread::id m_mainThreadId;
};
}
}

#endif // MU_FRAMEWORK_INVOKER_H
