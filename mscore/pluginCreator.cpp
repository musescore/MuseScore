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
#include "qmlplugin.h"
#include "icons.h"
#include "helpBrowser.h"
#include "preferences.h"
#include "libmscore/score.h"

namespace Ms {

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
      state       = PCState::INIT;
      item        = 0;
      view        = 0;
      dock        = 0;
      manualDock  = 0;
      helpBrowser = 0;

      setObjectName("PluginCreator");
      setIconSize(QSize(preferences.iconWidth * guiScaling, preferences.iconHeight * guiScaling));

      setupUi(this);

      QToolBar* fileTools = addToolBar(tr("File Operations"));
      fileTools->setObjectName("FileOperations");

      actionNew->setIcon(*icons[int(Icons::fileNew_ICON)]);
      actionNew->setShortcut(QKeySequence(QKeySequence::New));
      fileTools->addAction(actionNew);

      actionOpen->setIcon(*icons[int(Icons::fileOpen_ICON)]);
      actionOpen->setShortcut(QKeySequence(QKeySequence::Open));
      fileTools->addAction(actionOpen);

      actionReload->setIcon(*icons[int(Icons::viewRefresh_ICON)]);
      fileTools->addAction(actionReload);

      actionSave->setIcon(*icons[int(Icons::fileSave_ICON)]);
      actionSave->setShortcut(QKeySequence(QKeySequence::Save));
      fileTools->addAction(actionSave);

      actionQuit->setShortcut(QKeySequence(QKeySequence::Close));

      actionManual->setIcon(QIcon(*icons[int(Icons::helpContents_ICON)]));
      actionManual->setShortcut(QKeySequence(QKeySequence::HelpContents));
      fileTools->addAction(actionManual);

      QToolBar* editTools = addToolBar(tr("Edit Operations"));
      editTools->setObjectName("EditOperations");
      actionUndo->setIcon(*icons[int(Icons::undo_ICON)]);
      actionUndo->setShortcut(QKeySequence(QKeySequence::Undo));
      actionRedo->setIcon(*icons[int(Icons::redo_ICON)]);
      actionRedo->setShortcut(QKeySequence(QKeySequence::Redo));
      if (qApp->layoutDirection() == Qt::LayoutDirection::LeftToRight) {
            editTools->addAction(actionUndo);
            editTools->addAction(actionRedo);
            }
      else {
            editTools->addAction(actionUndo);
            editTools->addAction(actionRedo);
            }
      actionUndo->setEnabled(false);
      actionRedo->setEnabled(false);

      log->setReadOnly(true);
      log->setMaximumBlockCount(1000);

      readSettings();
      setState(PCState::EMPTY);

      connect(run,        SIGNAL(clicked()),     SLOT(runClicked()));
      connect(stop,       SIGNAL(clicked()),     SLOT(stopClicked()));
      connect(actionOpen, SIGNAL(triggered()),   SLOT(loadPlugin()));
      connect(actionReload, SIGNAL(triggered()), SLOT(load()));
      connect(actionSave, SIGNAL(triggered()),   SLOT(savePlugin()));
      connect(actionNew,  SIGNAL(triggered()),   SLOT(newPlugin()));
      connect(actionQuit, SIGNAL(triggered()),   SLOT(close()));
      connect(actionManual, SIGNAL(triggered()), SLOT(showManual()));
      connect(actionUndo, SIGNAL(triggered()),         textEdit,   SLOT(undo()));
      connect(actionRedo, SIGNAL(triggered()),         textEdit,   SLOT(redo()));
      connect(textEdit,   SIGNAL(undoAvailable(bool)), actionUndo, SLOT(setEnabled(bool)));
      connect(textEdit,   SIGNAL(redoAvailable(bool)), actionRedo, SLOT(setEnabled(bool)));
      connect(textEdit,   SIGNAL(textChanged()), SLOT(textChanged()));
      }

//---------------------------------------------------------
//   manualPath
//---------------------------------------------------------

QString PluginCreator::manualPath()
      {
      QString path = mscoreGlobalShare;
      path += "/manual/plugins/plugins.html";
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
            case PCState::INIT:
                  switch(newState) {
                        case PCState::INIT:
                              break;
                        case PCState::EMPTY:
                              setTitle("");
                              actionSave->setEnabled(false);
                              run->setEnabled(false);
                              stop->setEnabled(false);
                              textEdit->setEnabled(false);
                              break;
                        case PCState::CLEAN:
                        case PCState::DIRTY:
                              break;
                        }
                  break;
            case PCState::EMPTY:
                  switch(newState) {
                        case PCState::INIT:
                        case PCState::EMPTY:
                              break;
                        case PCState::CLEAN:
                              setTitle(path);
                              run->setEnabled(true);
                              textEdit->setEnabled(true);
                              break;
                        case PCState::DIRTY:
                              return;
                        }
                  break;
            case PCState::CLEAN:
                  switch(newState) {
                        case PCState::INIT:
                        case PCState::EMPTY:
                        case PCState::CLEAN:
                              break;
                        case PCState::DIRTY:
                              actionSave->setEnabled(true);
                              break;
                        }
                  break;
            case PCState::DIRTY:
                  switch(newState) {
                        case PCState::INIT:
                        case PCState::EMPTY:
                        case PCState::CLEAN:
                              actionSave->setEnabled(false);
                        case PCState::DIRTY:
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
      QString t(tr("MuseScore Plugin Creator"));
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
      settings.beginGroup(objectName());
      settings.setValue("windowState", saveState());
      settings.setValue("splitter", splitter->saveState());
      settings.endGroup();

      MuseScore::saveGeometry(this);
      }

//---------------------------------------------------------
//   readSettings
//---------------------------------------------------------

void PluginCreator::readSettings()
      {
      if (!useFactorySettings) {
            QSettings settings;
            settings.beginGroup(objectName());
            splitter->restoreState(settings.value("splitter").toByteArray());
            restoreState(settings.value("windowState").toByteArray());
            settings.endGroup();
            }

      MuseScore::restoreGeometry(this);
      }

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void PluginCreator::closeEvent(QCloseEvent* ev)
      {
      if (state == PCState::DIRTY) {
            QMessageBox::StandardButton n = QMessageBox::warning(this, tr("MuseScore"),
               tr("Plugin \"%1\" has changes.\n"
               "Save before closing?").arg(path),
               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
               QMessageBox::Save);
            if (n == QMessageBox::Save)
                  savePlugin();
            else if (n == QMessageBox::Cancel) {
                  ev->ignore();
                  return;
                  }
            }
      emit closed(false);
      QWidget::closeEvent(ev);
      }

//---------------------------------------------------------
//   qmlMsgHandler
//---------------------------------------------------------

static void qmlMsgHandler(QtMsgType type, const QMessageLogContext &, const QString & msg)
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
            default:
                  s = QString("Info: %1\n").arg(msg);
                  break;
            }
      mscore->pluginCreator()->msg(s);
      }

//---------------------------------------------------------
//   runClicked
//---------------------------------------------------------

void PluginCreator::runClicked()
      {
      log->clear();
      QQmlEngine* qml = Ms::MScore::qml();
      connect(qml, SIGNAL(warnings(const QList<QQmlError>&)),
         SLOT(qmlWarnings(const QList<QQmlError>&)));

      item = 0;
      QQmlComponent component(qml);
      component.setData(textEdit->toPlainText().toUtf8(), QUrl());
      QObject* obj = component.create();
      if (obj == 0) {
            msg("creating component failed\n");
            foreach(QQmlError e, component.errors())
                  msg(QString("   line %1: %2\n").arg(e.line()).arg(e.description()));
            stop->setEnabled(false);
            return;
            }
      qInstallMessageHandler(qmlMsgHandler);
      stop->setEnabled(true);
      run->setEnabled(false);

      item = qobject_cast<QmlPlugin*>(obj);
      if(MuseScoreCore::mscoreCore->currentScore() == nullptr && item->requiresScore() == true) {
            QMessageBox::information(0,
                  QMessageBox::tr("MuseScore"),
                  QMessageBox::tr("No score open.\n"
                  "This plugin requires an open score to run.\n"),
                  QMessageBox::Ok, QMessageBox::NoButton);
            delete obj;
            item = nullptr;
            closePlugin();
            return;
            }
      item->setFilePath(path.isEmpty() ? QString() : path.section('/', 0, -2));

      if (item->pluginType() == "dock" || item->pluginType() == "dialog") {
            view = new QQuickView(qml, 0);
            view->setTitle(item->menuPath().mid(item->menuPath().lastIndexOf(".") + 1));
            view->setColor(QApplication::palette().color(QPalette::Window));
            view->setResizeMode(QQuickView::SizeRootObjectToView);
            view->setWidth(item->width());
            view->setHeight(item->height());
            item->setParentItem(view->contentItem());

            if (item->pluginType() == "dock") {
                  dock = new QDockWidget("Plugin", 0);
                  dock->setAttribute(Qt::WA_DeleteOnClose);
                  dock->setWidget(QWidget::createWindowContainer(view));
                  dock->widget()->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
                  Qt::DockWidgetArea area = Qt::RightDockWidgetArea;
                  if (item->dockArea() == "left")
                        area = Qt::LeftDockWidgetArea;
                  else if (item->dockArea() == "top")
                        area = Qt::TopDockWidgetArea;
                  else if (item->dockArea() == "bottom")
                        area = Qt::BottomDockWidgetArea;
                  addDockWidget(area, dock);
                  connect(dock, SIGNAL(destroyed()), SLOT(closePlugin()));
                  dock->widget()->setAttribute(Qt::WA_DeleteOnClose);
                  dock->show();
                  }
            view->show();
            view->raise();
            connect(view, SIGNAL(destroyed()), SLOT(closePlugin()));
            }

      connect(qml,  SIGNAL(quit()), SLOT(closePlugin()));

      if (mscore->currentScore() && item->pluginType() != "dock")
            mscore->currentScore()->startCmd();
      item->runPlugin();
      if (mscore->currentScore() && item->pluginType() != "dock")
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
      qInstallMessageHandler(0);
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
      if (state == PCState::DIRTY) {
            QMessageBox::StandardButton n = QMessageBox::warning(this, tr("MuseScore"),
               tr("Plugin \"%1\" has changes.\n"
               "Save before closing?").arg(path),
               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
               QMessageBox::Save);
            if (n == QMessageBox::Save)
                  savePlugin();
            else if (n == QMessageBox::Cancel)
                  return;
            }
      path = mscore->getPluginFilename(true);
      load();
      }

void PluginCreator::load()
      {
      if (path.isEmpty())
            return;
      QFile f(path);
      QFileInfo fi(f);
      if (f.open(QIODevice::ReadOnly)) {
            textEdit->setPlainText(f.readAll());
            run->setEnabled(true);
            f.close();
            }
      else {
            path = QString();
            }
      created = false;
      setState(PCState::CLEAN);
      setTitle( fi.completeBaseName() );
      setToolTip(path);
      raise();
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
      QFileInfo fi(f);
      if(fi.suffix() != "qml" ) {
            QMessageBox::critical(mscore, tr("MuseScore: Save Plugin"), tr("Cannot determine file type"));
            return;
      }

      if (f.open(QIODevice::WriteOnly)) {
            f.write(textEdit->toPlainText().toUtf8());
            f.close();
            textEdit->document()->setModified(false);
            created = false;
            setState(PCState::CLEAN);
            setTitle( fi.completeBaseName() );
            setToolTip(path);
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
      if (state == PCState::DIRTY) {
            QMessageBox::StandardButton n = QMessageBox::warning(this, tr("MuseScore"),
               tr("Plugin \"%1\" has changes.\n"
               "Save before closing?").arg(path),
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
         "import QtQuick 2.0\n"
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
      setState(PCState::CLEAN);
      setTitle(path);
      setToolTip(path);
      raise();
      }

//---------------------------------------------------------
//   textChanged
//---------------------------------------------------------

void PluginCreator::textChanged()
      {
      actionSave->setEnabled(textEdit->document()->isModified());
      setState(PCState::DIRTY);
      }

//---------------------------------------------------------
//   qmlWarnings
//---------------------------------------------------------

void PluginCreator::qmlWarnings(const QList<QQmlError>& el)
      {
      foreach(const QQmlError& e, el)
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
            manualDock = new QDockWidget(tr("Manual"), 0);
            manualDock->setObjectName("Manual");

            manualDock->setWidget(helpBrowser);
            Qt::DockWidgetArea area = Qt::RightDockWidgetArea;
            addDockWidget(area, manualDock);
            helpBrowser->setContent(manualPath());
            }
      manualDock->setVisible(!manualDock->isVisible());
      }
}

