# This file is part of KDDockWidgets.
#
# SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Renato Araujo Oliveira Filho <renato.araujo@kdab.com>
#
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

''' KDDockWidgets example (Qt5) '''

import sys

from PySide2 import QtWidgets, QtCore
# pylint: disable=no-name-in-module
from PyKDDockWidgets import KDDockWidgets

from MyWidget import MyWidget

try:
    # pylint: disable=unused-import
    import rc_assets
except ImportError:
    sys.exit(
        '''
Oops.. rc_assets needs to be generated first.
Please run:
 rcc -g python -o rc_assets.py ../../examples/dockwidgets/resources_example.qrc
 (Make sure to use the rcc from the Qt5 version used to generate the bindings!)
 On some systems rcc might be invoked as rcc-qt5.
'''
    )

if __name__ == "__main__":
    QtWidgets.QApplication.setAttribute(QtCore.Qt.AA_EnableHighDpiScaling)
    QtWidgets.QApplication.setAttribute(QtCore.Qt.AA_UseHighDpiPixmaps)
    app = QtWidgets.QApplication(sys.argv)

    app.setOrganizationName("KDAB")
    app.setApplicationName("Test app")

    KDDockWidgets.initFrontend(KDDockWidgets.FrontendType.QtWidgets)
    app.setStyle(QtWidgets.QStyleFactory.create("Fusion"))

    # 1. Create our main window
    mainWindow = KDDockWidgets.MainWindow("MyMainWindow")
    mainWindow.setWindowTitle("Main Window")
    mainWindow.resize(1200, 1200)
    mainWindow.show()

    # 2. Create a dock widget, it needs a unique name
    dock1 = KDDockWidgets.DockWidget("MyDock1")
    widget1 = MyWidget("", "")
    dock1.setWidget(widget1)

    dock2 = KDDockWidgets.DockWidget("MyDock2")
    widget2 = MyWidget(":/assets/base.png", ":/assets/KDAB_bubble_fulcolor.png")
    dock2.setWidget(widget2)

    dock3 = KDDockWidgets.DockWidget("MyDock3")
    widget3 = MyWidget(":/assets/base.png", ":/assets/KDAB_bubble_fulcolor.png")
    dock3.setWidget(widget3)

    dock4 = KDDockWidgets.DockWidget("MyDock4")
    widget4 = MyWidget(":/assets/base.png", ":/assets/KDAB_bubble_fulcolor.png")
    dock4.setWidget(widget4)

    dock5 = KDDockWidgets.DockWidget("MyDock5")
    widget5 = MyWidget(":/assets/base.png", ":/assets/KDAB_bubble_fulcolor.png")
    dock5.setWidget(widget5)

    # 3. Add them to the main window
    mainWindow.addKDockWidget(dock1, KDDockWidgets.Location.Location_OnLeft)
    mainWindow.addKDockWidget(dock2, KDDockWidgets.Location.Location_OnTop)

    # 4. Add dock3 to the right of dock2
    mainWindow.addKDockWidget(dock3, KDDockWidgets.Location.Location_OnRight, dock2)

    # 5. dock4 is docked at the bottom, with 200px height
    preferredSize = QtCore.QSize(0, 200)
    mainWindow.addKDockWidget(dock4, KDDockWidgets.Location.Location_OnBottom, None, preferredSize)

    # 5. dock5 will be its own top level (floating window)
    dock5.open()

    app.exec_()
