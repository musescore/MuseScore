/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "Object_p.h"
#include "Logging_p.h"

using namespace KDDockWidgets::Core;

Object::Object(Object *parent)
{
    setParent(parent);
}

Object::~Object()
{
    if (m_parent)
        m_parent->removeChild(this);

    aboutToBeDeleted.emit();

    const auto children = m_children;
    for (Object *child : children) {
        delete child;
    }
}

void Object::setParent(Object *parent)
{
    if (parent == m_parent)
        return;

    if (m_parent)
        m_parent->removeChild(this);

    m_parent = parent;

    if (parent)
        parent->addChild(this);
}

Object *Object::parent() const
{
    return m_parent;
}

void Object::removeChild(Object *child)
{
    auto it = std::find(m_children.begin(), m_children.end(), child);
    if (it == m_children.cend()) {
        KDDW_ERROR("Object::removeChild: Could not find child");
    } else {
        m_children.erase(it);
    }
}

void Object::addChild(Object *child)
{
    m_children.push_back(child);
}

QString Object::objectName() const
{
    return m_name;
}

void Object::setObjectName(const QString &name)
{
    m_name = name;
}

QString Object::tr(const char *text)
{
    // Not translated for Flutter yet
    return QString(text);
}
