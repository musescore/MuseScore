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
#include <stddef.h> //381
#include <list> //304
#include <functional> //219
#include <set> //219
#include <stdint.h> //192
#include <cmath> //155
#include <_string.h> //154
#include <unordered_map> //131
#include <iterator> //115
#include <_stdlib.h> //104
#include <optional> //83
#include <_stdio.h> //76
#include <math.h> //68
#include <string_view> //64
#include <unordered_set> //63
#include <array> //60
#include <__fwd/sstream.h> //52
#include <__hash_table> //50
#include <limits> //47
#include <assert.h> //47
#include <initializer_list> //44
#include <sstream> //37
#include <__fwd/string_view.h> //34
#include <stdlib.h> //33
#include <variant> //29
#include <__math/exponential_functions.h> //28
#include <stdio.h> //27
#include <tuple> //22
#include <cstring> //22
#include <limits.h> //22
#include <regex> //21

// QtCore includes
#include <qstring.h> //928
#include <qtmetamacros.h> //794
#include <qlist.h> //660
#include <qvariant.h> //452
#include <qglobal.h> //439
#include <qobject.h> //382
#include <qcontainerfwd.h> //378
#include <qbytearray.h> //354
#include <qnamespace.h> //310
#include <qmap.h> //292
#include <qpoint.h> //150
#include <qrect.h> //140
#include <qhash.h> //137
#include <qsize.h> //114
#include <qabstractitemmodel.h> //100
#include <qcoreevent.h> //91
#include <qurl.h> //88
#include <qdebug.h> //79
#include <qmetatype.h> //74
#include <qiodevice.h> //70
#include <qtimer.h> //66
#include <qobjectdefs.h> //54
#include <qchar.h> //52
#include <qjsondocument.h> //50
#include <qfile.h> //46
#include <qcoreapplication.h> //43
#include <qtextstream.h> //38
#include <qstringliteral.h> //38
#include <qjsonvalue.h> //36
#include <qjsonobject.h> //36
#include <qdatetime.h> //34
#include <qjsonarray.h> //34
#include <qbuffer.h> //34
#include <qset.h> //32
#include <qpointer.h> //31
#include <qprocess.h> //26
#include <qdir.h> //26
#include <qmimedata.h> //26
#include <qloggingcategory.h> //25
#include <qfileinfo.h> //24
#include <qscopedvaluerollback.h> //22
#include <qeventloop.h> //20
#include <qalgorithms.h> //20

// QtGui includes
#include <qguiapplication.h> //106
#include <qcolor.h> //90
#include <qevent.h> //80
#include <qpixmap.h> //44
#include <qpainter.h> //42
#include <qwindow.h> //40
#include <qfont.h> //30
#include <qicon.h> //22

// QtQml includes
#include <qqml.h> //62
#include <qjsvalue.h> //41
#include <qqmllist.h> //30
#include <qqmlengine.h> //26

// QtQuick includes
#include <qquickitem.h> //106
#include <qquickwindow.h> //20

// QtWidgets includes
#include <qwidget.h> //62
#include <qapplication.h> //50
#include <qdialog.h> //42
#include <qpushbutton.h> //26
#include <qspinbox.h> //24
#include <qradiobutton.h> //22
#include <qcombobox.h> //22
#include <qlabel.h> //20
#include <qcheckbox.h> //20
#include <qdialogbuttonbox.h> //20
