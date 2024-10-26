/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Waqar Ahmed <waqar.ahmed@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "Logging_p.h"
#include "QtCompat_p.h"

#include <nlohmann/json.hpp>

namespace KDDockWidgets {

inline void to_json(nlohmann::json &j, Size size)
{
    j["width"] = size.width();
    j["height"] = size.height();
}

inline void from_json(const nlohmann::json &j, Size &size)
{
    Size s;
    size.setWidth(j.value("width", s.width()));
    size.setHeight(j.value("height", s.height()));
}

inline void to_json(nlohmann::json &j, Rect rect)
{
    j["x"] = rect.x();
    j["y"] = rect.y();
    j["width"] = rect.width();
    j["height"] = rect.height();
}

inline void from_json(const nlohmann::json &j, Rect &rect)
{
    Rect r;
    rect.setX(j.value("x", r.x()));
    rect.setY(j.value("y", r.y()));
    rect.setWidth(j.value("width", r.width()));
    rect.setHeight(j.value("height", r.height()));
}

}

QT_BEGIN_NAMESPACE

inline void from_json(const nlohmann::json &j, QString &string)
{
    string = QString::fromStdString(j.get<std::string>());
}

inline void to_json(nlohmann::json &j, const QString &s)
{
    j = s.toStdString();
}

inline void from_json(const nlohmann::json &j, KDDockWidgets::Vector<QString> &stringList)
{
    if (!j.is_null() && !j.is_array()) {
        KDDW_ERROR("This is not an array, fix the code");
        stringList.clear();
        return;
    }
    stringList.reserve(( int )j.size());
    for (const auto &v : j) {
        stringList.push_back(v.get<QString>());
    }
}

inline void to_json(nlohmann::json &j, const KDDockWidgets::Vector<QString> &stringList)
{
    for (const auto &s : stringList) {
        j.push_back(s);
    }
}
#ifdef KDDW_FRONTEND_QT

inline void to_json(nlohmann::json &j, QSize size)
{
    KDDockWidgets::to_json(j, size);
}

inline void from_json(const nlohmann::json &j, QSize &size)
{
    KDDockWidgets::from_json(j, size);
}

inline void to_json(nlohmann::json &j, QRect rect)
{
    KDDockWidgets::to_json(j, rect);
}

inline void from_json(const nlohmann::json &j, QRect &rect)
{
    KDDockWidgets::from_json(j, rect);
}

#endif

QT_END_NAMESPACE
