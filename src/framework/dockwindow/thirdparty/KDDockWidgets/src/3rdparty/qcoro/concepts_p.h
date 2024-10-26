// SPDX-FileCopyrightText: 2021 Daniel Vrátil <dvratil@kde.org>
//
// SPDX-License-Identifier: MIT

#pragma once

#ifndef Q_MOC_RUN

#include <concepts>

#if defined(__clang__)

// Sadly, libc++ doesn't currently implement any concepts, so
// we need to implement them ourselves.

namespace QCoro::concepts {

template<typename T>
concept destructible = std::is_nothrow_destructible_v<T>;

template<typename T, typename ... Args>
concept constructible_from = destructible<T>
    && std::is_constructible_v<T, Args...>;

} // namespace QCoro::concepts

#else

namespace QCoro::concepts {

using namespace std;

} // namespace QCoro::concepts

#endif // clang

#endif // Q_MOC_RUN
