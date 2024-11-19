# This file is part of KDDockWidgets.
#
# SPDX-FileCopyrightText: 2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Renato Araujo <renato.araujo@kdab.com>
#
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

# pylint: disable=missing-module-docstring,missing-class-docstring,missing-function-docstring

import unittest
import importlib
import inspect

from config import TstConfig


class TestImportModules(unittest.TestCase):
    def test_importModules(self):
        m = importlib.import_module(TstConfig.bindingsNamespace + '.KDDockWidgets')
        moduleSymbols = []
        for t in inspect.getmembers(m):
            moduleSymbols.append(t[0])

        symbols = ['MainWindow', 'DockWidget']
        for symbol in symbols:
            self.assertIn(symbol, moduleSymbols)


if __name__ == '__main__':
    TstConfig.initLibraryPath()
    unittest.main()
