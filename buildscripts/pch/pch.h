/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MU_PCH_H
#define MU_PCH_H

/*
This is not `all.h`, you don’t need to put all the includes that are used in the project here.

Here should be only the most frequently used includes.
If you put a few includes here, the build will not speed up much.
If you put a lot of includes here, then the build may also be slower than possible (due to the large .pch  file)
So, here should be not few and not many includes :)

It is best to use a tool to determine the most commonly used inclusions.
See https://github.com/musescore/musescore_devtools/tree/main/include-what-you-use
*/

// Std includes
#include <memory> //413
#include <vector> //401
#include <algorithm> //274
#include <utility> //267
#include <string> //236
#include <stddef.h> //213
#include <map> //161
#include <stdint.h> //65
#include <string.h> //65
#include <set> //62
#include <list> //61
#include <ostream> //51
#include <cmath> //51
#include <stdio.h> //47
#include <iterator> //40
#include <functional> //38
#include <type_traits> //30
#include <math.h> //30
#include <stdlib.h> //30
#include <limits> //28
#include <initializer_list> //28
#include <array> //25
#include <errno.h> //20

// Qt includes
#include <QString> //771
#include <QList> //465
#include <QVariant> //376
#include <QByteArray> //257
#include <QPointF> //249
#include <QRectF> //191
#include <QStringList> //164
#include <QObject> //152
#include <QVector> //143
#include <QPainter> //139
#include <QColor> //126
#include <QFlags> //99
#include <QIODevice> //92
#include <QPen> //90
#include <QMap> //88
#include <QFile> //85
#include <QLineF> //67
#include <QChar> //66
#include <QApplication> //61
#include <QWidget> //52
#include <QVariantMap> //51
#include <QModelIndex> //41
#include <QFont> //40
#include <QFileInfo> //40
#include <QSizeF> //39
#include <QSet> //39
#include <QRect> //38
#include <QDebug> //35
#include <QSize> //34
#include <QUrl> //32
#include <QTransform> //30
#include <QDialog> //27
#include <QXmlStreamReader> //26
#include <QVariantList> //25
#include <QMetaType> //25
#include <QFontMetricsF> //24
#include <QMultiMap> //22
#include <QPushButton> //22
#include <QPixmap> //22
#include <QXmlStreamAttributes> //22

#endif //MU_PCH_H
