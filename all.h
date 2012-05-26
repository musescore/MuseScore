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
#include <QtGui/QItemDelegate>
#include <QtCore/QModelIndex>
#include <QtGui/QStandardItemModel>
#include <QtGui/QSpinBox>
#include <QtGui/QFormLayout>
#include <QtGui/QDateTimeEdit>
#include <QtGui/QInputDialog>

#include <QtWebKit/QWebView>
#include <QtWebKit/QWebFrame>

#include <QtXml/QtXml>
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

#if QT_VERSION >= 0x040400
#include <QtCore/QAtomicInt>
#endif
#if QT_VERSION >= 0x040700
#include <QtGui/QStaticText>
#endif
// #include <QtGui/QGlyphRun>
#include <QtGui/QPainterPath>
#include <QtGui/QScrollArea>
#include <QtGui/QScrollBar>
#include <QtGui/QToolBar>
#include <QtGui/QWhatsThis>
#include <QtGui/QBitmap>
#include <QtGui/QPixmap>
#include <QtGui/QDockWidget>
#include <QtGui/QStackedWidget>
#include <QtGui/QStackedLayout>
#include <QtGui/QListWidget>
#include <QtGui/QTreeWidget>
#include <QtGui/QMessageBox>
#include <QtGui/QPrinter>
#include <QtGui/QPainter>
#include <QtGui/QComboBox>
#include <QtGui/QAction>
#include <QtGui/QActionGroup>
#include <QtGui/QLayout>
#include <QtGui/QBoxLayout>
#include <QtGui/QMainWindow>
#include <QtGui/QFileDialog>
#include <QtGui/QPrintDialog>
#include <QtGui/QColorDialog>
#include <QtGui/QKeyEvent>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QSplitter>
#include <QtGui/QItemDelegate>
#include <QtGui/QStandardItemModel>
#include <QtGui/QSpinBox>
#include <QtGui/QFontDatabase>
#include <QtGui/QFontComboBox>
#include <QtGui/QApplication>
#include <QtGui/QToolTip>
#include <QtCore/QProcess>
#include <QtGui/QDesktopServices>
#include <QtGui/QTextDocument>
#include <QtGui/QTextDocumentFragment>
#include <QtGui/QTextCursor>
#include <QtGui/QAbstractTextDocumentLayout>
#include <QtGui/QToolBox>
#include <QtGui/QToolButton>
#include <QtGui/QTextBlock>
#include <QtGui/QTextList>
#include <QtGui/QTextFrameFormat>
#include <QtGui/QClipboard>
#include <QtGui/QStatusBar>
#include <QtGui/QSplashScreen>
#include <QtGui/QPushButton>
#include <QtGui/QStylePainter>
#include <QtGui/QStyleOptionButton>
#include <QtGui/QStyleFactory>
#include <QtGui/QWizard>
#include <QtGui/QRadioButton>
#include <QtGui/QFileSystemModel>
#include <QtGui/QHeaderView>
#include <QtGui/QUndoGroup>
#include <QtGui/QUndoStack>
#include <QtGui/QProgressBar>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QCheckBox>
#include <QtGui/QGraphicsView>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsRectItem>
#include <QtGui/QLabel>
#include <QtGui/QFocusFrame>
#include <QtGui/QMouseEventTransition>
#include <QtGui/QCommonStyle>
#include <QtGui/QLineEdit>
#include <QtGui/QGroupBox>
#include <QtGui/QDial>
#include <QtGui/QTextEdit>
#include <QtGui/QMdiSubWindow>
#include <QtGui/QSpacerItem>
#include <QtGui/QGraphicsSceneMouseEvent>

#include <QtSvg/QSvgRenderer>
#include <QtSvg/QSvgGenerator>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkCookieJar>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QUdpSocket>

#if QT_VERSION >= 0x040800
#include <QtNetwork/QHttpPart>
#include <QtNetwork/QHttpMultiPart>
#endif

#include <QtScript/QScriptEngine>
#include <QtScript/QScriptValueIterator>
#include <QtScript/QScriptable>
#include <QtScript/QScriptClass>
#include <QtScript/QScriptClassPropertyIterator>
#if QT_VERSION >= 0x040500
#include <QtScriptTools/QScriptEngineDebugger>
#endif

#endif

