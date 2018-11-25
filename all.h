//=============================================================================
//  MusE
//  Linux Music Score Editor
//
//  Copyright (C) 2004-2011 Werner Schweer (ws@seh.de)
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef __ALLQT_H__
#define __ALLQT_H__

#ifndef NDEBUG
#define ABORTN(n) { static int k = 0; ++k; if (k == n) abort(); }
#else
#define ABORTN(a)
#endif

#if defined __cplusplus

#if (defined (_MSCVER) || defined (_MSC_VER))
   // Define to opt-in to deprecated features (bind2nd, mem_fun) removed in VS2017 c++17 mode.
   #undef _HAS_AUTO_PTR_ETC
   #define _HAS_AUTO_PTR_ETC 1
#endif

#include <stdio.h>
#include <limits.h>
#include <map>
#include <set>
#include <deque>
#include <errno.h>
#include <fcntl.h>
// VStudio does not have <unistd.h>, <io.h> & <process.h> replace many functions from it...
#if (defined (_MSCVER) || defined (_MSC_VER))
   #include <io.h>
   #include <process.h>
#else
   #include <unistd.h>
#endif
#include <math.h>
#include <array>
#include <functional>
#include <memory>

// Disable warning C4127: conditional expression is constant in VS2017 (generated in header file qvector.h)
#if (defined (_MSCVER) || defined (_MSC_VER))
   #pragma warning ( push )
   #pragma warning ( disable: 4127)
#endif

#include <QtGui>
#include <QLoggingCategory>
#include <QModelIndex>

#ifdef QT_WEBENGINE_LIB
// no precompiled QtWebEngine in Qt 5.6 windows gcc
#include <QWebEngineView>
#include <QWebEngineUrlRequestInterceptor>
#include <QWebEngineProfile>
#endif

#include <QtXml>
#include <QAbstractMessageHandler>
#include <QXmlSchema>
#include <QXmlSchemaValidator>
#include <QXmlStreamReader>

#include <QPointF>
#include <QVariant>
#include <QMap>
#include <QByteArray>
#include <QDateTime>
#include <QtGlobal>
#include <QtDebug>
#include <QSharedData>

#include <QAtomicInt>
#include <QErrorMessage>

#include <QPainterPath>
#include <QPixmap>
#include <QPainter>
#include <QKeyEvent>

#include <QFontDatabase>
#include <QProcess>
#include <QDesktopServices>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <QTextCursor>
#include <QAbstractTextDocumentLayout>
#include <QTextBlock>
#include <QTextList>
#include <QClipboard>
#include <QPlainTextEdit>
#include <QStyledItemDelegate>

#include <QDateTimeEdit>
#include <QInputDialog>
#include <QFormLayout>
#include <QItemDelegate>
#include <QStandardItemModel>
#include <QSpinBox>
#include <QScrollArea>
#include <QScrollBar>
#include <QToolBar>
#include <QTreeWidget>
#include <QFileDialog>
#ifdef QT_PRINTSUPPORT_LIB
#include <QPrintDialog>
#include <QPrinter>
#endif
#include <QColorDialog>
#include <QDockWidget>
#include <QStackedWidget>
#include <QStackedLayout>
#include <QListWidget>
#include <QMessageBox>
#include <QComboBox>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QSplitter>
#include <QAction>
#include <QActionGroup>
#include <QLayout>
#include <QBoxLayout>
#include <QStandardItemModel>
#include <QToolTip>
#include <QToolBox>
#include <QToolButton>
#include <QPushButton>
#include <QWizard>
#include <QGroupBox>
#include <QDial>
#include <QTextEdit>
#include <QLineEdit>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QProgressBar>
#include <QProgressDialog>
#include <QRadioButton>
#include <QButtonGroup>
#include <QSplashScreen>
#include <QFontComboBox>
#include <QApplication>
#include <QStatusBar>
#include <QStyle>
#include <QStylePainter>
#include <QStyleOptionButton>
#include <QHeaderView>
#include <QUndoGroup>
#include <QUndoStack>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QLabel>
#include <QFocusFrame>
#include <QMouseEventTransition>
#include <QCommonStyle>
#include <QMdiSubWindow>
#include <QSpacerItem>
#include <QGraphicsSceneMouseEvent>
#include <QtConcurrent>
#include <QScreen>
#include <QGestureEvent>

#include <QSvgRenderer>
#include <QSvgGenerator>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include <QNetworkProxyFactory>
#include <QHostAddress>
#include <QUdpSocket>

#include <QHttpPart>
#include <QHttpMultiPart>

#include <QQmlEngine>
#include <QQmlComponent>
#include <QQuickItem>
#include <QQuickPaintedItem>
#include <QQuickView>
#include <QQuickWidget>

#include <QHelpEngine>
#include <QWidgetAction>
#include <QHelpIndexModel>
#include <QTextBrowser>

#include <QJsonDocument>


// change Q_ASSERT to NOP if not debugging

#ifdef QT_NO_DEBUG
#undef Q_ASSERT_X
#define Q_ASSERT_X(a,b,c)
#undef Q_ASSERT
#define Q_ASSERT(a)
#endif

#if (defined (_MSCVER) || defined (_MSC_VER))
   // Undefined problematic #def'd macros in Microsoft headers
   #undef STRING_NONE
   #undef small
#endif

#endif  // __cplusplus

#endif

