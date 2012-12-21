//=============================================================================
//  MusE
//  Linux Music Score Editor
//  $Id: allqt.h,v 1.24 2006/03/02 17:08:30 wschweer Exp $
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

#include <stdio.h>
#include <limits.h>
#include <map>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>

#include <QtGui/QtGui>
#include <QtCore/QModelIndex>

#include <QtWebKit/QWebView>
#include <QtWebKit/QWebFrame>

#include <QtXml/QtXml>
#include <QtXmlPatterns/QAbstractMessageHandler>
#include <QtXmlPatterns/QXmlSchema>
#include <QtXmlPatterns/QXmlSchemaValidator>

#ifdef Q_WS_X11
#include <QtGui/QX11Info>
#endif

#include <QtCore/QPointF>
#include <QtCore/QVariant>
#include <QtCore/QMap>
#include <QtCore/QDateTime>
#include <QtCore/QtGlobal>
#include <QtCore/QtDebug>
#include <QtCore/QSharedData>

#include <QtCore/QAtomicInt>
#include <QtGui/QStaticText>

// #include <QtGui/QGlyphRun>
#include <QtGui/QPainterPath>
#include <QtGui/QBitmap>
#include <QtGui/QPixmap>
#include <QtGui/QPainter>
#include <QtGui/QKeyEvent>

#include <QtGui/QFontDatabase>
#include <QtCore/QProcess>
#include <QtGui/QDesktopServices>
#include <QtGui/QTextDocument>
#include <QtGui/QTextDocumentFragment>
#include <QtGui/QTextCursor>
#include <QtGui/QAbstractTextDocumentLayout>
#include <QtGui/QTextBlock>
#include <QtGui/QTextList>
#include <QtGui/QTextFrameFormat>
#include <QtGui/QClipboard>

#if QT_VERSION >= 0x050000
#include <QtWidgets/QDateTimeEdit>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QItemDelegate>
#include <QtWidgets/QStandardItemModel>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWhatsThis>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QFileDialog>
#include <QtPrintSupport/QPrintDialog>
#include <QtPrintSupport/QPrinter>
#include <QtWidgets/QColorDialog>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStackedLayout>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QAction>
#include <QtWidgets/QActionGroup>
#include <QtWidgets/QLayout>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QStandardItemModel>
#include <QtWidgets/QToolTip>
#include <QtWidgets/QToolBox>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWizard>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QDial>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSplashScreen>
#include <QtWidgets/QFontComboBox>
#include <QtWidgets/QApplication>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QStylePainter>
#include <QtWidgets/QStyleOptionButton>
#include <QtWidgets/QStyleFactory>
#include <QtWidgets/QFileSystemModel>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QUndoGroup>
#include <QtWidgets/QUndoStack>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsRectItem>
#include <QtWidgets/QLabel>
#include <QtWidgets/QFocusFrame>
#include <QtWidgets/QMouseEventTransition>
#include <QtWidgets/QCommonStyle>
#include <QtWidgets/QMdiSubWindow>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QGraphicsSceneMouseEvent>

#else

#include <QtGui/QDateTimeEdit>
#include <QtGui/QInputDialog>
#include <QtGui/QFormLayout>
#include <QtGui/QItemDelegate>
#include <QtGui/QStandardItemModel>
#include <QtGui/QSpinBox>
#include <QtGui/QScrollArea>
#include <QtGui/QScrollBar>
#include <QtGui/QToolBar>
#include <QtGui/QWhatsThis>
#include <QtGui/QTreeWidget>
#include <QtGui/QFileDialog>
#include <QtGui/QPrintDialog>
#include <QtGui/QColorDialog>
#include <QtGui/QDockWidget>
#include <QtGui/QStackedWidget>
#include <QtGui/QStackedLayout>
#include <QtGui/QListWidget>
#include <QtGui/QMessageBox>
#include <QtGui/QPrinter>
#include <QtGui/QComboBox>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QSplitter>
#include <QtGui/QAction>
#include <QtGui/QActionGroup>
#include <QtGui/QLayout>
#include <QtGui/QBoxLayout>
#include <QtGui/QStandardItemModel>
#include <QtGui/QToolTip>
#include <QtGui/QToolBox>
#include <QtGui/QToolButton>
#include <QtGui/QPushButton>
#include <QtGui/QWizard>
#include <QtGui/QGroupBox>
#include <QtGui/QDial>
#include <QtGui/QTextEdit>
#include <QtGui/QLineEdit>
#include <QtGui/QCheckBox>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QProgressBar>
#include <QtGui/QRadioButton>
#include <QtGui/QSplashScreen>
#include <QtGui/QFontComboBox>
#include <QtGui/QApplication>
#include <QtGui/QStatusBar>
#include <QtGui/QStylePainter>
#include <QtGui/QStyleOptionButton>
#include <QtGui/QStyleFactory>
#include <QtGui/QFileSystemModel>
#include <QtGui/QHeaderView>
#include <QtGui/QUndoGroup>
#include <QtGui/QUndoStack>
#include <QtGui/QGraphicsView>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsRectItem>
#include <QtGui/QLabel>
#include <QtGui/QFocusFrame>
#include <QtGui/QMouseEventTransition>
#include <QtGui/QCommonStyle>
#include <QtGui/QMdiSubWindow>
#include <QtGui/QSpacerItem>
#include <QtGui/QGraphicsSceneMouseEvent>
#endif

#include <QtSvg/QSvgRenderer>
#include <QtSvg/QSvgGenerator>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkCookieJar>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QUdpSocket>

#include <QtNetwork/QHttpPart>
#include <QtNetwork/QHttpMultiPart>

#if QT_VERSION >= 0x050000
#include <QtNetwork/QNetworkCookie>
#include <QtConcurrent/QFuture>
#include <QtConcurrent/QFutureWatcher>
#include <QtQuick1/QDeclarativeEngine>
#include <QtQuick1/QDeclarativeComponent>
#include <QtQuick1/QDeclarativeItem>
#include <QtQuick1/QDeclarativeView>
#else
#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/QDeclarativeComponent>
#include <QtDeclarative/QDeclarativeItem>
#include <QtDeclarative/QDeclarativeView>
#endif
#endif

