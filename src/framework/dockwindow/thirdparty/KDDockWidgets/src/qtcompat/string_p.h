/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <string>

/// Dummy QString class so flutter port builds
class QString : public std::string
{
public:
    QString() = default;
    QString(const char *str)
        : std::string(str)
    {
    }

    bool isEmpty() const
    {
        return empty();
    }

#ifndef DARTAGNAN_BINDINGS_RUN
    std::string toStdString() const
    {
        return *this;
    }

    QString &operator+=(const QString &str)
    {
        std::string::operator+=(str);
        return *this;
    }

    bool contains(std::string_view needle) const
    {
        return find(needle) != std::string::npos;
    }

    static QString number(long n)
    {
        return fromStdString(std::to_string(n));
    }
#endif

    static QString fromUtf8(const char *str)
    {
        return QString(str);
    }

    const char *data() const
    {
        return c_str();
    }

    static QString fromStdString(std::string s)
    {
        return *static_cast<QString *>(&s);
    }
};

inline QString operator+(const QString &s1, const QString &s2)
{
    return QString::fromStdString(std::string(s1) + std::string(s2));
}

// Temporary dummy QByteArray to make flutter port build
class QByteArray : public std::string
{
public:
    bool isEmpty() const
    {
        return empty();
    }

    static QByteArray fromStdString(std::string s)
    {
        return *static_cast<QByteArray *>(&s);
    }

    const char *constData() const
    {
        return c_str();
    }
};

namespace std {
template<>
struct hash<QString>
{
    std::size_t operator()(const QString &str) const
    {
        return std::hash<std::string>()(str);
    }
};

}

using QLatin1String = QString;
