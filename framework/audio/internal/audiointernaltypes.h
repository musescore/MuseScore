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
#ifndef MU_AUDIO_AUDIOINTERNALTYPES_H
#define MU_AUDIO_AUDIOINTERNALTYPES_H

#include <memory>
#include <map>

namespace mu {
namespace audio {
// === Args ===
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
}
}

#endif // MU_AUDIO_AUDIOINTERNALTYPES_H
