/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#ifndef __NOTIFIER_H__
#define __NOTIFIER_H__

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

    Notifier<Data>* _notifier = nullptr;

public:
    Listener() = default;
    Listener(Notifier<Data>* n)
        : _notifier(n) {}
    // do not copy notifier attachment
    Listener(const Listener<Data>&) {}
    Listener(Listener<Data>&&);
    Listener& operator=(const Listener<Data>&) { return *this; }
    Listener& operator=(Listener&&);
    virtual ~Listener();

    void setNotifier(Notifier<Data>* n);

    void detachNotifier(Notifier<Data>* n)
    {
        if (_notifier == n) {
            setNotifier(nullptr);
        }
    }

    Notifier<Data>* notifier() { return _notifier; }
    const Notifier<Data>* notifier() const { return _notifier; }

    virtual void receive(Data d) = 0;

    template<typename T>
    friend void swap(Listener<T>& l1, Listener<T>& l2);
};

//---------------------------------------------------------
//   Notifier
//---------------------------------------------------------

template<typename Data>
class Notifier
{
    std::vector<Listener<Data>*> _listeners;
    bool _atChange = false;

public:
    Notifier() = default;
    // do not copy listeners list
    Notifier(const Notifier<Data>&) {}
    Notifier& operator=(const Notifier<Data>&) { return *this; }
    ~Notifier()
    {
        _atChange = true;     // we don't need to update listeners list anymore
        for (Listener<Data>* l : _listeners) {
            l->detachNotifier(this);
        }
    }

    void addListener(Listener<Data>* l)
    {
        if (_atChange || !l) {
            return;
        }
        _atChange = true;
        _listeners.push_back(l);
        l->setNotifier(this);
        _atChange = false;
    }

    void removeListener(Listener<Data>* l)
    {
        if (_atChange || !l) {
            return;
        }
        _atChange = true;
        _listeners.erase(std::remove(_listeners.begin(), _listeners.end(), l), _listeners.end());
        l->detachNotifier(this);
        _atChange = false;
    }

    void notify(Data d) const
    {
        for (Listener<Data>* l : _listeners) {
            l->receive(d);
        }
    }
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
    if (_notifier) {
        _notifier->removeListener(this);
    }
}

template<typename Data>
void Listener<Data>::setNotifier(Notifier<Data>* n)
{
    if (n == _notifier) {
        return;
    }
    Notifier<Data>* oldNotifier = _notifier;
    _notifier = n;
    if (oldNotifier) {
        oldNotifier->removeListener(this);
    }
    if (_notifier) {
        _notifier->addListener(this);
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
