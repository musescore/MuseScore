//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MU_AUDIO_RPCTYPES_H
#define MU_AUDIO_RPCTYPES_H

#include <string>
#include <map>
#include <memory>

namespace mu::audio::rpc {
enum class TargetName {
    Undefined = 0,
    AudioEngine = 1,
    Sequencer = 2,
    DevTools = 11
};

struct Target {
    TargetName name = TargetName::Undefined;
    int instanceID = 0;
    Target() = default;
    Target(TargetName n)
        : name(n) {}

    inline bool operator ==(const Target& other) const { return other.name == name && other.instanceID == instanceID; }
    inline bool operator !=(const Target& other) const { return !this->operator ==(other); }
};

using Method = std::string;

//! NOTE Depending on the implementation of the RPC channel,
//! the data may or may not need to be serialized.
//! For example, if we communicate with Web Worker or a separate process,
//! then in this case it is impossible to transfer objects, structures, pointers and etc - the data must be serialized.
//! If we communicate between threads in the same application, then the data does not need to be serialized.

class Args
{
public:

    template<typename T>
    static Args make_arg1(const T& val)
    {
        Args d;
        d.setArg<T>(0, val);
        return d;
    }

    template<typename T1, typename T2>
    static Args make_arg2(const T1& val1, const T2& val2)
    {
        Args d;
        d.setArg<T1>(0, val1);
        d.setArg<T2>(1, val2);
        return d;
    }

    template<typename T>
    void setArg(int i, const T& val)
    {
        IArg* p = new Arg<T>(val);
        m_args[i] = std::shared_ptr<IArg>(p);
    }

    template<typename T>
    T arg(int i = 0, T def = T()) const
    {
        auto it = m_args.find(i);
        if (it == m_args.cend()) {
            return def;
        }
        IArg* p = it->second.get();
        Arg<T>* d = reinterpret_cast<Arg<T>*>(p);
        return d->val;
    }

    bool hasArg(int i) const
    {
        auto it = m_args.find(i);
        if (it != m_args.cend()) {
            return true;
        }
        return false;
    }

    int count() const
    {
        return int(m_args.size());
    }

    void clear()
    {
        m_args.clear();
    }

    void swap(Args& other)
    {
        m_args.swap(other.m_args);
    }

    struct IArg {
        virtual ~IArg() = default;
    };

    template<typename T>
    struct Arg : public IArg {
        T val;
        Arg(const T& v)
            : IArg(), val(v) {}
    };

private:
    std::map<int, std::shared_ptr<IArg> > m_args;
};

struct Msg {
    Target target;
    Method method;
    Args args;

    Msg() = default;
    Msg(const Target& t, const Method& m)
        : target(t), method(m) {}

    Msg(const Target& t, const Method& m, const Args& a)
        : target(t), method(m), args(a) {}
};
}

#endif // MU_AUDIO_RPCTYPES_H
