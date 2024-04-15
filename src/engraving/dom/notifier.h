/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MU_ENGRAVING_NOTIFIER_H
#define MU_ENGRAVING_NOTIFIER_H

#include "global/allocator.h"

namespace mu::engraving {
template<typename Data> class Notifier;

//---------------------------------------------------------
//   Listener
//---------------------------------------------------------

template<typename Data>
class Listener
{
    OBJECT_ALLOCATOR(engraving, Listener)

public:
    Listener() = default;
    Listener(Notifier<Data>* n)
        : m_notifier(n) {}
    // do not copy notifier attachment
    Listener(const Listener<Data>&) {}
    Listener(Listener<Data>&&);
    Listener& operator=(const Listener<Data>&) { return *this; }
    Listener& operator=(Listener&&);
    virtual ~Listener();

    void setNotifier(Notifier<Data>* n);

    void detachNotifier(Notifier<Data>* n)
    {
        if (m_notifier == n) {
            setNotifier(nullptr);
        }
    }

    Notifier<Data>* notifier() { return m_notifier; }
    const Notifier<Data>* notifier() const { return m_notifier; }

    virtual void receive(Data d) = 0;

    template<typename T>
    friend void swap(Listener<T>& l1, Listener<T>& l2);

private:
    Notifier<Data>* m_notifier = nullptr;
};

//---------------------------------------------------------
//   Notifier
//---------------------------------------------------------

template<typename Data>
class Notifier
{
public:
    Notifier() = default;
    // do not copy listeners list
    Notifier(const Notifier<Data>&) {}
    Notifier& operator=(const Notifier<Data>&) { return *this; }
    ~Notifier()
    {
        m_atChange = true;     // we don't need to update listeners list anymore
        for (Listener<Data>* l : m_listeners) {
            l->detachNotifier(this);
        }
    }

    void addListener(Listener<Data>* l)
    {
        if (m_atChange || !l) {
            return;
        }
        m_atChange = true;
        m_listeners.push_back(l);
        l->setNotifier(this);
        m_atChange = false;
    }

    void removeListener(Listener<Data>* l)
    {
        if (m_atChange || !l) {
            return;
        }
        m_atChange = true;
        m_listeners.erase(std::remove(m_listeners.begin(), m_listeners.end(), l), m_listeners.end());
        l->detachNotifier(this);
        m_atChange = false;
    }

    void notify(Data d) const
    {
        for (Listener<Data>* l : m_listeners) {
            l->receive(d);
        }
    }

private:

    std::vector<Listener<Data>*> m_listeners;
    bool m_atChange = false;
};

template<typename Data>
Listener<Data>::Listener(Listener<Data>&& other)
{
    if (Notifier<Data>* n = other.notifier()) {
        n->removeListener(other);
        setNotifier(n);
    }
}

template<typename Data>
Listener<Data>& Listener<Data>::operator=(Listener<Data>&& other)
{
    if (Notifier<Data>* n = other.notifier()) {
        n->removeListener(other);
        setNotifier(n);
    } else {
        setNotifier(nullptr);
    }

    return *this;
}

template<typename Data>
Listener<Data>::~Listener()
{
    if (m_notifier) {
        m_notifier->removeListener(this);
    }
}

template<typename Data>
void Listener<Data>::setNotifier(Notifier<Data>* n)
{
    if (n == m_notifier) {
        return;
    }
    Notifier<Data>* oldNotifier = m_notifier;
    m_notifier = n;
    if (oldNotifier) {
        oldNotifier->removeListener(this);
    }
    if (m_notifier) {
        m_notifier->addListener(this);
    }
}

template<typename Data>
void swap(Listener<Data>& l1, Listener<Data>& l2)
{
    Notifier<Data>* n1 = l1.notifier();
    Notifier<Data>* n2 = l2.notifier();
    l1.setNotifier(n2);
    l2.setNotifier(n1);
}
} // namespace mu::engraving
#endif
