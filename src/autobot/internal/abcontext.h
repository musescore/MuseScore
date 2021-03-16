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
#ifndef MU_AUTOBOT_ABCONTEXT_H
#define MU_AUTOBOT_ABCONTEXT_H

#include <any>
#include <map>

#include "ret.h"

namespace mu::autobot {
struct AbContext
{
    enum class Key {
        Undefined = 0,
        ScoreFile,
        ViewZoom,
        CurDrawData,
        RefDrawData,
        DiffDrawData
    };

    Ret ret;

    template<typename T>
    void setVal(const Key& key, const T& v)
    {
        IVal* p = new Val<T>(v);
        m_vals[key] = std::shared_ptr<IVal>(p);
    }

    template<typename T>
    T val(const Key& key, T def = T()) const
    {
        auto it = m_vals.find(key);
        if (it == m_vals.cend()) {
            return def;
        }
        IVal* p = it->second.get();
        Val<T>* d = reinterpret_cast<Val<T>*>(p);
        return d->val;
    }

    void clear()
    {
        m_vals.clear();
        ret = Ret();
    }

private:

    struct IVal {
        virtual ~IVal() = default;
    };

    template<typename T>
    struct Val : public IVal {
        T val;
        Val(const T& v)
            : IVal(), val(v) {}
    };

    std::map<Key, std::shared_ptr<IVal> > m_vals;
};
}
#endif // MU_AUTOBOT_ABCONTEXT_H
