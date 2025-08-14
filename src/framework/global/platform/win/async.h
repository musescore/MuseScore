/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include <type_traits>

#include <winrt/windows.foundation.h>

#include "global/async/promise.h"
#include "global/types/ret.h"

namespace muse::async {
template<typename T, typename U, typename Fn,
         std::enable_if_t<std::is_convertible_v<std::invoke_result_t<Fn, U>, T>, int> = 0>
Promise<T> toPromise(winrt::Windows::Foundation::IAsyncOperation<U> asyncOp, Fn&& transformResult)
{
    using winrt::Windows::Foundation::IAsyncOperation;
    using winrt::Windows::Foundation::AsyncStatus;

    return Promise<T>([&](auto resolve, auto reject) {
        asyncOp.Completed([=](const IAsyncOperation<U>& sender, const AsyncStatus status) {
            if (status != AsyncStatus::Completed) {
                Ret ret = make_ret(Ret::Code::InternalError);
                (void)reject(ret.code(), ret.text());
                return;
            }

            (void)resolve(transformResult(sender.GetResults()));
        });

        return Promise<T>::Result::unchecked();
    }, PromiseType::AsyncByBody);
}

template<typename T, typename U,
         std::enable_if_t<std::is_constructible_v<T, U>, int> = 0>
Promise<T> toPromise(winrt::Windows::Foundation::IAsyncOperation<U> asyncOp)
{
    return toPromise<T>(asyncOp, [](auto&& u) -> T { return T { std::forward<decltype(u)>(u) }; });
}
}
