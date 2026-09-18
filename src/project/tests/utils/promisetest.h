/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#pragma once

#include <gmock/gmock.h>

#include "async/asyncable.h"
#include "async/processevents.h"
#include "async/promise.h"
#include "interactive/iinteractive.h"
#include "types/retval.h"
#include "types/val.h"

namespace mu::project {
class PromiseTest : public ::testing::Test, public muse::async::Asyncable
{
protected:
    //! A deferred call may queue another one, and a flow can be many of them long, hence the repetition.
    static void drainDeferredCalls()
    {
        for (int i = 0; i < 100; ++i) {
            muse::async::processMessages();
        }
    }

    //! Subscribes to `promise`, drains the deferred calls and hands back the result.
    template<typename T>
    T await(muse::async::Promise<T> promise)
    {
        //! NOTE Held on the heap: a flow that outlives the drain would otherwise
        //! report back into a stack frame that is already gone
        struct Awaited {
            bool resolved = false;
            T result = T(muse::make_ret(muse::Ret::Code::UnknownError));
        };

        auto awaited = std::make_shared<Awaited>();

        promise.onResolve(this, [awaited](const T& value) {
            awaited->result = value;
            awaited->resolved = true;
        });

        drainDeferredCalls();

        EXPECT_TRUE(awaited->resolved) << "the promise did not resolve";
        return awaited->result;
    }

    //! A flow that is already over, and resolves with `value`.
    template<typename T>
    static muse::async::Promise<T> resolvedPromise(const T& value)
    {
        return muse::async::make_promise<T>([value](auto resolve) {
            return resolve(value);
        });
    }

    //! A flow that is already over, and rejects with `ret`.
    template<typename T>
    static muse::async::Promise<T> rejectedPromise(const muse::Ret& ret)
    {
        return muse::async::make_promise<T>([ret](auto, auto reject) {
            return reject(ret.code(), ret.text());
        });
    }

    //! A dialog the user answers with `btn`.
    static muse::async::Promise<muse::IInteractive::Result> dialogResult(int btn)
    {
        return muse::async::make_promise<muse::IInteractive::Result>([btn](auto resolve, auto) {
            return resolve(muse::IInteractive::Result(btn));
        });
    }

    static muse::async::Promise<muse::IInteractive::Result> dialogResult(
        muse::IInteractive::Button btn = muse::IInteractive::Button::Ok)
    {
        return dialogResult(int(btn));
    }

    //! A dialog that settles the way `answer` says: resolved with its value, or rejected with its error.
    static muse::async::Promise<muse::Val> dialogAnswer(const muse::RetVal<muse::Val>& answer)
    {
        return muse::async::make_promise<muse::Val>([answer](auto resolve, auto reject) {
            if (!answer.ret) {
                return reject(answer.ret.code(), answer.ret.text());
            }

            return resolve(answer.val);
        });
    }

    //! NOTE A flow may hold on to a mock past the test, so verify it here and let it leak.
    template<typename T>
    static void release(const std::shared_ptr<T>& mock)
    {
        ::testing::Mock::VerifyAndClearExpectations(mock.get());
        ::testing::Mock::AllowLeak(mock.get());
    }
};
}
