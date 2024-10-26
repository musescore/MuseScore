# This file is part of KDDockWidgets.
#
# SPDX-FileCopyrightText: 2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Renato Araujo <renato.araujo@kdab.com>
#
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

import os
import sys

class TstConfig(object):
    bindingsNamespace = "PyKDDockWidgets"

    def initLibraryPath():
        if sys.platform == 'win32' and sys.version_info[0] == 3 and sys.version_info[1] >= 8:
            os.add_dll_directory("@CMAKE_RUNTIME_OUTPUT_DIRECTORY@")
