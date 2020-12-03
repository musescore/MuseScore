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
#include "invoker.h"

#include <QMetaObject>

#include "log.h"

using namespace mu::framework;

std::thread::id Invoker::m_mainThreadId;

void Invoker::setup()
{
    m_mainThreadId = std::this_thread::get_id();
}

void Invoker::invoke(const Call& func)
{
    IF_ASSERT_FAILED(func) {
        return;
    }

    if (std::this_thread::get_id() == m_mainThreadId) {
        func();
    } else {
        static const char* name = "doInvoke";

        Functor* f = new Functor(func);
        void* ptr = reinterpret_cast<void*>(f);
        QMetaObject::invokeMethod(this, name, Qt::QueuedConnection, Q_ARG(void*, ptr));
    }
}

void Invoker::doInvoke(void* ptr)
{
    Functor* f = reinterpret_cast<Functor*>(ptr);
    f->call();
    delete f;
}
