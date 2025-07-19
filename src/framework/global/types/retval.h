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
#ifndef MUSE_GLOBAL_RETVAL_H
#define MUSE_GLOBAL_RETVAL_H

#include "ret.h"
#include "../async/channel.h"
#include "../async/notification.h"

namespace muse {
template<typename T>
struct RetVal {
    Ret ret;
    T val;

    RetVal() = default;
    RetVal(const Ret& r)
        : ret(r) {}

    static RetVal<T> make_ok(const T& v)
    {
        RetVal<T> rv;
        rv.ret = muse::make_ret(Ret::Code::Ok);
        rv.val = v;
        return rv;
    }

    static RetVal<T> make_ret(const Ret& ret)
    {
        RetVal<T> rv;
        rv.ret = ret;
        return rv;
    }

    static RetVal<T> make_ret(int code, const std::string& text = "")
    {
        RetVal<T> rv;
        rv.ret = Ret(code, text);
        return rv;
    }
};

template<typename T1, typename T2>
struct RetVal2 {
    Ret ret;
    T1 val1;
    T2 val2;

    static RetVal2<T1, T2> make_ok(const T1& v1, const T2& v2)
    {
        RetVal2<T1, T2> rv;
        rv.ret = muse::make_ret(Ret::Code::Ok);
        rv.val1 = v1;
        rv.val2 = v2;
        return rv;
    }

    static RetVal2<T1, T2> make_ret(const Ret& ret)
    {
        RetVal2<T1, T2> rv;
        rv.ret = ret;
        return rv;
    }

    static RetVal2<T1, T2> make_ret(int code, const std::string& text = "")
    {
        RetVal2<T1, T2> rv;
        rv.ret = Ret(code, text);
        return rv;
    }
};

template<typename T>
struct RetValCh {
    Ret ret;
    T val;
    async::Channel<T> ch;
    RetValCh() = default;
    RetValCh(const Ret& r)
        : ret(r) {}
};

template<typename T>
struct RetCh {
    Ret ret;
    async::Channel<T> ch;
};

template<typename T>
struct ValCh {
    T val = T();
    async::Channel<T> ch;

    void set(const T& v) { val = v; ch.send(v); }
};

template<typename T>
struct ValNt {
    T val = T();
    async::Notification notification;

    void set(const T& v) { val = v; notification.notify(); }
};
}

#endif // MUSE_GLOBAL_RETVAL_H
