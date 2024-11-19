# This file is part of KDDockWidgets.
#
# SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Renato Araujo Oliveira Filho <renato.araujo@kdab.com>
#
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

# pylint: disable=missing-module-docstring,missing-class-docstring,missing-function-docstring

from PySide2 import QtWidgets, QtGui, QtCore

# pylint: disable=too-few-public-methods


class MyWidget(QtWidgets.QWidget):
    s_images = {}

    def __init__(self, backgroundFile="", logoFile="", parent=None):
        super().__init__(parent)

        self.background = self._lookupImage(backgroundFile)
        self.logo = self._lookupImage(logoFile)
        self.setFocusPolicy(QtCore.Qt.StrongFocus)

    # pylint: disable=no-self-use
    def _lookupImage(self, imageName):
        if imageName == "":
            return None

        if imageName not in MyWidget.s_images:
            MyWidget.s_images[imageName] = QtGui.QImage(imageName)

        return MyWidget.s_images[imageName]

    def drawLogo(self, p):

        if not self.logo:
            return

        ratio = self.logo.height() / (self.logo.width() * 1.0)
        maxWidth = int(0.80 * self.size().width())
        maxHeight = int(0.80 * self.size().height())
        proposedHeight = int(maxWidth * ratio)
        if proposedHeight <= maxHeight:
            width = maxWidth
        else:
            width = int(maxHeight / ratio)

        height = int(width * ratio)
        targetLogoRect = QtCore.QRect(0, 0, width, height)
        targetLogoRect.moveCenter(self.rect().center(
        ) + QtCore.QPoint(0, -int(self.size().height() * 0.00)))
        p.drawImage(targetLogoRect, self.logo, self.logo.rect())
