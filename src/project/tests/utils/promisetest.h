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
    //! A deferred call may queue another one, hence the repetition.
    static void drainDeferredCalls()
    {
        for (int i = 0; i < 10; ++i) {
            muse::async::processMessages();
        }
    }

    //! Subscribes to `promise`, drains the deferred calls and hands back the result.
    template<typename T>
    T await(muse::async::Promise<T> promise)
    {
        T result = T(muse::make_ret(muse::Ret::Code::UnknownError));
        bool resolved = false;
        promise.onResolve(this, [&result, &resolved](const T& value) {
            result = value;
            resolved = true;
        });

        drainDeferredCalls();

        EXPECT_TRUE(resolved) << "the promise did not resolve";
        return result;
    }

    //! A flow that is already over, and resolves with `value`.
    template<typename T>
    static muse::async::Promise<T> resolvedPromise(const T& value)
    {
        return muse::async::make_promise<T>([value](auto resolve) {
            return resolve(value);
        });
    }

    //! A dialog the user answers with `btn`.
    static muse::async::Promise<muse::IInteractive::Result> dialogResult(
        muse::IInteractive::Button btn = muse::IInteractive::Button::Ok)
    {
        return muse::async::make_promise<muse::IInteractive::Result>([btn](auto resolve, auto) {
            return resolve(muse::IInteractive::Result(int(btn)));
        });
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
