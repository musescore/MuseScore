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
#include "globals.h"
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
#include "plugins.h"
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

namespace Ms {

extern QString localeName;

//---------------------------------------------------------
//   registerPlugin
//---------------------------------------------------------

void MuseScore::registerPlugin(PluginDescription* plugin)
      {
      QString pluginPath = plugin->path;
      QFileInfo np(pluginPath);
      if (np.suffix() != "qml")
            return;
      QString baseName = np.baseName();

      foreach(QString s, plugins) {
            QFileInfo fi(s);
            if (fi.baseName() == baseName) {
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
      QQmlComponent component(qml(), QUrl::fromLocalFile(pluginPath));
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

//---------------------------------------------------------
//   qml
//---------------------------------------------------------

QQmlEngine* MuseScore::qml()
      {
      if (_qml == 0) {
            //-----------some qt bindings
            _qml = new QQmlEngine;
#ifdef Q_OS_WIN
            QStringList importPaths;
            QDir dir(QCoreApplication::applicationDirPath() + QString("/../qml"));
            importPaths.append(dir.absolutePath());
            _qml->setImportPathList(importPaths);
#endif
#ifdef Q_OS_MAC
            QStringList importPaths;
            QDir dir(mscoreGlobalShare + QString("/qml"));
            importPaths.append(dir.absolutePath());
            _qml->setImportPathList(importPaths);
#endif
            qmlRegisterType<MsProcess>  ("MuseScore", 1, 0, "QProcess");
            qmlRegisterType<FileIO, 1>  ("FileIO",    1, 0, "FileIO");
            //-----------mscore bindings
            qmlRegisterType<MScore>     ("MuseScore", 1, 0, "MScore");
            qmlRegisterType<MsScoreView>("MuseScore", 1, 0, "ScoreView");
            qmlRegisterType<QmlPlugin>  ("MuseScore", 1, 0, "MuseScore");
            qmlRegisterType<Score>      ("MuseScore", 1, 0, "Score");
            qmlRegisterType<Segment>    ("MuseScore", 1, 0, "Segment");
            qmlRegisterType<Chord>      ("MuseScore", 1, 0, "Chord");
            qmlRegisterType<Note>       ("MuseScore", 1, 0, "Note");
            qmlRegisterType<Accidental> ("MuseScore", 1, 0, "Accidental");
            qmlRegisterType<Rest>       ("MuseScore", 1, 0, "Rest");
            qmlRegisterType<Measure>    ("MuseScore", 1, 0, "Measure");
            qmlRegisterType<Cursor>     ("MuseScore", 1, 0, "Cursor");
            qmlRegisterType<StaffText>  ("MuseScore", 1, 0, "StaffText");
            qmlRegisterType<Part>       ("MuseScore", 1, 0, "Part");
            qmlRegisterType<Staff>      ("MuseScore", 1, 0, "Staff");
            qmlRegisterType<Harmony>    ("MuseScore", 1, 0, "Harmony");
            qmlRegisterType<PageFormat> ("MuseScore", 1, 0, "PageFormat");
            qmlRegisterType<TimeSig>    ("MuseScore", 1, 0, "TimeSig");
            qmlRegisterType<KeySig>     ("MuseScore", 1, 0, "KeySig");
            qmlRegisterType<Slur>       ("MuseScore", 1, 0, "Slur");
            qmlRegisterType<Tie>        ("MuseScore", 1, 0, "Tie");
            qmlRegisterType<NoteDot>    ("MuseScore", 1, 0, "NoteDot");
            qmlRegisterType<FiguredBass>("MuseScore", 1, 0, "FiguredBass");
            qmlRegisterType<Text>       ("MuseScore", 1, 0, "MText");
            qmlRegisterType<Lyrics>     ("MuseScore", 1, 0, "Lyrics");
            qmlRegisterType<FiguredBassItem>("MuseScore", 1, 0, "FiguredBassItem");
            qmlRegisterType<LayoutBreak>("MuseScore", 1, 0, "LayoutBreak");

            qmlRegisterUncreatableType<Element>("MuseScore", 1, 0,
               "Element", tr("you cannot create an element"));

            //-----------virtual classes
            qmlRegisterType<ChordRest>();
            qmlRegisterType<SlurTie>();
            qmlRegisterType<Spanner>();

            }
      return _qml;
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
//   loadPlugins
//---------------------------------------------------------

void MuseScore::loadPlugins()
      {
      pluginMapper = new QSignalMapper(this);
      connect(pluginMapper, SIGNAL(mapped(int)), SLOT(pluginTriggered(int)));
      for (int i = 0; i < preferences.pluginList.size(); ++i) {
            PluginDescription* d = &preferences.pluginList[i];
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

      QQmlEngine* engine = qml();

      QQmlComponent component(engine);
      component.loadUrl(QUrl::fromLocalFile(pp));
      QObject* obj = component.create();
      if (obj == 0) {
            foreach(QQmlError e, component.errors())
                  qDebug("   line %d: %s\n", e.line(), qPrintable(e.description()));
            return;
            }

      QmlPlugin* p = qobject_cast<QmlPlugin*>(obj);

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
                  view->show();
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
      endCmd();
      }

//---------------------------------------------------------
//   MsScoreView
//---------------------------------------------------------

MsScoreView::MsScoreView(QQuickItem* parent)
   : QQuickPaintedItem(parent)
      {
      setAcceptedMouseButtons(Qt::LeftButton);
      score = 0;
      }

//---------------------------------------------------------
//   @@ FileIO
//---------------------------------------------------------

FileIO::FileIO(QObject *parent) :
    QObject(parent)
      {
      }

QString FileIO::read()
      {
      if (mSource.isEmpty()) {
            emit error("source is empty");
            return QString();
            }
      QUrl url(mSource);
      QString source(mSource);
      if(url.isValid() && url.isLocalFile()) {
            source = url.toLocalFile();
            }
      QFile file(source);
      QString fileContent;
      if ( file.open(QIODevice::ReadOnly) ) {
            QString line;
            QTextStream t( &file );
            do {
                line = t.readLine();
                fileContent += line + "\n";
                } while (!line.isNull());
            file.close();
            }
      else {
          emit error("Unable to open the file");
          return QString();
          }
      return fileContent;
      }

bool FileIO::write(const QString& data)
      {
      if (mSource.isEmpty())
            return false;

      QFile file(mSource);
      if (!file.open(QFile::WriteOnly | QFile::Truncate))
            return false;

      QTextStream out(&file);
      out << data;
      file.close();
      return true;
      }

bool FileIO::remove()
      {
      if (mSource.isEmpty())
            return false;

      QFile file(mSource);
      return file.remove();
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void MsScoreView::setScore(Score* s)
      {
      _currentPage = 0;
      score = s;

      if (score) {
            score->doLayout();

            Page* page = score->pages()[_currentPage];
            QRectF pr(page->abbox());
            qreal m1 = width()  / pr.width();
            qreal m2 = height() / pr.height();
            mag = qMax(m1, m2);

            _boundingRect = QRectF(0.0, 0.0, pr.width() * mag, pr.height() * mag);

            setWidth(pr.width() * mag);
            setHeight(pr.height() * mag);
            }
      update();
      }

//---------------------------------------------------------
//   paint
//---------------------------------------------------------

void MsScoreView::paint(QPainter* p)
      {
      p->setRenderHint(QPainter::Antialiasing, true);
      p->setRenderHint(QPainter::TextAntialiasing, true);
      p->fillRect(QRect(0, 0, width(), height()), _color);
      if (!score)
            return;
      p->scale(mag, mag);

      Page* page = score->pages()[_currentPage];
      QList<const Element*> el;
      foreach(System* s, *page->systems()) {
            foreach(MeasureBase* m, s->measures())
                  m->scanElements(&el, collectElements, false);
            }
      page->scanElements(&el, collectElements, false);

      foreach(const Element* e, el) {
            QPointF pos(e->pagePos());
            p->translate(pos);
            e->draw(p);
            p->translate(-pos);
            }
      }

//---------------------------------------------------------
//   setCurrentPage
//---------------------------------------------------------

void MsScoreView::setCurrentPage(int n)
      {
      if (score == 0)
            return;
      if (n < 0)
            n = 0;
      int nn = score->pages().size();
      if (nn == 0)
            return;
      if (n >= nn)
            n = nn - 1;
      _currentPage = n;
      update();
      }

//---------------------------------------------------------
//   nextPage
//---------------------------------------------------------

void MsScoreView::nextPage()
      {
      setCurrentPage(_currentPage + 1);
      }

//---------------------------------------------------------
//   prevPage
//---------------------------------------------------------

void MsScoreView::prevPage()
      {
      setCurrentPage(_currentPage - 1);
      }

const QRectF& MsScoreView::getGrip(int) const
      {
      static const QRectF a;
      return a;
      }

const QTransform& MsScoreView::matrix() const
      {
      static const QTransform t;
      return t; // _matrix;
      }

//---------------------------------------------------------
//   collectPluginMetaInformation
//---------------------------------------------------------

void collectPluginMetaInformation(PluginDescription* d)
      {
      printf("collect meta for <%s>\n", qPrintable(d->path));

      QQmlComponent component(mscore->qml(), QUrl::fromLocalFile(d->path));
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

