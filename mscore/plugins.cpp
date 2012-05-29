//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: plugins.cpp 5568 2012-04-22 10:08:43Z wschweer $
//
//  Copyright (C) 2009-2010 Werner Schweer and others
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
#include "globals.h"
#include "script.h"
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
#include "plugins.h"

// Q_DECLARE_METATYPE(Score*);

int QmlPlugin::mscoreVersion() const  { return version(); }

//---------------------------------------------------------
//   registerPlugin
//---------------------------------------------------------

void MuseScore::registerPlugin(const QString& pluginPath)
      {
      QFileInfo np(pluginPath);
      if (np.suffix() != "qml")
            return;
      QString baseName = np.baseName();

      foreach(QString s, plugins) {
            QFileInfo fi(s);
            if (fi.baseName() == baseName) {
                  if (MScore::debugMode)
                        qDebug("  Plugin <%s> already registered\n", qPrintable(pluginPath));
                  return;
                  }
            }

      QFile f(pluginPath);
      if (!f.open(QIODevice::ReadOnly)) {
            if (MScore::debugMode)
                  qDebug("Loading Plugin <%s> failed\n", qPrintable(pluginPath));
            return;
            }
      if (MScore::debugMode)
            qDebug("Register Plugin <%s>", qPrintable(pluginPath));
      f.close();
      if (qml == 0) {
            qml = new QDeclarativeEngine;
            qmlRegisterType<QmlPlugin>("MuseScore", 1, 0, "MuseScore");
            qmlRegisterType<Score>    ("MuseScore", 1, 0, "Score");
            qmlRegisterType<Segment>  ("MuseScore", 1, 0, "Segment");
//            qmlRegisterType<Element>  ("MuseScore", 1, 0, "Element");
            qmlRegisterType<Chord>    ("MuseScore", 1, 0, "Chord");
            qmlRegisterType<Rest>     ("MuseScore", 1, 0, "Rest");
            qmlRegisterType<Measure>  ("MuseScore", 1, 0, "Measure");
            qmlRegisterType<MScore>   ("MuseScore", 1, 0, "MScore");
            }
      QObject* obj = 0;
      {
      QDeclarativeComponent component(qml, QUrl::fromLocalFile(pluginPath));
      obj = component.create();
      if (obj == 0) {
            qDebug("creating component failed");
            return;
            }

      QmlPlugin* item = qobject_cast<QmlPlugin*>(obj);
      QString menuPath = item->menuPath();
      plugins.append(pluginPath);
      createMenuEntry(menuPath);
            }
      delete obj;
      }

//---------------------------------------------------------
//   createMenuEntry
//---------------------------------------------------------

void MuseScore::createMenuEntry(const QString& menu)
      {
      if (!pluginMapper)
            return;

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
                              qDebug("add Menu <%s>\n", qPrintable(m));
                        }
                  else if (i + 1 == n) {
                        QStringList sl = m.split(":");
                        QAction* a = 0;
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
                              a = new QAction(sl[1], 0);
                              cm->insertAction(ba, a);
                              }
                        else {
                              a = cm->addAction(m);
                              }
                        registerPlugin(a);
                        if (MScore::debugMode)
                              qDebug("add action <%s>\n", qPrintable(m));
                        }
                  else {
                        curMenu = ((QMenu*)curMenu)->addMenu(m);
                        if (MScore::debugMode)
                              qDebug("add menu <%s>\n", qPrintable(m));
                        }
                  }
            }
      }

//---------------------------------------------------------
//   pluginIdxFromPath
//---------------------------------------------------------

int MuseScore::pluginIdxFromPath(QString pluginPath) {
      QFileInfo np(pluginPath);
      QString baseName = np.baseName();
      int idx = 0;
      foreach(QString s, plugins) {
            QFileInfo fi(s);
            if (fi.baseName() == baseName)
                  return idx;
            idx++;
            }
      return -1;
      }

//---------------------------------------------------------
//   addGlobalObjectToPluginEngine
//---------------------------------------------------------

void MuseScore::addGlobalObjectToPluginEngine(const char * name, const QString & value )
      {
      se->globalObject().setProperty(name, se->newVariant(value));
      }

//---------------------------------------------------------
//   registerPlugin
//---------------------------------------------------------

void MuseScore::registerPlugin(QAction* a)
      {
      if (!pluginMapper) {
            qDebug("registerPlugin: no pluginMapper\n");
            return;
            }
      pluginActions.append(a);
      int pluginIdx = plugins.size() - 1; // plugin is already appended
      connect(a, SIGNAL(triggered()), pluginMapper, SLOT(map()));
      pluginMapper->setMapping(a, pluginIdx);
      }

//---------------------------------------------------------
//   loadPlugins
//---------------------------------------------------------

void MuseScore::loadPlugins()
      {
      pluginMapper = new QSignalMapper(this);
      connect(pluginMapper, SIGNAL(mapped(int)), SLOT(pluginTriggered(int)));
      foreach(PluginDescription* d, preferences.pluginList) {
            if (d->load) {
                  registerPlugin(d->path);
                  }
            }
      }

//---------------------------------------------------------
//   unloadPlugins
//---------------------------------------------------------

void MuseScore::unloadPlugins()
      {
      for (int idx = 0; idx < plugins.size() ; idx++) {
            ;
            }
      }

//---------------------------------------------------------
//   loadPlugin
//---------------------------------------------------------

bool MuseScore::loadPlugin(const QString& filename)
      {
      bool result = false;

      QDir pluginDir(mscoreGlobalShare + "plugins");
      if (MScore::debugMode)
            qDebug("Plugin Path <%s>\n", qPrintable(mscoreGlobalShare + "plugins"));

      if (filename.endsWith(".qml")){
            QFileInfo fi(pluginDir, filename);
            if (fi.exists()) {
                  QString path(fi.filePath());
                  registerPlugin(path);
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
      QDeclarativeView* view = new QDeclarativeView;
      view->setResizeMode(QDeclarativeView::SizeRootObjectToView);
      view->setSource(QUrl::fromLocalFile(pp));
      connect((QObject*)view->engine(), SIGNAL(quit()), view, SLOT(close()));
      view->show();
      QmlPlugin* p = (QmlPlugin*)(view->rootObject());
      p->runPlugin();
      }
