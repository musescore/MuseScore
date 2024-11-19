/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "KDDockWidgets.h"
#include "QtCompat_p.h"

#include <iostream>

/// Logging is done via spdlog.
/// If spdlog isn't available, then no logging is done, except if it's an error (>= level::err), in which
/// case we fallback to qWarning().
/// But preferably, please compile with spdlog support, as the formatting will be nicer.

#ifdef KDDW_HAS_SPDLOG

#include "spdlog_formatters_p.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#define KDDW_LOG(level, ...)                                                     \
    if (spdlog::should_log(level)) {                                             \
        auto logger = spdlog::get(KDDockWidgets::spdlogLoggerName());            \
        if (!logger) {                                                           \
            logger = spdlog::stdout_color_mt(KDDockWidgets::spdlogLoggerName()); \
        }                                                                        \
        if (logger->should_log(level)) {                                         \
            logger->log(level, __VA_ARGS__);                                     \
        }                                                                        \
    }

#define KDDW_ERROR(...) KDDW_LOG(spdlog::level::err, __VA_ARGS__)
#define KDDW_WARN(...) KDDW_LOG(spdlog::level::warn, __VA_ARGS__)
#define KDDW_INFO(...) KDDW_LOG(spdlog::level::info, __VA_ARGS__)
#define KDDW_DEBUG(...) KDDW_LOG(spdlog::level::debug, __VA_ARGS__)
#define KDDW_TRACE(...) KDDW_LOG(spdlog::level::trace, __VA_ARGS__)

#else

#define KDDW_WARN(...) (( void )0)
#define KDDW_INFO(...) (( void )0)
#define KDDW_DEBUG(...) (( void )0)
#define KDDW_TRACE(...) (( void )0)

#ifdef KDDW_FRONTEND_QT

#define KDDW_ERROR(...) printQWarning(__VA_ARGS__)

#include <QDebug>

// KDDW was built without spdlog support.
// trace/debug logging will be disabled, but let's have a fallback for important errors.
// spdlog::error() will be transformed into qWarning.

QT_BEGIN_NAMESPACE

inline QDebug operator<<(QDebug d, KDDockWidgets::InitialOption o)
{
    d << o.startsHidden();
    return d;
}

QT_END_NAMESPACE

template<typename Arg>
void printQWarningArg(Arg &&arg, QDebug &stream)
{
    stream << std::forward<Arg>(arg);
}

template<typename First, typename... Args>
void printQWarningArg(First &&first, Args &&...args, QDebug &stream)
{
    stream << std::forward<First>(first);
    printQWarningArg(std::forward<Args>(args)..., stream);
}

template<typename... Args>
void printQWarning(Args &&...args)
{
    auto stream = qWarning();
    (printQWarningArg(std::forward<Args>(args), stream), ...);
}

#else

// Flutter without spdlog is a no-op
#define KDDW_ERROR(...) (( void )0)

#endif

#endif

template<typename CharT, typename Traits>
std::basic_ostream<CharT, Traits> &operator<<(std::basic_ostream<CharT, Traits> &os, KDDockWidgets::Size size)
{
    os << "Size(" << size.width() << ", " << size.height() << ")";
    return os;
}

template<typename CharT, typename Traits>
std::basic_ostream<CharT, Traits> &operator<<(std::basic_ostream<CharT, Traits> &os, KDDockWidgets::Rect rect)
{
    os << "Rect(" << rect.x() << "," << rect.y() << " " << rect.width() << "x" << rect.height() << ")";
    return os;
}

template<typename CharT, typename Traits>
std::basic_ostream<CharT, Traits> &operator<<(std::basic_ostream<CharT, Traits> &os, const KDDockWidgets::Vector<double> &vec)
{
    os << "{ ";
    for (double v : vec) {
        os << v << ", ";
    }
    os << " }";
    return os;
}
