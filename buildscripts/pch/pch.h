/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#pragma once

/*
This is not `all.h`, you don’t need to put all the includes that are used in the project here.

Here should be only the most frequently used includes.
If you put a few includes here, the build will not speed up much.
If you put a lot of includes here, then the build may also be slower than possible (due to the large .pch file)
So, here should be not few and not many includes :)

It is best to use a tool to determine the most commonly used inclusions.
See https://github.com/musescore/musescore_devtools/tree/main/include-what-you-use
*/

// Std includes
#include <memory> //1222
#include <string> //1188
#include <vector> //1036
#include <utility> //695
#include <algorithm> //472
#include <map> //405
#include <cstddef> //381
#include <list> //304
#include <functional> //219
#include <set> //219
#include <cstdint> //192
#include <cmath> //155+68+28
#include <cstring> //154+22
#include <unordered_map> //131+50
#include <iterator> //115
#include <cstdlib> //104+33
#include <optional> //83
#include <cstdio> //76+27
#include <string_view> //64+34
#include <unordered_set> //63+50
#include <array> //60
#include <sstream> //52+37
#include <limits> //47
#include <cassert> //47
#include <initializer_list> //44
#include <variant> //29
#include <tuple> //22
#include <climits> //22
#include <regex> //21 and via types/string.h

#include <numeric> //via types/types.h(998) -> fraction.h
#include <iostream> //via log.h(591)

#ifndef NO_QT_SUPPORT

// QtCore includes
#include <QObject> //382+794(qtmetamacros.h)+54(qobjectdefs.h)
#include <QString> //928
#include <QList> //660
#include <QVariant> //452
#include <QtGlobal> //439
// #include <qcontainerfwd.h> //378 (no canonical Qt header available, but included via other headers anyway)
#include <QByteArray> //354
#include <Qt> //310
#include <QMap> //292
#include <QPoint> //150
#include <QRect> //140
#include <QHash> //137
#include <QSize> //114
#include <QAbstractItemModel> //100
#include <QEvent> //91
#include <QUrl> //88
#include <QDebug> //79
#include <QMetaType> //74
#include <QIODevice> //70
#include <QTimer> //66
#include <QChar> //52
#include <QJsonDocument> //50
#include <QFile> //46
#include <QCoreApplication> //43
#include <QTextStream> //38
#include <QStringLiteral> //38
#include <QJsonValue> //36
#include <QJsonObject> //36
#include <QDateTime> //34
#include <QJsonArray> //34
#include <QBuffer> //34
#include <QSet> //32
#include <QPointer> //31

#include <QPolygon> //via types/types.h -> geometry.h
#include <QLine>
#include <QPair>

// QtGui includes
#include <QGuiApplication> //106
#include <QColor> //90
#include <QEvent> //80
#include <QPixmap> //44
#include <QPainter> //42
#include <QWindow> //40
#include <QFont> //30

// QtQml includes
#include <QQmlEngine> //26+62(qqml.h)
#include <QJSValue> //41
#include <QQmlListProperty> //30

// QtQuick includes
#include <QQuickItem> //106

// QtWidgets includes
#include <QWidget> //62
#include <QApplication> //50
#include <QDialog> //42

#endif // NO_QT_SUPPORT
