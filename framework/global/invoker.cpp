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

void Invoker::invoke()
{
    if (std::this_thread::get_id() == m_mainThreadId) {
        doInvoke();
    } else {
        static const char* name = "doInvoke";
        QMetaObject::invokeMethod(this, name, Qt::QueuedConnection);
    }
}

void Invoker::doInvoke()
{
    if (m_call) {
        m_call();
    }
}

void Invoker::onInvoked(const Call& func)
{
    m_call = func;
}
