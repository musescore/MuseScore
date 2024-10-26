/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "KDDockWidgets.h"
#include "QtCompat_p.h"

#include <spdlog/spdlog.h>

template<>
struct fmt::formatter<KDDockWidgets::Size>
{
    constexpr auto parse(format_parse_context &ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(KDDockWidgets::Size size, FormatContext &ctx) const
    {
        return fmt::format_to(ctx.out(), "{}x{}", size.width(), size.height());
    }
};

template<>
struct fmt::formatter<KDDockWidgets::Point>
{
    constexpr auto parse(format_parse_context &ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(KDDockWidgets::Point point, FormatContext &ctx) const
    {
        return fmt::format_to(ctx.out(), "{}x{}", point.x(), point.y());
    }
};

template<>
struct fmt::formatter<KDDockWidgets::Rect>
{
    constexpr auto parse(format_parse_context &ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(KDDockWidgets::Rect r, FormatContext &ctx) const
    {
        return fmt::format_to(ctx.out(), "Rect({},{} {}x{})", r.x(), r.y(), r.width(), r.height());
    }
};


template<>
struct fmt::formatter<QString>
{
    constexpr auto parse(format_parse_context &ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const QString &str, FormatContext &ctx) const
    {
        return fmt::format_to(ctx.out(), "{}", str.toStdString());
    }
};

#ifdef KDDW_FRONTEND_QT
template<typename T>
struct fmt::formatter<QVector<T>>
{
    constexpr auto parse(format_parse_context &ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const QVector<T> &vec, FormatContext &ctx) const
    {

        auto out = ctx.out();
        out = fmt::format_to(out, "{}", "{ ");
        for (const auto &element : vec)
            out = fmt::format_to(out, "{}, ", element);
        out = fmt::format_to(out, "{}", " }");

        return out;
    }
};

template<typename F>
struct fmt::formatter<QFlags<F>>
{
    constexpr auto parse(format_parse_context &ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(QFlags<F> flags, FormatContext &ctx) const
    {

        auto out = ctx.out();
        out = fmt::format_to(out, "{}", ( int )flags);
        return out;
    }
};

#endif

// using enable_if_t = typename std::enable_if<B, T>::type;


namespace KDDockWidgets {
template<typename Enum, typename std::enable_if<std::is_enum<Enum>::value, int>::type = 0>
constexpr auto format_as(Enum e) noexcept -> typename std::underlying_type<Enum>::type
{
    return static_cast<typename std::underlying_type<Enum>::type>(e);
}
namespace Core {
template<typename Enum, fmt::enable_if_t<std::is_enum<Enum>::value, int> = 0>
constexpr auto format_as(Enum e) noexcept -> typename std::underlying_type<Enum>::type
{
    return static_cast<typename std::underlying_type<Enum>::type>(e);
}
}
}

template<>
struct fmt::formatter<Qt::Orientation>
{
    constexpr auto parse(format_parse_context &ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(Qt::Orientation o, FormatContext &ctx) const
    {
        if (o == Qt::Horizontal) {
            return fmt::format_to(ctx.out(), "Horizontal");
        } else if (o == Qt::Vertical) {
            return fmt::format_to(ctx.out(), "Vertical");
        } else {
            return fmt::format_to(ctx.out(), "InvalidOrientation!");
        }
    }
};

template<>
struct fmt::formatter<KDDockWidgets::DropLocation>
{
    constexpr auto parse(format_parse_context &ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(KDDockWidgets::DropLocation loc, FormatContext &ctx) const
    {

        switch (loc) {
        case KDDockWidgets::DropLocation_None:
            return fmt::format_to(ctx.out(), "DropLocation_None");
        case KDDockWidgets::DropLocation_Left:
            return fmt::format_to(ctx.out(), "DropLocation_Left");
        case KDDockWidgets::DropLocation_Top:
            return fmt::format_to(ctx.out(), "DropLocation_Top");
        case KDDockWidgets::DropLocation_Right:
            return fmt::format_to(ctx.out(), "DropLocation_Right");
        case KDDockWidgets::DropLocation_Bottom:
            return fmt::format_to(ctx.out(), "DropLocation_Bottom");
        case KDDockWidgets::DropLocation_Center:
            return fmt::format_to(ctx.out(), "DropLocation_Center");
        case KDDockWidgets::DropLocation_OutterLeft:
            return fmt::format_to(ctx.out(), "DropLocation_OutterLeft");
        case KDDockWidgets::DropLocation_OutterTop:
            return fmt::format_to(ctx.out(), "DropLocation_OutterTop");
        case KDDockWidgets::DropLocation_OutterRight:
            return fmt::format_to(ctx.out(), "DropLocation_OutterRight");
        case KDDockWidgets::DropLocation_OutterBottom:
            return fmt::format_to(ctx.out(), "DropLocation_OutterBottom");
        default:
            break;
        }

        return fmt::format_to(ctx.out(), "{}", ( int )loc);
    }
};

template<>
struct fmt::formatter<KDDockWidgets::InitialOption>
{
    constexpr auto parse(format_parse_context &ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const KDDockWidgets::InitialOption &opt, FormatContext &ctx) const
    {
        return fmt::format_to(ctx.out(), "[InitialOption: preferredSize={}, visibility={}]", opt.preferredSize, ( int )opt.visibility);
    }
};

#ifndef KDDW_FRONTEND_QT
template<typename T>
struct fmt::formatter<KDDockWidgets::Vector<T>>
{
    constexpr auto parse(format_parse_context &ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const KDDockWidgets::Vector<T> &vec, FormatContext &ctx) const
    {

        auto out = ctx.out();
        out = fmt::format_to(out, "{}", "{ ");
        for (const auto &element : vec)
            out = fmt::format_to(out, "{}, ", element);
        out = fmt::format_to(out, "{}", " }");

        return out;
    }
};

#endif
