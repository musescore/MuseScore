/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "docks_export.h"
#include "flutter/qcoro.h"

namespace KDDockWidgets {
namespace flutter {

class DOCKS_EXPORT CoRoutines
{
public:
    /// Suspends a coroutine
    /// While running the C++ tests we sometimes want to go back to the
    /// dart event loop for some time and then resume the C++
    /// It's up to Dart to return control to C++
    QCoro::Task<> suspend();

    /// Resumes the co-routine
    void resume();
};


}
}
