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
#include "libmscore/notedot.h"
#include "libmscore/figuredbass.h"

//---------------------------------------------------------
//   QmlPlugin
//---------------------------------------------------------

QmlPlugin::QmlPlugin(QDeclarativeItem* parent)
   : QDeclarativeItem(parent)
      {
      }

QmlPlugin::~QmlPlugin()
      {
      }

//---------------------------------------------------------
//   curScore
//---------------------------------------------------------

Score* QmlPlugin::curScore() const
      {
      return mscore->currentScore();
      }

//---------------------------------------------------------
//   scores
//---------------------------------------------------------

QDeclarativeListProperty<Score> QmlPlugin::scores()
      {
      return QDeclarativeListProperty<Score>(this, mscore->scores());
      }

//---------------------------------------------------------
//   writeScore
//---------------------------------------------------------

bool QmlPlugin::writeScore(Score* s, const QString& name, const QString& ext)
      {
      if(!s)
            return false;
      return mscore->saveAs(s, true, name, ext);
      }

//---------------------------------------------------------
//   readScore
//---------------------------------------------------------

Score* QmlPlugin::readScore(const QString& name)
      {
      return mscore->openScore(name);
      }

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
      QObject* obj = 0;
      QDeclarativeComponent component(qml(), QUrl::fromLocalFile(pluginPath));
      obj = component.create();
      if (obj == 0) {
            qDebug("creating component <%s> failed", qPrintable(pluginPath));
            foreach(QDeclarativeError e, component.errors()) {
                  qDebug("   line %d: %s", e.line(), qPrintable(e.description()));
                  }
            return;
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

QDeclarativeEngine* MuseScore::qml()
      {
      if (_qml == 0) {
            //-----------some qt bindings
            _qml = new QDeclarativeEngine;
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
            qmlRegisterType<FiguredBassItem>("MuseScore", 1, 0, "FiguredBassItem");
            //-----------virtual classes
            qmlRegisterType<Element>();
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
                              qDebug("add Menu <%s>\n", qPrintable(m));
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
//   loadPlugins
//---------------------------------------------------------

void MuseScore::loadPlugins()
      {
      pluginMapper = new QSignalMapper(this);
      connect(pluginMapper, SIGNAL(mapped(int)), SLOT(pluginTriggered(int)));
      foreach(PluginDescription* d, preferences.pluginList) {
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

      QDir pluginDir(mscoreGlobalShare + "plugins");
      if (MScore::debugMode)
            qDebug("Plugin Path <%s>\n", qPrintable(mscoreGlobalShare + "plugins"));

      if (filename.endsWith(".qml")){
            QFileInfo fi(pluginDir, filename);
            if (fi.exists()) {
                  QString path(fi.filePath());
//TODO                  registerPlugin(path);
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
      view->setResizeMode(QDeclarativeView::SizeViewToRootObject);
      view->setSource(QUrl::fromLocalFile(pp));
      connect((QObject*)view->engine(), SIGNAL(quit()), view, SLOT(close()));
      view->show();
      QmlPlugin* p = (QmlPlugin*)(view->rootObject());
      if (p->pluginType() == "dock") {
            QDockWidget* dock = new QDockWidget("Plugin", 0);
            dock->setAttribute(Qt::WA_DeleteOnClose);
            dock->setWidget(view);
            Qt::DockWidgetArea area = Qt::RightDockWidgetArea;
            if (p->dockArea() == "left")
                  area = Qt::LeftDockWidgetArea;
            else if (p->dockArea() == "top")
                  area = Qt::TopDockWidgetArea;
            else if (p->dockArea() == "bottom")
                  area = Qt::BottomDockWidgetArea;
            addDockWidget(area, dock);
            connect((QObject*)view->engine(), SIGNAL(quit()), dock, SLOT(close()));
            }
      else {
            connect((QObject*)view->engine(), SIGNAL(quit()), view, SLOT(close()));
            }
      if (cs)
            cs->startCmd();
      p->runPlugin();
      if (cs)
            cs->endCmd();
      endCmd();
      }

//---------------------------------------------------------
//   newElement
//---------------------------------------------------------

Element* QmlPlugin::newElement(int t)
      {
      Score* score = curScore();
      if (score == 0)
            return 0;
      return Element::create(ElementType(t), score);
      }

//---------------------------------------------------------
//   newScore
//---------------------------------------------------------

Score* QmlPlugin::newScore(const QString& name, const QString& part, int measures)
      {
      if (mscore->currentScore()) {
            mscore->currentScore()->endCmd();
            mscore->endCmd();
            }
      Score* score = new Score(MScore::defaultStyle());
      int view = mscore->appendScore(score);
      mscore->setCurrentView(0, view);
      qApp->processEvents();
      score->setName(name);
      score->appendPart(part);
      score->appendMeasures(measures);
      score->doLayout();
      score->startCmd();
      return score;
      }

//---------------------------------------------------------
//   cmd
//---------------------------------------------------------

void QmlPlugin::cmd(const QString& s)
      {
      Shortcut* sc = Shortcut::getShortcut(s.toAscii().data());
      if (sc)
            mscore->cmd(sc->action());
      else
            printf("QmlPlugin:cmd: not found <%s>\n", qPrintable(s));
      }

//---------------------------------------------------------
//   MsScoreView
//---------------------------------------------------------

MsScoreView::MsScoreView(QDeclarativeItem* parent)
   : QDeclarativeItem(parent)
      {
      setFlag(QGraphicsItem::ItemHasNoContents, false);
      setCacheMode(QGraphicsItem::ItemCoordinateCache);
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

      QFile file(mSource);
      QString fileContent;
      if ( file.open(QIODevice::ReadOnly) ) {
            QString line;
            QTextStream t( &file );
            do {
                line = t.readLine();
                fileContent += line;
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

bool FileIO::remove(const QString& data)
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

void MsScoreView::paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*)
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

      QDeclarativeComponent component(mscore->qml(), QUrl::fromLocalFile(d->path));
      QObject* obj = component.create();
      if (obj == 0) {
            qDebug("creating component <%s> failed", qPrintable(d->path));
            foreach(QDeclarativeError e, component.errors()) {
                  qDebug("   line %d: %s", e.line(), qPrintable(e.description()));
                  }
            return;
            }
      QmlPlugin* item = qobject_cast<QmlPlugin*>(obj);
      d->version      = item->version();
      d->description  = item->description();
      delete obj;
      }

