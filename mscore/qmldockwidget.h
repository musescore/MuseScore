//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer and others
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

#ifndef __QMLDOCKWIDGET_H__
#define __QMLDOCKWIDGET_H__

namespace Ms {

//---------------------------------------------------------
//   FocusChainBreak
//---------------------------------------------------------

class FocusChainBreak : public QQuickItem
      {
      Q_OBJECT

   signals:
      void requestFocusTransfer(bool forward);

   public:
      FocusChainBreak(QQuickItem* parent = nullptr);

      void focusInEvent(QFocusEvent*) override;
      };

//---------------------------------------------------------
//   MsQuickView
//---------------------------------------------------------

class MsQuickView : public QQuickView
      {
      Q_OBJECT

      QWidget* prevFocusWidget = nullptr;

      static void registerQmlTypes();
      void init();

   private slots:
      void transferFocus(bool forward);
      void onStatusChanged(QQuickView::Status);

   public:
      MsQuickView(const QUrl& source, QWindow* parent = nullptr);
      MsQuickView(QQmlEngine* engine, QWindow* parent)
         : QQuickView(engine, parent) { init(); }
      MsQuickView(QWindow* parent = nullptr)
         : QQuickView(parent) { init(); }

      void focusInEvent(QFocusEvent*) override;
      void keyPressEvent(QKeyEvent* e) override;
      };

//---------------------------------------------------------
//   QmlDockWidget
//---------------------------------------------------------

class QmlDockWidget : public QDockWidget
      {
      Q_OBJECT

      QQuickView* _view = nullptr;
      QQmlEngine* engine;

      QQuickView* getView();

   public:
      QmlDockWidget(QQmlEngine* e = nullptr, QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
      QmlDockWidget(QQmlEngine* e, const QString& title, QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

      void setSource(const QUrl& url);
      void source();

      QQmlContext* rootContext() { return getView()->rootContext(); }
      QQuickItem* rootObject() { return getView()->rootObject(); }
      };

}
#endif

