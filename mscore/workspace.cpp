//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2011 Werner Schweer and others
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

#include "workspace.h"
#include "musescore.h"
#include "libmscore/score.h"
#include "libmscore/imageStore.h"
#include "libmscore/xml.h"
#include "thirdparty/qzip/qzipreader_p.h"
#include "thirdparty/qzip/qzipwriter_p.h"
#include "preferences.h"
#include "palette.h"
#include "palette/paletteworkspace.h"
#include "extension.h"

#if defined(FOR_WINSTORE)  // or even just Q_OS_WIN ?
extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;
#else
int qt_ntfs_permission_lookup;
#endif

namespace Ms {

bool WorkspacesManager::isWorkspacesListDirty = true;
Workspace* WorkspacesManager::m_currentWorkspace = nullptr;
QList<Workspace*> WorkspacesManager::m_workspaces {};
QList<Workspace*> WorkspacesManager::m_visibleWorkspaces {};

QList<QPair<QAction*, QString>> Workspace::actionToStringList {};
QList<QPair<QMenu*  , QString>> Workspace::menuToStringList   {};

std::vector<QString> WorkspacesManager::defaultWorkspaces {
      QT_TRANSLATE_NOOP("Ms::Workspace", "Basic"),
      QT_TRANSLATE_NOOP("Ms::Workspace", "Advanced"),
      };

std::vector<QString> WorkspacesManager::defaultEditedWorkspaces {
      QT_TRANSLATE_NOOP("Ms::Workspace", "Basic edited"),
      QT_TRANSLATE_NOOP("Ms::Workspace", "Advanced edited"),
      };

//---------------------------------------------------------
//   editedWorkspaceName
//---------------------------------------------------------

static QString editedWorkspaceTranslatableName(const QString& oldWorkspaceTranslatableName)
      {
      if (oldWorkspaceTranslatableName.isEmpty())
            return QString();

      const auto it = std::find(WorkspacesManager::defaultWorkspaces.begin(), WorkspacesManager::defaultWorkspaces.end(), oldWorkspaceTranslatableName);

      if (it != WorkspacesManager::defaultWorkspaces.end()) {
            const int idx = it - WorkspacesManager::defaultWorkspaces.begin();
            if (idx < int(WorkspacesManager::defaultEditedWorkspaces.size()))
                  return WorkspacesManager::defaultEditedWorkspaces[idx];
            }

      return QString();
      }

//---------------------------------------------------------
//   editedWorkspaceName
//---------------------------------------------------------

QString WorkspacesManager::defaultWorkspaceTranslatableName(const QString& editedWorkspaceName)
      {
      const auto it = std::find(WorkspacesManager::defaultEditedWorkspaces.begin(), WorkspacesManager::defaultEditedWorkspaces.end(), editedWorkspaceName);

      if (it != WorkspacesManager::defaultEditedWorkspaces.end()) {
            const int idx = it - WorkspacesManager::defaultEditedWorkspaces.begin();
            if (idx < int(WorkspacesManager::defaultWorkspaces.size()))
                  return WorkspacesManager::defaultWorkspaces[idx];
            }

      return QString();
      }

/**
 *   Reverts current workspace to the state of the `source` workspace
 */
void MuseScore::resetWorkspace()
      {
      //if currentWorkspace is the `edited` Basic or Advanced one, remove the edited and show the source one
      if (WorkspacesManager::isDefaultEditedWorkspace(WorkspacesManager::currentWorkspace())) {
            const QString& currWorkspaceName = WorkspacesManager::currentWorkspace()->translatableName();
            const QString& defaultWorkspaceName = WorkspacesManager::defaultWorkspaceTranslatableName(currWorkspaceName);
            Q_ASSERT(!defaultWorkspaceName.isEmpty());
            Workspace* defaultWorkspace = WorkspacesManager::findByTranslatableName(defaultWorkspaceName);
            Workspace* currWorkspace = WorkspacesManager::currentWorkspace();
            changeWorkspace(defaultWorkspace);
            WorkspacesManager::remove(currWorkspace);
            }
      //else if currentWorkspace is a custom workspace, reset all palettes, toolbars, menus and GUI to the values defined in the source workspace
      else
            WorkspacesManager::currentWorkspace()->reset();
      }

//---------------------------------------------------------
//   showWorkspaceMenu
//---------------------------------------------------------

void MuseScore::showWorkspaceMenu()
      {
      if (workspaces == 0) {
            workspaces = new QActionGroup(this);
            workspaces->setExclusive(true);
            connect(workspaces, SIGNAL(triggered(QAction*)), SLOT(changeWorkspace(QAction*)));
            }
      else {
            for (QAction* a : workspaces->actions())
                  workspaces->removeAction(a);
            }
      menuWorkspaces->clear();

      for (Workspace* p : WorkspacesManager::visibleWorkspaces()) {
            QAction* a = workspaces->addAction(qApp->translate("Ms::Workspace", p->name().toUtf8()));
            a->setCheckable(true);
            a->setData(p->path());
            a->setChecked(p->name() == preferences.getString(PREF_APP_WORKSPACE));
            menuWorkspaces->addAction(a);
            }

      menuWorkspaces->addSeparator();
      QAction* a = new QAction(tr("New…"), this);
      connect(a, SIGNAL(triggered()), SLOT(createNewWorkspace()));
      menuWorkspaces->addAction(a);

      a = new QAction(tr("Edit"), this);
      a->setDisabled(WorkspacesManager::currentWorkspace()->readOnly());
      connect(a, SIGNAL(triggered()), SLOT(editWorkspace()));
      menuWorkspaces->addAction(a);

      a = new QAction(tr("Delete"), this);
      a->setDisabled(WorkspacesManager::currentWorkspace()->readOnly());
      connect(a, SIGNAL(triggered()), SLOT(deleteWorkspace()));
      menuWorkspaces->addAction(a);

      a = new QAction(tr("Reset workspace"), this);
      connect(a, SIGNAL(triggered()), SLOT(resetWorkspace()));
      menuWorkspaces->addAction(a);
      }

//---------------------------------------------------------
//   deleteWorkspace
//---------------------------------------------------------

void MuseScore::deleteWorkspace()
      {
      Workspace* workspace = WorkspacesManager::currentWorkspace();
      if (!workspace)
            return;

      QMessageBox::StandardButton reply =
         (MScore::noGui && MScore::testMode)
         ? QMessageBox::Yes
         : QMessageBox::question(0,
                 QWidget::tr("Are you sure?"),
                 QWidget::tr("Do you really want to delete the '%1' workspace?").arg(workspace->name()),
                 QMessageBox::Yes | QMessageBox::No,
                 QMessageBox::Yes
                 );
      if (reply != QMessageBox::Yes)
            return;

      WorkspacesManager::remove(workspace);
      WorkspacesManager::setCurrentWorkspace(WorkspacesManager::workspaces().first());
      changeWorkspace(WorkspacesManager::currentWorkspace());
      updateIcons();
      }

//---------------------------------------------------------
//   changeWorkspace
//---------------------------------------------------------

void MuseScore::changeWorkspace(QAction* a)
      {
      changeWorkspace(a->text());
      }

//---------------------------------------------------------
//   changeWorkspace
//---------------------------------------------------------

void MuseScore::changeWorkspace(const QString& name)
      {
      for (Workspace* p : WorkspacesManager::workspaces()) {
            if (qApp->translate("Ms::Workspace", p->name().toUtf8()) == name) {
                  changeWorkspace(p);
                  return;
                  }
            }
      qDebug("   workspace \"%s\" not found", qPrintable(name));
      }

//---------------------------------------------------------
//   changeWorkspace
//---------------------------------------------------------

void MuseScore::changeWorkspace(Workspace* p, bool first)
      {
      if (!first)
            WorkspacesManager::currentWorkspace()->save();

      if (WorkspacesManager::currentWorkspace())
            disconnect(getPaletteWorkspace(), &PaletteWorkspace::userPaletteChanged, WorkspacesManager::currentWorkspace(), QOverload<>::of(&Workspace::setDirty));

      p->read();
      WorkspacesManager::setCurrentWorkspace(p);
      if (!first) {
            updateIcons();
            preferencesChanged(true);
            }

      connect(getPaletteWorkspace(), &PaletteWorkspace::userPaletteChanged, WorkspacesManager::currentWorkspace(), QOverload<>::of(&Workspace::setDirty), Qt::UniqueConnection);

      preferences.setPreference(PREF_APP_WORKSPACE, p->id());
      emit mscore->workspacesChanged();
      }

//---------------------------------------------------------
//   updateIcons
//---------------------------------------------------------

void MuseScore::updateIcons()
      {
      setIconSize(QSize(preferences.getInt(PREF_UI_THEME_ICONWIDTH) * guiScaling, preferences.getInt(PREF_UI_THEME_ICONHEIGHT) * guiScaling));
      for (QAction* a : fileTools->actions()) {
            QWidget* widget = fileTools->widgetForAction(a);
            QString className = widget->metaObject()->className();
            if (className != "Ms::AccessibleToolButton" && className != "QToolBarSeparator")
                  widget->setFixedHeight(preferences.getInt(PREF_UI_THEME_ICONHEIGHT) + 8);  // hack
                  // apparently needed for viewModeCombo, see MuseScore::populateFileOperations
            }
      for (QAction* a : cpitchTools->actions()) {
            QWidget* widget = cpitchTools->widgetForAction(a);
            if (widget->property("iconic-text") == true)
                  widget->setFixedHeight(preferences.getInt(PREF_UI_THEME_ICONHEIGHT) + 8);  // hack
                  // so that toolbar buttons with text but no icon can match
                  // the height of other toolbar buttons
            }
      }

bool WorkspacesManager::isDefaultWorkspace(Workspace* workspace)
      {
      //returns true if @workspace's name is one of the "Basic/Advanced"
      return std::find(defaultWorkspaces.begin(), defaultWorkspaces.end(), workspace->translatableName()) != defaultWorkspaces.end();
      }

bool WorkspacesManager::isDefaultEditedWorkspace(Workspace* workspace)
      {
      //returns true if @workspace's name is one of the "Basic edited/Advanced edited"
      return std::find(WorkspacesManager::defaultEditedWorkspaces.begin(), WorkspacesManager::defaultEditedWorkspaces.end(), workspace->translatableName()) != WorkspacesManager::defaultEditedWorkspaces.end();
      }

void WorkspacesManager::initCurrentWorkspace()
      {
      initWorkspaces();

      QString workspaceName = preferences.getString(PREF_APP_WORKSPACE);
      m_currentWorkspace = findByName(workspaceName);
      Q_ASSERT(!workspaces().empty());
      if (!m_currentWorkspace) {
          m_currentWorkspace = findByTranslatableName(workspaceName);
          if (!m_currentWorkspace) {
              m_currentWorkspace = workspaces().at(0);
          }
      }
      }

void WorkspacesManager::remove(Workspace* workspace)
      {
      m_workspaces.removeOne(findByName(workspace->name()));
      QFile f(workspace->path());
      f.remove();
      delete workspace;
      isWorkspacesListDirty = true;
      initWorkspaces();
      emit mscore->workspacesChanged();
      }

//---------------------------------------------------------
//   writeFailed
//---------------------------------------------------------

static void writeFailed(const QString& _path)
      {
      QString s = qApp->translate("Workspace", "Writing Workspace File\n%1\nfailed");
      QMessageBox::critical(mscore, qApp->translate("Workspace", "Writing Workspace File"), s.arg(_path));
      }

//---------------------------------------------------------
//   Workspace
//---------------------------------------------------------

Workspace::Workspace()
   : QObject(0)
      {
      _dirty = false;
      _readOnly = false;
      saveComponents = false;
      saveToolbars = false;
      saveMenuBar = false;

      _saveTimer.setInterval(0);
      _saveTimer.setSingleShot(true);
      connect(&_saveTimer, &QTimer::timeout, this, &Workspace::ensureWorkspaceSaved);
      }

QString Workspace::id() const
      {
      return !_translatableName.isEmpty() ? _translatableName : _name;
      }

//---------------------------------------------------------
//   makeUserWorkspacePath
///   Returns path for the workspace with the given \p name
///   creating all the necessary directories.
//---------------------------------------------------------

QString WorkspacesManager::makeUserWorkspacePath(const QString& name)
      {
      const QString ext(".workspace");
      QDir dir;
      dir.mkpath(dataPath);
      QString path(dataPath + "/workspaces");
      dir.mkpath(path);
      path += "/" + name + ext;
      return path;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Workspace::write()
      {
      if (_path.isEmpty())
            _path = WorkspacesManager::makeUserWorkspacePath(_name);

      MQZipWriter f(_path);
      f.setCreationPermissions(
         QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner
         | QFile::ReadUser | QFile::WriteUser | QFile::ExeUser
         | QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup
         | QFile::ReadOther | QFile::WriteOther | QFile::ExeOther);

      if (f.status() != MQZipWriter::NoError) {
            writeFailed(_path);
            return;
            }

      QBuffer cbuf;
      cbuf.open(QIODevice::ReadWrite);
      XmlWriter xml(gscore, &cbuf);
      xml.header();
      xml.stag("container");
      xml.stag("rootfiles");
      xml.stag(QString("rootfile full-path=\"%1\"").arg(XmlWriter::xmlString("workspace.xml")));
      xml.etag();
      for (ImageStoreItem* ip : imageStore) {
            if (!ip->isUsed(gscore))
                  continue;
            QString dstPath = QString("Pictures/") + ip->hashName();
            xml.tag("file", dstPath);
            }
      xml.etag();
      xml.etag();
      cbuf.seek(0);
      f.addFile("META-INF/container.xml", cbuf.data());

      // save images
      for (ImageStoreItem* ip : imageStore) {
            if (!ip->isUsed(gscore))
                  continue;
            QString dstPath = QString("Pictures/") + ip->hashName();
            f.addFile(dstPath, ip->buffer());
            }
      {
      xml.setClipboardmode(true);
      xml.header();
      xml.stag("museScore version=\"" MSC_VERSION "\"");
      xml.stag("Workspace");
      // xml.tag("name", _name);
      if (!_sourceWorkspaceName.isEmpty())
            xml.tag("source", _sourceWorkspaceName);
      const PaletteWorkspace* w = mscore->getPaletteWorkspace();
      w->write(xml);

      // write toolbar settings
      if (saveToolbars) {
            xml.stag("Toolbar name=\"noteInput\"");
            for (auto i : *mscore->noteInputMenuEntries())
                  xml.tag("action", i);
            xml.etag();
            xml.stag("Toolbar name=\"fileOperation\"");
            for (auto i : *mscore->fileOperationEntries())
                  xml.tag("action", i);
            xml.etag();
            xml.stag("Toolbar name=\"playbackControl\"");
            for (auto i : *mscore->playbackControlEntries())
                  xml.tag("action", i);
            xml.etag();
            }
      else {
            writeGlobalToolBar();
            }

      if (preferences.getUseLocalPreferences()) {
            xml.stag("Preferences");
            for (QString pref : preferences.getLocalPreferences().keys()) {
                  QVariant prefValue = preferences.getLocalPreferences().value(pref);
                  if (prefValue.isValid())
                        xml.tag("Preference name=\"" + pref + "\"", preferences.getLocalPreferences().value(pref));
                  }
            xml.etag();
            }

      if (saveMenuBar)
            writeMenuBar(xml);

      if (saveComponents) {
            QByteArray state_64 = mscore->saveState().toBase64();
            QString state(state_64);
            xml.tag("State", state);
            }
      else {
            writeGlobalGUIState();
            }

      xml.etag();
      xml.etag();
      f.addFile("workspace.xml", cbuf.data());
      cbuf.close();
      }

      if (f.status() != MQZipWriter::NoError)
            writeFailed(_path);
      }

//---------------------------------------------------------
//   writeGlobalMenuBar
//   writes global menu bar for workspaces
//---------------------------------------------------------

void Workspace::writeGlobalMenuBar(QMenuBar* mb)
      {
      QString default_path = "";
      QDir dir;
      dir.mkpath(dataPath);
      default_path = dataPath + "/workspaces";
      dir.mkpath(default_path);
      default_path += "/global";
      dir.mkpath(default_path);
      default_path += "/menubar.xml";

      QFile default_menubar (default_path);
      default_menubar.open(QIODevice::WriteOnly);

      if (!default_menubar.exists()) {
            writeFailed(default_path);
            return;
            }

      QBuffer cbuf;
      cbuf.open(QIODevice::ReadWrite);
      XmlWriter xml(gscore, &cbuf);
      xml.setClipboardmode(true);
      xml.header();
      xml.stag("museScore version=\"" MSC_VERSION "\"");

      writeMenuBar(xml, mb);

      xml.etag();
      default_menubar.write(cbuf.data());
      cbuf.close();
      default_menubar.close();
      }

//---------------------------------------------------------
//   writeGlobalToolBar
//   writes global tool bar for workspaces
//---------------------------------------------------------

void Workspace::writeGlobalToolBar()
      {
      QString default_path = "";
      QDir dir;
      dir.mkpath(dataPath);
      default_path = dataPath + "/workspaces";
      dir.mkpath(default_path);
      default_path += "/global";
      dir.mkpath(default_path);
      default_path += "/toolbar.xml";

      QFile default_toolbar (default_path);
      default_toolbar.open(QIODevice::WriteOnly);

      if (!default_toolbar.exists()) {
            writeFailed(default_path);
            return;
            }

      QBuffer cbuf;
      cbuf.open(QIODevice::ReadWrite);
      XmlWriter xml(gscore, &cbuf);
      xml.setClipboardmode(true);
      xml.header();
      xml.stag("museScore version=\"" MSC_VERSION "\"");

      xml.stag("Toolbar name=\"noteInput\"");
      for (auto i : *mscore->noteInputMenuEntries())
            xml.tag("action", i);
      xml.etag();
      xml.stag("Toolbar name=\"fileOperation\"");
      for (auto i : *mscore->fileOperationEntries())
            xml.tag("action", i);
      xml.etag();
      xml.stag("Toolbar name=\"playbackControl\"");
      for (auto i : *mscore->playbackControlEntries())
            xml.tag("action", i);
      xml.etag();

      xml.etag();
      default_toolbar.write(cbuf.data());
      cbuf.close();
      default_toolbar.close();
      }

//---------------------------------------------------------
//   writeGlobalGUIState
//   writes global GUI state for workspaces
//---------------------------------------------------------

void Workspace::writeGlobalGUIState()
      {
      QString default_path = "";
      QDir dir;
      dir.mkpath(dataPath);
      default_path = dataPath + "/workspaces";
      dir.mkpath(default_path);
      default_path += "/global";
      dir.mkpath(default_path);
      default_path += "/guistate.xml";

      QFile default_guistate (default_path);
      default_guistate.open(QIODevice::WriteOnly);

      if (!default_guistate.exists()) {
            writeFailed(default_path);
            return;
            }

      QBuffer cbuf;
      cbuf.open(QIODevice::ReadWrite);
      XmlWriter xml(gscore, &cbuf);
      xml.setClipboardmode(true);
      xml.header();
      xml.stag("museScore version=\"" MSC_VERSION "\"");

      QByteArray state_64 = mscore->saveState().toBase64();
      QString state(state_64);
      xml.tag("State", state);

      xml.etag();
      default_guistate.write(cbuf.data());
      cbuf.close();
      default_guistate.close();
      }

//---------------------------------------------------------
//   writeMenuBar
//---------------------------------------------------------

void Workspace::writeMenuBar(XmlWriter& xml, QMenuBar* mb)
      {
      // Loop through each menu in menubar. For each menu, call writeMenu.
      xml.stag("MenuBar");
      if (!mb)
            mb = mscore->menuBar();
      for (QAction* action : mb->actions()) {
            if (action->isSeparator())
                  xml.tag("action", "");
            else if (action->menu()) {
                  const QString menuString = findStringFromMenu(action->menu());
                  if (!menuString.isEmpty()) {
                        xml.stag("Menu name=\"" + menuString + "\"");
                        writeMenu(xml, action->menu());
                        xml.etag();
                        }
                  }
            else {
                  const QString actionString = findStringFromAction(action);
                  if (!actionString.isEmpty())
                        xml.tag("action", actionString);
                  }

            }
      xml.etag();
      }

//---------------------------------------------------------
//   writeMenu
//---------------------------------------------------------

void Workspace::writeMenu(XmlWriter& xml, QMenu* menu)
      {
      // Recursively save QMenu
      for (QAction* action : menu->actions()) {
            if (action->isSeparator())
                  xml.tag("action", "");
            else if (action->menu()) {
                  const QString menuString = findStringFromMenu(action->menu());
                  if (!menuString.isEmpty()) {
                        xml.stag("Menu name=\"" + menuString + "\"");
                        writeMenu(xml, action->menu());
                        xml.etag();
                        }
                  }
            else {
                  const QString actionString = findStringFromAction(action);
                  if (!actionString.isEmpty())
                        xml.tag("action", actionString);
                  }
            }
      }

extern QString readRootFile(MQZipReader*, QList<QString>&);

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void WorkspacesManager::readWorkspaceFile(const QString& path, std::function<void(XmlReader&)> readWorkspace)
      {
      MQZipReader f(path);
      QList<QString> images;
      QString rootfile = readRootFile(&f, images);
      //
      // load images
      //
      for (const QString& s : images)
            imageStore.add(s, f.fileData(s));

      if (rootfile.isEmpty()) {
            qDebug("can't find rootfile in: %s", qPrintable(path));
            return;
            }

      QByteArray ba = f.fileData(rootfile);
      XmlReader e(ba);
      e.setPasteMode(true);

      while (e.readNextStartElement()) {
            if (e.name() == "museScore") {
                  while (e.readNextStartElement()) {
                        if (e.name() == "Workspace")
                              readWorkspace(e);
                        else
                              e.unknown();
                        }
                  }
            }
      }

void Workspace::read()
      {
      saveToolbars = saveMenuBar = saveComponents = false;
      preferences.setUseLocalPreferences(false);
      if (_path.isEmpty() || !QFile(_path).exists()) {
            qDebug("cannot read workspace <%s>", qPrintable(_path));
            mscore->setDefaultPalette();
            readGlobalMenuBar();
            readGlobalToolBar();
            readGlobalGUIState();
            preferences.updateLocalPreferences();
            return;
            }
      QFileInfo fi(_path);
      qt_ntfs_permission_lookup++;
      _readOnly = !fi.isWritable();
      qt_ntfs_permission_lookup--;

      preferences.updateLocalPreferences();

      WorkspacesManager::readWorkspaceFile(_path, [this](XmlReader& e) { read(e); });
      }

/**
      Reset the workspace to the state of the source workspace.
      Works for Custom workspaces ONLY!
 */
void Workspace::reset()
      {
      saveToolbars = saveMenuBar = saveComponents = false;
      preferences.setUseLocalPreferences(false);
      preferences.updateLocalPreferences();
      const Workspace* srcWorkspace = sourceWorkspace();
      WorkspacesManager::readWorkspaceFile(srcWorkspace->path(), [this](XmlReader& e) { read(e); });
      save();
      }

std::unique_ptr<PaletteTree> Workspace::getPaletteTree() const
      {
      std::unique_ptr<PaletteTree> paletteTree(new PaletteTree);
      WorkspacesManager::readWorkspaceFile(_path, [&](XmlReader& e) {
            while (e.readNextStartElement()) {
                  if (e.name() == "PaletteBox")
                        paletteTree->read(e);
                  else
                        e.skipCurrentElement();
                  }
            });
      return paletteTree;
      }

void Workspace::read(XmlReader& e)
      {
      bool niToolbar = false;
      bool foToolbar = false;
      bool pcToolbar = false;
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "name")
                  e.readElementText();
            else if (tag == "source")
                  _sourceWorkspaceName = e.readElementText();
            else if (tag == "PaletteBox") {
                  PaletteWorkspace* w = mscore->getPaletteWorkspace();
                  w->read(e);
                  }
            else if (tag == "Toolbar") {
                  saveToolbars = true;
                  QString name = e.attribute("name");
                  std::list<const char *> toolbarEntries;
                  if (name == "noteInput")
                        toolbarEntries = mscore->allNoteInputMenuEntries();
                  else if (name == "fileOperation")
                        toolbarEntries = mscore->allFileOperationEntries();
                  else if (name == "playbackControl")
                        toolbarEntries = mscore->allPlaybackControlEntries();
                  else
                        qDebug() << "Error in loading workspace: " + name + " is not a toolbar";

                  std::list<const char*> l;
                  while (e.readNextStartElement()) {
                        const QStringRef& t(e.name());
                        if (t == "action") {
                              QString s = e.readElementText();
                              for (auto k : toolbarEntries) {
                                    if (k == s) {
                                          l.push_back(k);
                                          break;
                                          }
                                    }
                              }
                        else
                              e.unknown();
                        }
                  if (name == "noteInput") {
                        mscore->setNoteInputMenuEntries(l);
                        mscore->populateNoteInputMenu();
                        niToolbar = true;
                        }
                  else if (name == "fileOperation") {
                        mscore->setFileOperationEntries(l);
                        mscore->populateFileOperations();
                        foToolbar = true;
                        }
                  else if (name == "playbackControl") {
                        mscore->setPlaybackControlEntries(l);
                        mscore->populatePlaybackControls();
                        pcToolbar = true;
                        }
                  }
            else if (tag == "Preferences") {
                  preferences.setUseLocalPreferences(true);
                  while (e.readNextStartElement()) {
                        QString preference_name = e.attribute("name");
                        switch (preferences.defaultValue(preference_name).type()) {
                              case QVariant::Int:
                                    {
                                    int new_int = e.readInt();
                                    preferences.setLocalPreference(preference_name, QVariant(new_int));
                                    }
                                    break;
                              case QVariant::Color:
                                    {
                                    QColor new_color = e.readColor();
                                    preferences.setLocalPreference(preference_name, QVariant(new_color));
                                    }
                                    break;
                              case QVariant::String:
                                    {
                                    QString new_string = e.readXml();
                                    preferences.setLocalPreference(preference_name, QVariant(new_string));
                                    }
                                    break;
                              case QVariant::Bool:
                                    {
                                    bool new_bool = e.readBool();
                                    preferences.setLocalPreference(preference_name, QVariant(new_bool));
                                    }
                                    break;
                              case QVariant::LongLong:
                                    {
                                    bool new_longlong = e.readLongLong();
                                    preferences.setLocalPreference(preference_name, QVariant(new_longlong));
                                    break;
                                    }
                              default:
                                    qDebug() << preferences.defaultValue(preference_name).type() << " not handled.";
                                    e.unknown();
                              }
                        }
                  }
            else if (tag == "MenuBar") {
                  saveMenuBar = true;
                  QMenuBar* mb = mscore->menuBar();
                  const QObjectList menus(mb->children()); // need a copy
                  for (QObject* m : menus) {
                        QMenu* menu = qobject_cast<QMenu*>(m);
                        if (menu) {
                              menu->setParent(nullptr);
                              menu->deleteLater();
                              }
                        }
                  mb->clear();
                  menuToStringList.clear();
                  while (e.readNextStartElement()) {
                        if (e.hasAttribute("name")) { // is a menu
                              QString menu_id = e.attribute("name");
                              QMenu* menu = mb->addMenu(menu_id);
                              addMenuAndString(menu, menu_id);
                              readMenu(e, menu);
                              }
                        else { // is an action
                              QString action_id = e.readXml();
                              if (action_id.isEmpty())
                                    mb->addSeparator();
                              else {
                                    QAction* action = findActionFromString(action_id);
                                    mb->addAction(action);
                                    }
                              }
                        }
                  mscore->updateMenus();
                  }
            else if (tag == "State") {
                  saveComponents = true;
                  QString state_string = e.readXml();
                  QByteArray state_byte_array_64(state_string.toUtf8());
                  QByteArray state_byte_array = QByteArray::fromBase64(state_byte_array_64);
                  mscore->restoreState(state_byte_array);
                  }
            else
                  e.unknown();
            }
      if (saveToolbars) {
            if (!niToolbar) {
                  mscore->setNoteInputMenuEntries(mscore->allNoteInputMenuEntries());
                  mscore->populateNoteInputMenu();
                  }
            if (!foToolbar) {
                  mscore->setFileOperationEntries(mscore->allFileOperationEntries());
                  mscore->populateFileOperations();
                  }
            if (!pcToolbar) {
                  mscore->setPlaybackControlEntries(mscore->allPlaybackControlEntries());
                  mscore->populatePlaybackControls();
                  }
            }
      else {
            readGlobalToolBar();
            }
      if (!saveMenuBar)
            readGlobalMenuBar();
      if (!saveComponents)
            readGlobalGUIState();

      if (const Workspace* src = sourceWorkspace())
            mscore->getPaletteWorkspace()->setDefaultPaletteTree(src->getPaletteTree());
      }

//---------------------------------------------------------
//   readMenu
//---------------------------------------------------------

void Workspace::readMenu(XmlReader& e, QMenu* menu)
      {
      while (e.readNextStartElement()) {
            if (e.hasAttribute("name")) { // is a menu
                  QString menu_id = e.attribute("name");
                  QMenu* new_menu = menu->addMenu(menu_id);
                  addMenuAndString(new_menu, menu_id);
                  readMenu(e, new_menu);
                  }
            else { // is an action
                  QString action_id = e.readXml();
                  if (action_id.isEmpty())
                        menu->addSeparator();
                  else {
                        QAction* action = findActionFromString(action_id);
                        menu->addAction(action);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   readGlobalMenuBar
//---------------------------------------------------------

void Workspace::readGlobalMenuBar()
      {
      QString default_path = dataPath + "/workspaces/global/menubar.xml";

      QFile default_menubar(default_path);
      default_menubar.open(QIODevice::ReadOnly);

      QByteArray ba (default_menubar.readAll());
      XmlReader e(ba);
      e.setPasteMode(true);

      while (e.readNextStartElement()) {
            if (e.name() == "museScore") {
                  while (e.readNextStartElement()) {
                        if (e.name() == "MenuBar") {
                              QMenuBar* mb = mscore->menuBar();
                              const QObjectList menus(mb->children()); // need a copy
                              for (QObject* m : menus) {
                                    QMenu* menu = qobject_cast<QMenu*>(m);
                                    if (menu) {
                                          menu->setParent(nullptr);
                                          menu->deleteLater();
                                          }
                                    }
                              mb->clear();
                              menuToStringList.clear();
                              while (e.readNextStartElement()) {
                                    if (e.hasAttribute("name")) { // is a menu
                                          QString menu_id = e.attribute("name");
                                          QMenu* menu = mb->addMenu(menu_id);
                                          addMenuAndString(menu, menu_id);
                                          readMenu(e, menu);
                                          }
                                    else { // is an action
                                          QString action_id = e.readXml();
                                          if (action_id.isEmpty())
                                                mb->addSeparator();
                                          else {
                                                QAction* action = findActionFromString(action_id);
                                                mb->addAction(action);
                                                }
                                          }
                                    }
                              mscore->updateMenus();
                              }
                        else
                              e.unknown();
                        }
                  }
            }
      }

//---------------------------------------------------------
//   readGlobalToolBar
//---------------------------------------------------------

void Workspace::readGlobalToolBar()
      {
      QString default_path = dataPath + "/workspaces/global/toolbar.xml";

      QFile default_toolbar(default_path);
      default_toolbar.open(QIODevice::ReadOnly);

      QByteArray ba (default_toolbar.readAll());
      XmlReader e(ba);
      e.setPasteMode(true);

      while (e.readNextStartElement()) {
            if (e.name() == "museScore") {
                  while (e.readNextStartElement()) {
                        if (e.name() == "ToolBar") {
                              QString name = e.attribute("name");
                              std::list<const char *> toolbarEntries;
                              if (name == "noteInput")
                                    toolbarEntries = mscore->allNoteInputMenuEntries();
                              else if (name == "fileOperation")
                                    toolbarEntries = mscore->allFileOperationEntries();
                              else if (name == "playbackControl")
                                    toolbarEntries = mscore->allPlaybackControlEntries();
                              else
                                    qDebug() << "Error in loading workspace: " + name + " is not a toolbar";

                              std::list<const char*> l;
                              while (e.readNextStartElement()) {
                                    const QStringRef& tag(e.name());
                                    if (tag == "action") {
                                          QString s = e.readElementText();
                                          for (auto k : toolbarEntries) {
                                                if (k == s) {
                                                      l.push_back(k);
                                                      break;
                                                      }
                                                }
                                          }
                                    else
                                          e.unknown();
                                    }
                              if (name == "noteInput") {
                                    mscore->setNoteInputMenuEntries(l);
                                    mscore->populateNoteInputMenu();
                                    }
                              else if (name == "fileOperation") {
                                    mscore->setFileOperationEntries(l);
                                    mscore->populateFileOperations();
                                    }
                              else if (name == "playbackControl") {
                                    mscore->setPlaybackControlEntries(l);
                                    mscore->populatePlaybackControls();
                                    }
                              }
                        else
                              e.unknown();
                        }
                  }
            }
      }

//---------------------------------------------------------
//   readGlobalGUIState
//---------------------------------------------------------

void Workspace::readGlobalGUIState()
      {
      QString default_path = dataPath + "/workspaces/global/guistate.xml";

      QFile default_toolbar(default_path);
      default_toolbar.open(QIODevice::ReadOnly);

      QByteArray ba (default_toolbar.readAll());
      XmlReader e(ba);
      e.setPasteMode(true);

      while (e.readNextStartElement()) {
            if (e.name() == "museScore") {
                  while (e.readNextStartElement()) {
                        if (e.name() == "State") {
                              QString state_string = e.readXml();
                              QByteArray state_byte_array_64(state_string.toUtf8());
                              QByteArray state_byte_array = QByteArray::fromBase64(state_byte_array_64);
                              mscore->restoreState(state_byte_array);
                              }
                        else
                              e.unknown();
                        }
                  }
            }
      }

//---------------------------------------------------------
//   ensureWorkspaceSaved
//---------------------------------------------------------

void Workspace::ensureWorkspaceSaved()
      {
      if (!_dirty)
            return;

      if (_readOnly) {
            setTranslatableName(editedWorkspaceTranslatableName(translatableName()));

            if (translatableName().isEmpty()) {
                  /*: Name of the edited read-only workspace, %1 is replaced with the old workspace name */
                  setName(tr("%1 edited").arg(name()));
                  }
            else
                  setName(tr(translatableName().toUtf8()));

            _path = WorkspacesManager::makeUserWorkspacePath(translatableName().isEmpty() ? name() : translatableName());

            write();

            const QFileInfo fi(_path);
            qt_ntfs_permission_lookup++;
            _readOnly = !fi.isWritable();
            qt_ntfs_permission_lookup--;
            Q_ASSERT(!_readOnly);

            WorkspacesManager::refreshWorkspaces();
            preferences.setPreference(PREF_APP_WORKSPACE, id());
            emit mscore->workspacesChanged();
            }
      else
            write();
      }

//---------------------------------------------------------
//   setDirty
//---------------------------------------------------------

void Workspace::setDirty(bool val)
      {
      _dirty = val;
      _saveTimer.start();
      }

//---------------------------------------------------------
//   save
//---------------------------------------------------------

void Workspace::save()
      {
      if (!saveComponents)
            writeGlobalGUIState();
      if (!saveToolbars)
            writeGlobalToolBar();

      if (_readOnly)
            return;

      write();
      }

static QStringList findWorkspaceFiles()
      {
      QStringList path;
      path << mscoreGlobalShare + "workspaces";
      path << dataPath + "/workspaces";

      QStringList extensionsDir = Extension::getDirectoriesByType(Extension::workspacesDir);
      path.append(extensionsDir);

      QStringList nameFilters;
      nameFilters << "*.workspace";

      QStringList workspaces;

      for (const QString& s : path) {
            QDir dir(s);
            QStringList pl = dir.entryList(nameFilters, QDir::Files, QDir::Name);

            for (const QString& entry : pl) {
                  const QString workspacePath(s + "/" + entry);
                  workspaces << workspacePath;
                  }
            }

      return workspaces;
      }

void WorkspacesManager::initWorkspaces()
      {
      if (!isWorkspacesListDirty)
            return;
      
      QList<Workspace*> oldWorkspaces(m_workspaces);
      QList<Workspace*> editedWorkpaces;
      
      for (const QString& path : findWorkspaceFiles()) {
            Workspace* p = 0;
            QFileInfo fi(path);
            QString name(fi.completeBaseName());

            const bool isDefault = std::find(WorkspacesManager::defaultWorkspaces.begin(), WorkspacesManager::defaultWorkspaces.end(), name) != WorkspacesManager::defaultWorkspaces.end();
            const bool isEditedDefault = std::find(WorkspacesManager::defaultEditedWorkspaces.begin(), WorkspacesManager::defaultEditedWorkspaces.end(), name) != WorkspacesManager::defaultEditedWorkspaces.end();

            const bool translate = isDefault || isEditedDefault;

            for (Workspace* w : m_workspaces) {
                  if (w->name() == name || (translate && w->translatableName() == name)) {
                        p = w;
                        break;
                        }
                  }

            if (p)
                  oldWorkspaces.removeOne(p);
            else {
                  p = new Workspace;
                  m_workspaces.append(p);
                  }

            p->setPath(path);
            p->setName(name);

            if (translate)
                  p->setTranslatableName(name);

            qt_ntfs_permission_lookup++;
            p->setReadOnly(!fi.isWritable());
            qt_ntfs_permission_lookup--;

            if (isEditedDefault)
                  editedWorkpaces.push_back(p);
            }

      for (Workspace* old : oldWorkspaces)
            m_workspaces.removeOne(old);

      if (m_workspaces.empty())
            qFatal("No workspaces found");

      if (oldWorkspaces.contains(WorkspacesManager::currentWorkspace()))
            WorkspacesManager::setCurrentWorkspace(m_workspaces.first());

      qDeleteAll(oldWorkspaces);

      // hack
      for (int i = 0; i < m_workspaces.size(); i++) {
            const QString& trName = m_workspaces[i]->translatableName();
            if (trName == WorkspacesManager::defaultWorkspaces[0] || trName == WorkspacesManager::defaultEditedWorkspaces[0]) {
                  m_workspaces.move(i, 0);
                  break;
                  }
            }
      
      retranslate(m_workspaces);
      
      // Delete default workspaces if there are corresponding user-edited ones
      m_visibleWorkspaces = m_workspaces;
      for (Workspace* ew : editedWorkpaces) {
            const QString uneditedName = defaultWorkspaceTranslatableName(ew->translatableName());
            if (uneditedName.isEmpty())
                  continue;

            for (auto it = m_visibleWorkspaces.begin(); it != m_visibleWorkspaces.end(); ++it) {
                  Workspace* w = *it;
                  if (w->translatableName() == uneditedName) {
                        m_visibleWorkspaces.erase(it);
                        break;
                        }
                  }
            }
      
      isWorkspacesListDirty = false;
      }

//---------------------------------------------------------
//   refreshWorkspaces
//---------------------------------------------------------

void WorkspacesManager::refreshWorkspaces()
      {
      isWorkspacesListDirty = true;
      initWorkspaces();
      }

const Workspace* Workspace::sourceWorkspace() const
      {
      const QString sourceName = _sourceWorkspaceName.isEmpty() ? WorkspacesManager::defaultWorkspaces[0] : _sourceWorkspaceName;

      if (translatableName() == sourceName || name() == sourceName)
            return this;

      Workspace* sourceWorkspace = WorkspacesManager::findByName(sourceName);
      if (!sourceWorkspace)
            sourceWorkspace = WorkspacesManager::findByTranslatableName(sourceName);
      
      return !!sourceWorkspace ? sourceWorkspace : WorkspacesManager::workspaces()[0];
      }

void WorkspacesManager::retranslate(QList<Workspace*>& workspacesList)
      {
      for (auto w : workspacesList) {
            if (!w->translatableName().isEmpty()) {
                  auto transName = qApp->translate("Ms::Workspace", w->translatableName().toLatin1().constData());
                  w->setName(transName);
                  }
            }
      }

void WorkspacesManager::retranslateAll()
      {
      retranslate(m_workspaces);
      }

//---------------------------------------------------------
//   createNewWorkspace
//---------------------------------------------------------

Workspace* WorkspacesManager::createNewWorkspace(const QString& name)
      {
      Workspace* w = new Workspace;
      w->setName(name);
      w->setPath("");
      w->setDirty(false);
      w->setReadOnly(false);
      w->write();
      w->setSourceWorkspaceName(WorkspacesManager::currentWorkspace()->sourceWorkspaceName());

      m_workspaces.append(w);
      m_visibleWorkspaces.append(w);
      return w;
      }

//---------------------------------------------------------
//   clearWorkspaces
//---------------------------------------------------------

void WorkspacesManager::clearWorkspaces()
      {
      m_currentWorkspace = nullptr;
      for (Workspace* w : m_workspaces)
            w->deleteLater();
      m_workspaces.clear();
      m_visibleWorkspaces.clear();
      isWorkspacesListDirty = true;
      }

//---------------------------------------------------------
//   addActionAndString
//---------------------------------------------------------

void Workspace::addActionAndString(QAction* action, QString string)
      {
      QPair<QAction*, QString> pair;
      pair.first = action;
      pair.second = string;
      actionToStringList.append(pair);
      }

//---------------------------------------------------------
//   addRemainingFromMenuBar
//---------------------------------------------------------

void Workspace::addRemainingFromMenuBar(QMenuBar* mb)
      {
      // Loop through each menu in menubar. For each menu, call writeMenu.
      for (QAction* action : mb->actions()) {
            if (action->isSeparator())
                  continue;
            else if (action->menu())
                  addRemainingFromMenu(action->menu());
            else if (!action->data().toString().isEmpty())
                  addActionAndString(action, action->data().toString());
            }
      }

//---------------------------------------------------------
//   addRemainingFromMenu
//---------------------------------------------------------

void Workspace::addRemainingFromMenu(QMenu* menu)
      {
      // Recursively save QMenu
      for (QAction* action : menu->actions()) {
            if (action->isSeparator())
                  continue;
            else if (action->menu())
                  addRemainingFromMenu(action->menu());
            else if (!action->data().toString().isEmpty())
                  addActionAndString(action, action->data().toString());
            }
      }

//---------------------------------------------------------
//   findActionFromString
//---------------------------------------------------------

QAction* Workspace::findActionFromString(QString string)
      {
      for (auto pair : actionToStringList) {
            if (pair.second == string)
                  return pair.first;
            }
      return 0;
      }

//---------------------------------------------------------
//   findStringFromAction
//---------------------------------------------------------

QString Workspace::findStringFromAction(QAction* action)
      {
      for (auto pair : actionToStringList) {
            if (pair.first == action)
                  return pair.second;
            }
      return 0;
      }

//---------------------------------------------------------
//   addMenuAndString
//---------------------------------------------------------

void Workspace::addMenuAndString(QMenu* menu, QString string)
      {
      QPair<QMenu*, QString> pair;
      pair.first = menu;
      pair.second = string;
      menuToStringList.append(pair);
      }

//---------------------------------------------------------
//   findMenuFromString
//---------------------------------------------------------

QMenu* Workspace::findMenuFromString(QString string)
      {
      for (auto pair : menuToStringList) {
            if (pair.second == string)
                  return pair.first;
            }
      return 0;
      }

//---------------------------------------------------------
//   findStringFromMenu
//---------------------------------------------------------

QString Workspace::findStringFromMenu(QMenu* menu)
      {
      for (auto pair : menuToStringList) {
            if (pair.first == menu)
                  return pair.second;
            }
      return 0;
      }

//---------------------------------------------------------
//   rename
//---------------------------------------------------------

void Workspace::rename(const QString& s)
      {
      const QString newPath = WorkspacesManager::makeUserWorkspacePath(s);

      QFile file(_path);
      if (file.exists())
            file.rename(newPath);

      setName(s);
      _path = newPath;
      save();
      }
}
