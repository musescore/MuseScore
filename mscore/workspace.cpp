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
Workspace* Workspace::currentWorkspace;

Workspace Workspace::_advancedWorkspace {
      QT_TR_NOOP("Advanced"), QString("Advanced"), false, true
      };

Workspace Workspace::_basicWorkspace {
      QT_TR_NOOP("Basic"), QString("Basic"), false, true
      };

QList<Workspace*> Workspace::_workspaces {
      &_basicWorkspace,
      &_advancedWorkspace
      };

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
//   createNewWorkspace
//---------------------------------------------------------

void MuseScore::createNewWorkspace()
      {
      QString s = QInputDialog::getText(this, tr("Read Workspace Name"),
         tr("Workspace name:"));
      if (s.isEmpty())
            return;
      s = s.replace( QRegExp( "[" + QRegExp::escape( "\\/:*?\"<>|" ) + "]" ), "_" ); //FAT/NTFS special chars
      for (;;) {
            bool notFound = true;
            for (Workspace* p : Workspace::workspaces()) {
                  if ((qApp->translate("Ms::Workspace", p->name().toUtf8()).toLower() == s.toLower()) ||
                     (s.toLower() == QString("basic")) || (s.toLower() == QString("advanced"))) {
                        notFound = false;
                        break;
                        }
                  }
            if (!notFound) {
                  s = QInputDialog::getText(this,
                     tr("Read Workspace Name"),
                     tr("'%1' does already exist,\nplease choose a different name:").arg(s)
                     );
                  if (s.isEmpty())
                        return;
                  s = s.replace( QRegExp( "[" + QRegExp::escape( "\\/:*?\"<>|" ) + "]" ), "_" ); //FAT/NTFS special chars
                  }
            else
                  break;
            }
      if (Workspace::currentWorkspace->dirty())
            Workspace::currentWorkspace->save();
      Workspace::currentWorkspace = Workspace::createNewWorkspace(s);
      preferences.setPreference(PREF_APP_WORKSPACE, Workspace::currentWorkspace->name());
      PaletteBox* paletteBox = mscore->getPaletteBox();
      paletteBox->updateWorkspaces();
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
            if (p->name() == a->text()) { // no need for qApp->translate since "Basic" and "Advanced" are not deletable
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
      PaletteBox* paletteBox = mscore->getPaletteBox();
      paletteBox->clear();
      Workspace::currentWorkspace = Workspace::workspaces().first();
      preferences.setPreference(PREF_APP_WORKSPACE, Workspace::currentWorkspace->name());
      changeWorkspace(Workspace::currentWorkspace);
      paletteBox = mscore->getPaletteBox();
      paletteBox->updateWorkspaces();
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
                  PaletteBox* paletteBox = mscore->getPaletteBox();
                  paletteBox->updateWorkspaces();
                  return;
                  }
            }
      qDebug("   workspace \"%s\" not found", qPrintable(a->text()));
      }

//---------------------------------------------------------
//   changeWorkspace
//---------------------------------------------------------

void MuseScore::changeWorkspace(Workspace* p)
      {
      Workspace::currentWorkspace->save();
      p->read();
      Workspace::currentWorkspace = p;
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
      QBuffer cbuf;
      cbuf.open(QIODevice::ReadWrite);
      XmlWriter xml(gscore, &cbuf);
      xml.setClipboardmode(true);
      xml.header();
      xml.stag("museScore version=\"" MSC_VERSION "\"");
      xml.stag("Workspace");
      // xml.tag("name", _name);
      PaletteBox* pb = mscore->getPaletteBox();
      pb->write(xml);

      // write toolbar settings
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
      xml.etag();
      f.addFile("workspace.xml", cbuf.data());
      cbuf.close();
      }

      if (f.status() != MQZipWriter::NoError)
            writeFailed(_path);
      }

extern QString readRootFile(MQZipReader*, QList<QString>&);

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Workspace::read()
      {
      if (_path == "Advanced") {
            mscore->setAdvancedPalette();
            for (Palette* p : mscore->getPaletteBox()->palettes())
                  p->setSystemPalette(true);
            mscore->setNoteInputMenuEntries(MuseScore::advancedNoteInputMenuEntries());
            mscore->populateNoteInputMenu();
            mscore->setFileOperationEntries(mscore->allFileOperationEntries());
            mscore->populateFileOperations();
            mscore->setPlaybackControlEntries(mscore->allPlaybackControlEntries());
            mscore->populatePlaybackControls();
            return;
            }
      if (_path == "Basic") {
            mscore->setBasicPalette();
            for (Palette* p : mscore->getPaletteBox()->palettes())
                  p->setSystemPalette(true);
            mscore->setNoteInputMenuEntries(MuseScore::basicNoteInputMenuEntries());
            mscore->populateNoteInputMenu();
            mscore->setFileOperationEntries(mscore->allFileOperationEntries());
            mscore->populateFileOperations();
            mscore->setPlaybackControlEntries(mscore->allPlaybackControlEntries());
            mscore->populatePlaybackControls();
            return;
            }
      if (_path.isEmpty() || !QFile(_path).exists()) {
            qDebug("cannot read workspace <%s>", qPrintable(_path));
            mscore->setAdvancedPalette();       // set default palette
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
            else
                  e.unknown();
            }
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

//---------------------------------------------------------
//   save
//---------------------------------------------------------

void Workspace::save()
      {
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
            QMutableListIterator<Workspace*> i(_workspaces);
            int index = 0;
            while (i.hasNext()) {
                  Workspace* w = i.next();
                  if (index >= 2) {
                        delete w;
                        i.remove();
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
                        p->setReadOnly(!fi.isWritable());
                        _workspaces.append(p);
                        }
                  }
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
//   createNewWorkspace
//---------------------------------------------------------

Workspace* Workspace::createNewWorkspace(const QString& name)
      {
      Workspace* p = new Workspace;
      p->setName(name);
      p->setPath("");
      p->setDirty(false);
      p->setReadOnly(false);
      p->write();

      // all palettes in new workspace are editable

      PaletteBox* paletteBox = mscore->getPaletteBox();
      QList<Palette*> pl = paletteBox->palettes();
      for (Palette* p : pl) {
            p->setSystemPalette(false);
            for (int i = 0; i < p->size(); ++i)
                  p->setCellReadOnly(i, false);
            }

      _workspaces.append(p);
      return p;
      }

}

