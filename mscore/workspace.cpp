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
#include "palettebox.h"
#include "extension.h"

namespace Ms {

bool Workspace::workspacesRead = false;
//std::unordered_map<std::string, QVariant> Workspace::localPreferences {};
Workspace* Workspace::currentWorkspace = nullptr;

QList<Workspace*> Workspace::_workspaces {};

QList<QPair<QAction*, QString>> Workspace::actionToStringList {};
QList<QPair<QMenu*  , QString>> Workspace::menuToStringList   {};


//---------------------------------------------------------
//   undoWorkspace
//---------------------------------------------------------

void MuseScore::undoWorkspace()
      {
      Workspace::currentWorkspace->read();
      Workspace::currentWorkspace->setDirty(false);
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

      const QList<Workspace*> pl = Workspace::workspaces();
      for (Workspace* p : pl) {
            QAction* a = workspaces->addAction(qApp->translate("Ms::Workspace", p->name().toUtf8()));
            a->setCheckable(true);
            a->setData(p->path());
            a->setChecked(p->name() == preferences.getString(PREF_APP_WORKSPACE));
            menuWorkspaces->addAction(a);
            }

      menuWorkspaces->addSeparator();
      QAction* a = new QAction(tr("New..."), this);
      connect(a, SIGNAL(triggered()), SLOT(createNewWorkspace()));
      menuWorkspaces->addAction(a);

      a = new QAction(tr("Edit"), this);
      a->setDisabled(Workspace::currentWorkspace->readOnly());
      connect(a, SIGNAL(triggered()), SLOT(editWorkspace()));
      menuWorkspaces->addAction(a);

      a = new QAction(tr("Delete"), this);
      a->setDisabled(Workspace::currentWorkspace->readOnly());
      connect(a, SIGNAL(triggered()), SLOT(deleteWorkspace()));
      menuWorkspaces->addAction(a);

      a = new QAction(tr("Undo Changes"), this);
      a->setDisabled(Workspace::currentWorkspace->readOnly());
      connect(a, SIGNAL(triggered()), SLOT(undoWorkspace()));
      menuWorkspaces->addAction(a);
      }

//---------------------------------------------------------
//   deleteWorkspace
//---------------------------------------------------------

void MuseScore::deleteWorkspace()
      {
      if (!workspaces)
            return;
      QAction* a = workspaces->checkedAction();
      if (!a)
            return;
      Workspace* workspace = 0;
      for (Workspace* p : Workspace::workspaces()) {
            if (p->name() == a->text()) {
                  workspace = p;
                  break;
                  }
            }
      if (!workspace)
            return;

      QMessageBox::StandardButton reply;
      reply = QMessageBox::question(0,
                 QWidget::tr("Are you sure?"),
                 QWidget::tr("Do you really want to delete the '%1' workspace?").arg(workspace->name()),
                 QMessageBox::Yes | QMessageBox::No,
                 QMessageBox::Yes
                 );
      if (reply != QMessageBox::Yes)
            return;

      Workspace::workspaces().removeOne(workspace);
      QFile f(workspace->path());
      f.remove();
      delete workspace;
      PaletteBox* pb = mscore->getPaletteBox();
      pb->clear();
      Workspace::currentWorkspace = Workspace::workspaces().first();
      preferences.setPreference(PREF_APP_WORKSPACE, Workspace::currentWorkspace->name());
      changeWorkspace(Workspace::currentWorkspace);
      pb = mscore->getPaletteBox();
      pb->updateWorkspaces();
      updateIcons();
      }

//---------------------------------------------------------
//   changeWorkspace
//---------------------------------------------------------

void MuseScore::changeWorkspace(QAction* a)
      {
      for (Workspace* p :Workspace::workspaces()) {
            if (qApp->translate("Ms::Workspace", p->name().toUtf8()) == a->text()) {
                  changeWorkspace(p);
                  preferences.setPreference(PREF_APP_WORKSPACE, Workspace::currentWorkspace->name());
                  PaletteBox* pb = mscore->getPaletteBox();
                  pb->updateWorkspaces();
                  updateIcons();
                  return;
                  }
            }
      qDebug("   workspace \"%s\" not found", qPrintable(a->text()));
      }

//---------------------------------------------------------
//   changeWorkspace
//---------------------------------------------------------

void MuseScore::changeWorkspace(Workspace* p, bool first)
      {
      Workspace::currentWorkspace->save();
      p->read();
      Workspace::currentWorkspace = p;
      if (!first) {
            updateIcons();
            preferencesChanged(true);
            }
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
      }

//---------------------------------------------------------
//   initWorkspace
//---------------------------------------------------------

void Workspace::initWorkspace()
      {
      for (Workspace* p : Workspace::workspaces()) {
            if (p->name() == preferences.getString(PREF_APP_WORKSPACE)) {
                  currentWorkspace = p;
                  break;
                  }
            }
      Q_ASSERT(!Workspace::workspaces().empty());
      if (currentWorkspace == 0)
            currentWorkspace = Workspace::workspaces().at(0);
      }

//---------------------------------------------------------
//   writeFailed
//---------------------------------------------------------

static void writeFailed(const QString& _path)
      {
      QString s = qApp->translate("Workspace", "Writing Workspace File\n%1\nfailed: ");
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
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Workspace::write()
      {
      if (_path.isEmpty()) {
            QString ext(".workspace");
            QDir dir;
            dir.mkpath(dataPath);
            _path = dataPath + "/workspaces";
            dir.mkpath(_path);
            _path += "/" + _name + ext;
            }
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
      PaletteBox* pb = mscore->getPaletteBox();
      pb->write(xml);

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
                  xml.stag("Menu name=\"" + findStringFromMenu(action->menu()) + "\"");
                  writeMenu(xml, action->menu());
                  xml.etag();
                  }
            else
                  xml.tag("action", findStringFromAction(action));

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
                  xml.stag("Menu name=\"" + findStringFromMenu(action->menu()) + "\"");
                  writeMenu(xml, action->menu());
                  xml.etag();
                  }
            else {
                  xml.tag("action", findStringFromAction(action));
                  }
            }
      }

extern QString readRootFile(MQZipReader*, QList<QString>&);

//---------------------------------------------------------
//   read
//---------------------------------------------------------

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
      _readOnly = !fi.isWritable();

      MQZipReader f(_path);
      QList<QString> images;
      QString rootfile = readRootFile(&f, images);
      //
      // load images
      //
      for (const QString& s : images)
            imageStore.add(s, f.fileData(s));

      if (rootfile.isEmpty()) {
            qDebug("can't find rootfile in: %s", qPrintable(_path));
            return;
            }

      QByteArray ba = f.fileData(rootfile);
      XmlReader e(ba);

      preferences.updateLocalPreferences();

      while (e.readNextStartElement()) {
            if (e.name() == "museScore") {
                  while (e.readNextStartElement()) {
                        if (e.name() == "Workspace")
                              read(e);
                        else
                              e.unknown();
                        }
                  }
            }
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
            else if (tag == "PaletteBox") {
                  PaletteBox* paletteBox = mscore->getPaletteBox();
                  paletteBox->clear();
                  paletteBox->read(e);
                  QList<Palette*> pl = paletteBox->palettes();
                  foreach (Palette* p, pl) {
                        p->setSystemPalette(_readOnly);
                        connect(paletteBox, SIGNAL(changed()), SLOT(setDirty()));
                        }
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
                              default:
                                    qDebug() << preferences.defaultValue(preference_name).type() << " not handled.";
                                    e.unknown();
                              }
                        }
                  }
            else if (tag == "MenuBar") {
                  saveMenuBar = true;
                  QMenuBar* mb = mscore->menuBar();
                  mb->clear();
                  while (e.readNextStartElement()) {
                        if (e.hasAttribute("name")) { // is a menu
                              QString menu_id = e.attribute("name");
                              QMenu* menu = findMenuFromString(menu_id);
                              if (menu) {
                                    menu->clear();
                                    mb->addMenu(menu);
                                    readMenu(e, menu);
                                    }
                              else {
                                    menu = new QMenu(menu_id);
                                    mb->addMenu(menu);
                                    readMenu(e, menu);
                                    }
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
      }

//---------------------------------------------------------
//   readMenu
//---------------------------------------------------------

void Workspace::readMenu(XmlReader& e, QMenu* menu)
      {
      while (e.readNextStartElement()) {
            if (e.hasAttribute("name")) { // is a menu
                  QString menu_id = e.attribute("name");
                  QMenu* new_menu = findMenuFromString(menu_id);
                  if (new_menu) {
                        new_menu->clear();
                        menu->addMenu(new_menu);
                        readMenu(e, new_menu);
                        }
                  else {
                        new_menu = new QMenu(menu_id);
                        menu->addMenu(new_menu);
                        readMenu(e, new_menu);
                        }
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

      while (e.readNextStartElement()) {
            if (e.name() == "museScore") {
                  while (e.readNextStartElement()) {
                        if (e.name() == "MenuBar") {
                              QMenuBar* mb = mscore->menuBar();
                              mb->clear();
                              while (e.readNextStartElement()) {
                                    if (e.hasAttribute("name")) { // is a menu
                                          QString menu_id = e.attribute("name");
                                          QMenu* menu = findMenuFromString(menu_id);
                                          if (menu) {
                                                menu->clear();
                                                mb->addMenu(menu);
                                                readMenu(e, menu);
                                                }
                                          else {
                                                menu = new QMenu(menu_id);
                                                mb->addMenu(menu);
                                                readMenu(e, menu);
                                                }
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
      PaletteBox* pb = mscore->getPaletteBox();
      if (pb)
            write();
      }

//---------------------------------------------------------
//   workspaces
//---------------------------------------------------------

QList<Workspace*>& Workspace::workspaces()
      {
      if (!workspacesRead) {
            // Remove all workspaces but Basic and Advanced
            QMutableListIterator<Workspace*> it(_workspaces);
            int index = 0;
            while (it.hasNext()) {
                  Workspace* w = it.next();
                  if (index >= 2) {
                        delete w;
                        it.remove();
                        }
                  index++;
                  }
            QStringList path;
            path << mscoreGlobalShare + "workspaces";
            path << dataPath + "/workspaces";

            QStringList extensionsDir = Extension::getDirectoriesByType(Extension::workspacesDir);
            path.append(extensionsDir);

            QStringList nameFilters;
            nameFilters << "*.workspace";

            for (const QString& s : path) {
                  QDir dir(s);
                  bool translate = (s == mscoreGlobalShare + "workspaces");
                  QStringList pl = dir.entryList(nameFilters, QDir::Files, QDir::Name);

                  foreach (const QString& entry, pl) {
                        Workspace* p = 0;
                        QFileInfo fi(s + "/" + entry);
                        QString name(fi.completeBaseName());
                        for (Workspace* w : _workspaces) {
                              if (w->name() == name) {
                                    p = w;
                                    break;
                                    }
                              }
                        if (!p)
                              p = new Workspace;
                        p->setPath(s + "/" + entry);
                        p->setName(name);
                        if (translate)
                              p->setTranslatableName(name);
                        p->setReadOnly(!fi.isWritable());
                        _workspaces.append(p);
                        }
                  }
            // hack
            for (int i = 0; i < _workspaces.size(); i++) {
                  if (_workspaces[i]->translatableName() == "Basic") {
                        _workspaces.move(i, 0);
                        break;
                        }
                  }
            retranslate(&_workspaces);
            workspacesRead = true;
            }

      return _workspaces;
      }

//---------------------------------------------------------
//   refreshWorkspaces
//---------------------------------------------------------

QList<Workspace*>& Workspace::refreshWorkspaces()
      {
      workspacesRead = false;
      return workspaces();
      }

//---------------------------------------------------------
//   retranslate
//---------------------------------------------------------

void Workspace::retranslate(QList<Workspace*>* workspacesList)
      {
      if (!workspacesList)
            workspacesList = &workspaces();
      for (auto w : *workspacesList) {
            if (!w->translatableName().isEmpty())
                  w->setName(tr(w->translatableName().toLatin1().constData()));
            }
      }

//---------------------------------------------------------
//   createNewWorkspace
//---------------------------------------------------------

Workspace* Workspace::createNewWorkspace(const QString& name)
      {
      Workspace* w = new Workspace;
      w->setName(name);
      w->setPath("");
      w->setDirty(false);
      w->setReadOnly(false);
      w->write();

      // all palettes in new workspace are editable

      PaletteBox* paletteBox = mscore->getPaletteBox();
      QList<Palette*> pl = paletteBox->palettes();
      for (Palette* p : pl) {
            p->setSystemPalette(false);
            for (int i = 0; i < p->size(); ++i)
                  p->setCellReadOnly(i, false);
            }

      _workspaces.append(w);
      return w;
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
      QFile file (_path);
      file.remove();
      setName(s);
      _path = "";
      save();
      }
}
