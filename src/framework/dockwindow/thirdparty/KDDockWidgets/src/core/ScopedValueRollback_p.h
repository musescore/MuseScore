/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "KDDockWidgets.h"

class ScopedValueRollback
{
public:
    explicit ScopedValueRollback(bool &var, bool newValue)
        : m_originalValue(var)
        , m_variable(var)
    {
        m_variable = newValue;
    }

    ~ScopedValueRollback()
    {
        m_variable = m_originalValue;
    }

    KDDW_DELETE_COPY_CTOR(ScopedValueRollback)

    const bool m_originalValue;
    bool &m_variable;
};
