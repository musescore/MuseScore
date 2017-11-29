//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2009-2012 Werner Schweer and others
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

#include "musescore.h"
#include "libmscore/score.h"
#include "libmscore/undo.h"
// #include "globals.h"
#include "config.h"
#include "preferences.h"
#include "libmscore/chord.h"
#include "libmscore/note.h"
#include "libmscore/utils.h"
#include "libmscore/mscore.h"
#include "libmscore/measurebase.h"
#include "libmscore/measure.h"
#include "libmscore/segment.h"
#include "libmscore/rest.h"
#include "libmscore/stafftext.h"
// #include "plugins.h"
#include "libmscore/cursor.h"
#include "libmscore/page.h"
#include "libmscore/system.h"
#include "libmscore/staff.h"
#include "libmscore/part.h"
#include "libmscore/timesig.h"
#include "libmscore/keysig.h"
#include "libmscore/harmony.h"
#include "libmscore/slur.h"
#include "libmscore/tie.h"
#include "libmscore/notedot.h"
#include "libmscore/figuredbass.h"
#include "libmscore/accidental.h"
#include "libmscore/lyrics.h"
#include "libmscore/layoutbreak.h"
#include "qmlplugin.h"
#include "pluginManager.h"

namespace Ms {

//---------------------------------------------------------
//   registerPlugin
//---------------------------------------------------------

void MuseScore::registerPlugin(PluginDescription* plugin)
      {
      QString pluginPath = plugin->path;
      QFileInfo np(pluginPath);
      if (np.suffix() != "qml")
            return;
      QString baseName = np.completeBaseName();

      foreach(QString s, plugins) {
            QFileInfo fi(s);
            if (fi.completeBaseName() == baseName) {
                  if (MScore::debugMode)
                        qDebug("  Plugin <%s> already registered", qPrintable(pluginPath));
                  return;
                  }
            }

      QFile f(pluginPath);
      if (!f.open(QIODevice::ReadOnly)) {
            if (MScore::debugMode)
                  qDebug("Loading Plugin <%s> failed", qPrintable(pluginPath));
            return;
            }
      if (MScore::debugMode)
            qDebug("Register Plugin <%s>", qPrintable(pluginPath));
      f.close();
      QObject* obj = 0;
      QQmlComponent component(Ms::MScore::qml(), QUrl::fromLocalFile(pluginPath));
      obj = component.create();
      if (obj == 0) {
            qDebug("creating component <%s> failed", qPrintable(pluginPath));
            foreach(QQmlError e, component.errors()) {
                  qDebug("   line %d: %s", e.line(), qPrintable(e.description()));
                  }
            return;
            }
      //
      // load translation
      //
      QLocale locale;
      QString t = np.absolutePath() + "/translations/locale_" + MuseScore::getLocaleISOCode().left(2) + ".qm";
      QTranslator* translator = new QTranslator;
      if (!translator->load(t)) {
//            qDebug("cannot load qml translations <%s>", qPrintable(t));
            delete translator;
            }
      else {
//            qDebug("load qml translations <%s>", qPrintable(t));
            qApp->installTranslator(translator);
            }

      QmlPlugin* item = qobject_cast<QmlPlugin*>(obj);
      QString menuPath = item->menuPath();
      plugin->menuPath = menuPath;
      plugins.append(pluginPath);
      createMenuEntry(plugin);

      QAction* a = plugin->shortcut.action();
      pluginActions.append(a);
      int pluginIdx = plugins.size() - 1; // plugin is already appended
      connect(a, SIGNAL(triggered()), pluginMapper, SLOT(map()));
      pluginMapper->setMapping(a, pluginIdx);

      delete obj;
      }

void MuseScore::unregisterPlugin(PluginDescription* plugin)
      {
      QString pluginPath = plugin->path;
      QFileInfo np(pluginPath);
      if (np.suffix() != "qml")
            return;
      QString baseName = np.completeBaseName();

      bool found = false;
      foreach(QString s, plugins) {
            QFileInfo fi(s);
            if (fi.completeBaseName() == baseName) {
                  found = true;
                  break;
                  }
            }
      if (!found) {
            if (MScore::debugMode)
                  qDebug("  Plugin <%s> not registered", qPrintable(pluginPath));
            return;
            }
      plugins.removeAll(pluginPath);


      removeMenuEntry(plugin);
      QAction* a = plugin->shortcut.action();
      pluginActions.removeAll(a);

      disconnect(a, SIGNAL(triggered()), pluginMapper, SLOT(map()));
      pluginMapper->removeMappings(a);

      }

//---------------------------------------------------------
//   createMenuEntry
//    syntax: "entry.entry.entry"
//---------------------------------------------------------

void MuseScore::createMenuEntry(PluginDescription* plugin)
      {
      if (!pluginMapper)
            return;

      QString menu = plugin->menuPath;
      QStringList ml;
      QString s;
      bool escape = false;
      foreach (QChar c, menu) {
            if (escape) {
                  escape = false;
                  s += c;
                  }
            else {
                  if (c == '\\')
                        escape = true;
                  else {
                        if (c == '.') {
                              ml += s;
                              s = "";
                              }
                        else {
                              s += c;
                              }
                        }
                  }
            }
      if (!s.isEmpty())
            ml += s;

      int n            = ml.size();
      QWidget* curMenu = menuBar();

      for(int i = 0; i < n; ++i) {
            QString m  = ml[i];
            bool found = false;
            QList<QObject*> ol = curMenu->children();
            foreach(QObject* o, ol) {
                  QMenu* menu = qobject_cast<QMenu*>(o);
                  if (!menu)
                        continue;
                  if (menu->objectName() == m || menu->title() == m) {
                        curMenu = menu;
                        found = true;
                        break;
                        }
                  }
            if (!found) {
                  if (i == 0) {
                        curMenu = new QMenu(m, menuBar());
                        curMenu->setObjectName(m);
                        menuBar()->insertMenu(menuBar()->actions().back(), (QMenu*)curMenu);
                        if (MScore::debugMode)
                              qDebug("add Menu <%s>", qPrintable(m));
                        }
                  else if (i + 1 == n) {
                        QStringList sl = m.split(":");
                        QAction* a = plugin->shortcut.action();
                        QMenu* cm = static_cast<QMenu*>(curMenu);
                        if (sl.size() == 2) {
                              QList<QAction*> al = cm->actions();
                              QAction* ba = 0;
                              foreach(QAction* ia, al) {
                                    if (ia->text() == sl[0]) {
                                          ba = ia;
                                          break;
                                          }
                                    }
                              a->setText(sl[1]);
                              cm->insertAction(ba, a);
                              }
                        else {
                              a->setText(m);
                              cm->addAction(a);
                              }

                        if (MScore::debugMode)
                              qDebug("plugins: add action <%s>", qPrintable(m));
                        }
                  else {
                        curMenu = ((QMenu*)curMenu)->addMenu(m);
                        if (MScore::debugMode)
                              qDebug("add menu <%s>", qPrintable(m));
                        }
                  }
            }
      }


void MuseScore::removeMenuEntry(PluginDescription* plugin)
      {
      if (!pluginMapper)
            return;

      QString menu = plugin->menuPath;
      QStringList ml;
      QString s;
      bool escape = false;
      foreach (QChar c, menu) {
            if (escape) {
                  escape = false;
                  s += c;
                  }
            else {
                  if (c == '\\')
                        escape = true;
                  else {
                        if (c == '.') {
                              ml += s;
                              s = "";
                              }
                        else {
                              s += c;
                              }
                        }
                  }
            }
      if (!s.isEmpty())
            ml += s;

      if(ml.isEmpty())
            return;

      int n            = ml.size();
      QWidget* curMenu = menuBar();

      for(int i = 0; i < n-1; ++i) {
            QString m  = ml[i];
            QList<QObject*> ol = curMenu->children();
            foreach(QObject* o, ol) {
                  QMenu* menu = qobject_cast<QMenu*>(o);
                  if (!menu)
                        continue;
                  if (menu->objectName() == m || menu->title() == m) {
                        curMenu = menu;
                        break;
                        }
                  }
            }
      QString m  = ml[n-1];
      QStringList sl = m.split(":");
      QAction* a = plugin->shortcut.action();
      QMenu* cm = static_cast<QMenu*>(curMenu);
      cm->removeAction(a);
      for(int i = n-2; i >= 0; --i) {

            QMenu* menu = qobject_cast<QMenu*>(cm->parent());
            if (cm->isEmpty())
                  if(cm->isEmpty()) {
                        delete cm;
                        }
            cm = menu;
            }
      }

//---------------------------------------------------------
//   pluginIdxFromPath
//---------------------------------------------------------

int MuseScore::pluginIdxFromPath(QString pluginPath) {
      QFileInfo np(pluginPath);
      QString baseName = np.completeBaseName();
      int idx = 0;
      foreach(QString s, plugins) {
            QFileInfo fi(s);
            if (fi.completeBaseName() == baseName)
                  return idx;
            idx++;
            }
      return -1;
      }

//---------------------------------------------------------
//   loadPlugins
//---------------------------------------------------------

void MuseScore::loadPlugins()
      {
      pluginMapper = new QSignalMapper(this);
      connect(pluginMapper, SIGNAL(mapped(int)), SLOT(pluginTriggered(int)));
      for (int i = 0; i < pluginManager->pluginCount(); ++i) {
            PluginDescription* d = pluginManager->getPluginDescription(i);
            if (d->load)
                  registerPlugin(d);
            }
      }

//---------------------------------------------------------
//   unloadPlugins
//---------------------------------------------------------

void MuseScore::unloadPlugins()
      {
      for (int idx = 0; idx < plugins.size() ; idx++) {
            ; // TODO
            }
      }

//---------------------------------------------------------
//   loadPlugin
//---------------------------------------------------------

bool MuseScore::loadPlugin(const QString& filename)
      {
      bool result = false;

      if (!pluginMapper) {
            pluginMapper = new QSignalMapper(this);
            connect(pluginMapper, SIGNAL(mapped(int)), SLOT(pluginTriggered(int)));
            }

      QDir pluginDir(mscoreGlobalShare + "plugins");
      if (MScore::debugMode)
            qDebug("Plugin Path <%s>", qPrintable(mscoreGlobalShare + "plugins"));

      if (filename.endsWith(".qml")){
            QFileInfo fi(pluginDir, filename);
            if (!fi.exists())
                  fi = QFileInfo(preferences.getString(PREF_APP_PATHS_MYPLUGINS), filename);
            if (fi.exists()) {
                  QString path(fi.filePath());
                  PluginDescription* p = new PluginDescription;
                  p->path = path;
                  p->load = false;
                  collectPluginMetaInformation(p);
                  registerPlugin(p);
                  result = true;
                  }
            }
      return result;
      }

//---------------------------------------------------------
//   pluginTriggered
//---------------------------------------------------------

void MuseScore::pluginTriggered(int idx)
      {
      QString pp = plugins[idx];

      QQmlEngine* engine = Ms::MScore::qml();

      QQmlComponent component(engine);
      component.loadUrl(QUrl::fromLocalFile(pp));
      QObject* obj = component.create();
      if (obj == 0) {
            foreach(QQmlError e, component.errors())
                  qDebug("   line %d: %s", e.line(), qPrintable(e.description()));
            return;
            }

      QmlPlugin* p = qobject_cast<QmlPlugin*>(obj);
      if(MuseScoreCore::mscoreCore->currentScore() == nullptr && p->requiresScore() == true) {
            QMessageBox::information(0,
                  QMessageBox::tr("MuseScore"),
                  QMessageBox::tr("No score open.\n"
                  "This plugin requires an open score to run.\n"),
                  QMessageBox::Ok, QMessageBox::NoButton);
            delete obj;
            return;
            }
      p->setFilePath(pp.section('/', 0, -2));

      if (p->pluginType() == "dock" || p->pluginType() == "dialog") {
            QQuickView* view = new QQuickView(engine, 0);
            view->setSource(QUrl::fromLocalFile(pp));
            view->setTitle(p->menuPath().mid(p->menuPath().lastIndexOf(".") + 1));
            view->setColor(QApplication::palette().color(QPalette::Window));
            //p->setParentItem(view->contentItem());
            //view->setWidth(p->width());
            //view->setHeight(p->height());
            view->setResizeMode(QQuickView::SizeRootObjectToView);
            if (p->pluginType() == "dock") {
                  QDockWidget* dock = new QDockWidget("Plugin", 0);
                  dock->setAttribute(Qt::WA_DeleteOnClose);
                  Qt::DockWidgetArea area = Qt::RightDockWidgetArea;
                  if (p->dockArea() == "left")
                        area = Qt::LeftDockWidgetArea;
                  else if (p->dockArea() == "top")
                        area = Qt::TopDockWidgetArea;
                  else if (p->dockArea() == "bottom")
                        area = Qt::BottomDockWidgetArea;
                  QWidget* w = QWidget::createWindowContainer(view);
                  dock->setWidget(w);
                  addDockWidget(area, dock);
                  connect(engine, SIGNAL(quit()), dock, SLOT(close()));
                  dock->show();
                  }
            else {
                  connect(engine, SIGNAL(quit()), view, SLOT(close()));
                  view->show();
                  }
            }

      // dont call startCmd for non modal dialog
      if (cs && p->pluginType() != "dock")
            cs->startCmd();
      p->runPlugin();
      if (cs && p->pluginType() != "dock")
            cs->endCmd();
//      endCmd();
      }

//---------------------------------------------------------
//   collectPluginMetaInformation
//---------------------------------------------------------

void collectPluginMetaInformation(PluginDescription* d)
      {
      qDebug("Collect meta for <%s>", qPrintable(d->path));

      QQmlComponent component(Ms::MScore::qml(), QUrl::fromLocalFile(d->path));
      QObject* obj = component.create();
      if (obj == 0) {
            qDebug("creating component <%s> failed", qPrintable(d->path));
            foreach(QQmlError e, component.errors()) {
                  qDebug("   line %d: %s", e.line(), qPrintable(e.description()));
                  }
            return;
            }
      QmlPlugin* item = qobject_cast<QmlPlugin*>(obj);
      if (item) {
            d->version      = item->version();
            d->description  = item->description();
            }
      delete obj;
      }
}

