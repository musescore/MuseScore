//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "pluginCreator.h"
#include "musescore.h"
#include "plugins.h"
#include "icons.h"
#include "helpBrowser.h"

extern bool useFactorySettings;

//static const char* states[] = {
//      "S_INIT", "S_EMPTY", "S_CLEAN", "S_DIRTY"
//      };


//---------------------------------------------------------
//   PluginCreator
//---------------------------------------------------------

PluginCreator::PluginCreator(QWidget* parent)
   : QMainWindow(parent)
      {
      state       = S_INIT;
      item        = 0;
      view        = 0;
      dock        = 0;
      manualDock  = 0;
      helpBrowser = 0;

      setupUi(this);

      QToolBar* fileTools = addToolBar(tr("File Operations"));

      actionNew->setIcon(*icons[fileNew_ICON]);
      fileTools->addAction(actionNew);

      actionOpen->setIcon(*icons[fileOpen_ICON]);
      fileTools->addAction(actionOpen);

      actionSave->setIcon(*icons[fileSave_ICON]);
      fileTools->addAction(actionSave);

      actionManual->setIcon(QIcon(":/data/help.png"));
      fileTools->addAction(actionManual);

      textEdit->setTabStopWidth(6);
      log->setReadOnly(true);
      log->setMaximumBlockCount(1000);

      readSettings();
      setState(S_EMPTY);

      connect(run,        SIGNAL(clicked()),    SLOT(runClicked()));
      connect(stop,       SIGNAL(clicked()),    SLOT(stopClicked()));
      connect(actionOpen, SIGNAL(triggered()),  SLOT(loadPlugin()));
      connect(actionSave, SIGNAL(triggered()),  SLOT(savePlugin()));
      connect(actionNew,  SIGNAL(triggered()),  SLOT(newPlugin()));
      connect(actionManual, SIGNAL(triggered()), SLOT(showManual()));
      connect(textEdit,   SIGNAL(textChanged()), SLOT(textChanged()));
      }

//---------------------------------------------------------
//   manualPath
//---------------------------------------------------------

QString PluginCreator::manualPath()
      {
      QString path = mscoreGlobalShare;
      path += "/manual/plugins.html";

      QString p1 = QDir::homePath();                  // hack for debugging
      p1 += "/mscore/share/manual/plugins.html";
      if (QFile(p1).exists())
            return p1;
      return path;
      }

//---------------------------------------------------------
//   setState
//---------------------------------------------------------

void PluginCreator::setState(PCState newState)
      {
      if (state == newState)
            return;
      switch(state) {
            case S_INIT:
                  switch(newState) {
                        case S_INIT:
                              break;
                        case S_EMPTY:
                              setTitle("");
                              actionSave->setEnabled(false);
                              run->setEnabled(false);
                              stop->setEnabled(false);
                              textEdit->setEnabled(false);
                              break;
                        case S_CLEAN:
                        case S_DIRTY:
                              break;
                        }
                  break;
            case S_EMPTY:
                  switch(newState) {
                        case S_INIT:
                        case S_EMPTY:
                              break;
                        case S_CLEAN:
                              setTitle(path);
                              run->setEnabled(true);
                              textEdit->setEnabled(true);
                              break;
                        case S_DIRTY:
                              return;
                        }
                  break;
            case S_CLEAN:
                  switch(newState) {
                        case S_INIT:
                        case S_EMPTY:
                        case S_CLEAN:
                              break;
                        case S_DIRTY:
                              actionSave->setEnabled(true);
                              break;
                        }
                  break;
            case S_DIRTY:
                  switch(newState) {
                        case S_INIT:
                        case S_EMPTY:
                        case S_CLEAN:
                              actionSave->setEnabled(false);
                        case S_DIRTY:
                              break;
                        }
                  break;
            }
      state = newState;
      }

//---------------------------------------------------------
//   setTitle
//---------------------------------------------------------

void PluginCreator::setTitle(const QString& s)
      {
      QString t(tr("MuseScore Plugin Editor"));
      if (s.isEmpty())
            setWindowTitle(t);
      else
            setWindowTitle(t + " - " + s);
      }

//---------------------------------------------------------
//   writeSettings
//---------------------------------------------------------

void PluginCreator::writeSettings()
      {
      QSettings settings;
      settings.beginGroup("PluginCreator");
      settings.setValue("geometry", saveGeometry());
      settings.setValue("windowState", saveState());
      settings.setValue("splitter", splitter->saveState());
      settings.endGroup();
      }

//---------------------------------------------------------
//   readSettings
//---------------------------------------------------------

void PluginCreator::readSettings()
      {
      if (!useFactorySettings) {
            QSettings settings;
            settings.beginGroup("PluginCreator");
            splitter->restoreState(settings.value("splitter").toByteArray());
            restoreGeometry(settings.value("geometry").toByteArray());
            restoreState(settings.value("windowState").toByteArray());
            settings.endGroup();
            }
      }

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void PluginCreator::closeEvent(QCloseEvent* ev)
      {
      if (state == S_DIRTY) {
            QMessageBox::StandardButton n = QMessageBox::warning(this, tr("MuseScore"),
               tr("Plugin \"%1\" has changes\n"
               "save before closing?").arg(path),
               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
               QMessageBox::Save);
            if (n == QMessageBox::Save)
                  savePlugin();
            else if (n == QMessageBox::Cancel) {
                  ev->ignore();
                  return;
                  }
            }
      emit closed();
      QWidget::closeEvent(ev);
      }

//---------------------------------------------------------
//   qmlMsgHandler
//---------------------------------------------------------

static void qmlMsgHandler(QtMsgType type, const char* msg)
      {
      QString s;
      switch(type) {
            case QtDebugMsg:
                  s = QString("Debug: %1\n").arg(msg);
                  break;
            case QtWarningMsg:
                  s = QString("Warning: %1\n").arg(msg);
                  break;
            case QtCriticalMsg:
                  s = QString("Critical: %1\n").arg(msg);
                  break;
            case QtFatalMsg:
                  s = QString("Fatal: %1\n").arg(msg);
                  break;
            }
      mscore->getPluginCreator()->msg(s);
      }

//---------------------------------------------------------
//   runClicked
//---------------------------------------------------------

void PluginCreator::runClicked()
      {
      QDeclarativeEngine* qml = mscore->qml();
      connect(qml, SIGNAL(warnings(const QList<QDeclarativeError>&)),
         SLOT(qmlWarnings(const QList<QDeclarativeError>&)));

      item = 0;
      QDeclarativeComponent component(qml);
      component.setData(textEdit->toPlainText().toUtf8(), QUrl());
      QObject* obj = component.create();
      if (obj == 0) {
            msg("creating component failed\n");
            foreach(QDeclarativeError e, component.errors())
                  msg(QString("   line %1: %2\n").arg(e.line()).arg(e.description()));
            stop->setEnabled(false);
            return;
            }
      qInstallMsgHandler(qmlMsgHandler);
      stop->setEnabled(true);
      run->setEnabled(false);

      item = qobject_cast<QmlPlugin*>(obj);
      view = new QDeclarativeView;
      view->setResizeMode(QDeclarativeView::SizeRootObjectToView);
      QGraphicsObject* go = qobject_cast<QGraphicsObject*>(item);
      view->scene()->addItem(go);

      view->show();

      if (item->pluginType() == "dock") {
            dock = new QDockWidget("Plugin", 0);
            dock->setAttribute(Qt::WA_DeleteOnClose);
            dock->setWidget(view);
            Qt::DockWidgetArea area = Qt::RightDockWidgetArea;
            if (item->dockArea() == "left")
                  area = Qt::LeftDockWidgetArea;
            else if (item->dockArea() == "top")
                  area = Qt::TopDockWidgetArea;
            else if (item->dockArea() == "bottom")
                  area = Qt::BottomDockWidgetArea;
            addDockWidget(area, dock);
            }
      view->raise();
      view->setAttribute(Qt::WA_DeleteOnClose);
      connect(qml,  SIGNAL(quit()),      SLOT(closePlugin()));
      connect(view, SIGNAL(destroyed()), SLOT(closePlugin()));
      if (dock)
            connect(dock, SIGNAL(destroyed()), SLOT(closePlugin()));

      if (mscore->currentScore())
            mscore->currentScore()->startCmd();
      item->runPlugin();
      if (mscore->currentScore())
            mscore->currentScore()->endCmd();
      mscore->endCmd();
      }

//---------------------------------------------------------
//   closePlugin
//---------------------------------------------------------

void PluginCreator::closePlugin()
      {
      stop->setEnabled(false);
      run->setEnabled(true);
      if (view)
            view->close();
      if (dock)
            dock->close();
      qInstallMsgHandler(0);
      }

//---------------------------------------------------------
//   stopClicked
//---------------------------------------------------------

void PluginCreator::stopClicked()
      {
      closePlugin();
      }

//---------------------------------------------------------
//   loadPlugin
//---------------------------------------------------------

void PluginCreator::loadPlugin()
      {
      if (state == S_DIRTY) {
            QMessageBox::StandardButton n = QMessageBox::warning(this, tr("MuseScore"),
               tr("Plugin \"%1\" has changes\n"
               "save before closing?").arg(path),
               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
               QMessageBox::Save);
            if (n == QMessageBox::Save)
                  savePlugin();
            else if (n == QMessageBox::Cancel)
                  return;
            }
      path = mscore->getPluginFilename(true);
      if (path.isEmpty())
            return;
      QFile f(path);
      if (f.open(QIODevice::ReadOnly)) {
            textEdit->setPlainText(f.readAll());
            run->setEnabled(true);
            f.close();
            }
      else {
            path = QString();
            }
      created = false;
      setTitle(path);
      setState(S_CLEAN);
      }

//---------------------------------------------------------
//   savePlugin
//---------------------------------------------------------

void PluginCreator::savePlugin()
      {
      if (created) {
            path = mscore->getPluginFilename(false);
            if (path.isEmpty())
                  return;
            }
      QFile f(path);
      if (f.open(QIODevice::WriteOnly)) {
            f.write(textEdit->toPlainText().toUtf8());
            f.close();
            textEdit->document()->setModified(false);
            created = false;
            setState(S_CLEAN);
            }
      else {
            // TODO
            }
      raise();
      }

//---------------------------------------------------------
//   newPlugin
//---------------------------------------------------------

void PluginCreator::newPlugin()
      {
      if (state == S_DIRTY) {
            QMessageBox::StandardButton n = QMessageBox::warning(this, tr("MuseScore"),
               tr("Plugin \"%1\" has changes\n"
               "save before closing?").arg(path),
               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
               QMessageBox::Save);
            if (n == QMessageBox::Save)
                  savePlugin();
            else if (n == QMessageBox::Cancel)
                  return;
            }
      path    = tr("untitled");
      created = true;
      QString s(
         "import QtQuick 1.0\n"
         "import MuseScore 1.0\n"
         "\n"
         "MuseScore {\n"
         "      menuPath: \"Plugins.pluginName\"\n"
         "      onRun: {\n"
         "            console.log(\"hello world\")\n"
         "            Qt.quit()\n"
         "            }\n"
         "      }\n");
      textEdit->setPlainText(s);
      setState(S_CLEAN);
      raise();
      }

//---------------------------------------------------------
//   textChanged
//---------------------------------------------------------

void PluginCreator::textChanged()
      {
      actionSave->setEnabled(textEdit->document()->isModified());
      setState(S_DIRTY);
      }

//---------------------------------------------------------
//   qmlWarnings
//---------------------------------------------------------

void PluginCreator::qmlWarnings(const QList<QDeclarativeError>& el)
      {
      foreach(const QDeclarativeError& e, el)
            msg(QString("%1:%2: %3\n").arg(e.line()).arg(e.column()).arg(e.description()));
      }

//---------------------------------------------------------
//   msg
//---------------------------------------------------------

void PluginCreator::msg(const QString& s)
      {
      log->moveCursor(QTextCursor::End);
      log->textCursor().insertText(s);
      }

//---------------------------------------------------------
//   showManual
//---------------------------------------------------------

void PluginCreator::showManual()
      {
      if (helpBrowser == 0) {
            helpBrowser = new HelpBrowser;
            manualDock = new QDockWidget("Manual", 0);
            manualDock->setWidget(helpBrowser);
            Qt::DockWidgetArea area = Qt::RightDockWidgetArea;
            addDockWidget(area, manualDock);
            helpBrowser->setContent(manualPath());
            }
      manualDock->show();
      }

