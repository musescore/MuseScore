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
#include "libmscore/qzipreader_p.h"
#include "libmscore/qzipwriter_p.h"
#include "preferences.h"
#include "palette.h"
#include "palettebox.h"

namespace Ms {

bool Workspace::workspacesRead = false;
QList<Workspace*> Workspace::_workspaces;
Workspace* Workspace::currentWorkspace;

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
            foreach(QAction* a, workspaces->actions())
                  workspaces->removeAction(a);
            }
      menuWorkspaces->clear();

      const QList<Workspace*> pl = Workspace::workspaces();
      foreach (Workspace* p, pl) {
            QAction* a = workspaces->addAction(p->name());
            a->setCheckable(true);
            a->setData(p->path());
            a->setChecked(a->text() == preferences.workspace);
            menuWorkspaces->addAction(a);
            }

      menuWorkspaces->addSeparator();
      QAction* a = new QAction(tr("New"), this);
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
      QString s = QInputDialog::getText(this, tr("MuseScore: Read Workspace Name"),
         tr("Workspace Name:"));
      if (s.isEmpty())
            return;
      for (;;) {
            bool notFound = true;
            foreach(Workspace* p, Workspace::workspaces()) {
                  if (p->name() == s) {
                        notFound = false;
                        break;
                        }
                  }
            if (!notFound) {
                  s = QInputDialog::getText(this,
                     tr("MuseScore: Read Workspace Name"),
                     QString(tr("'%1' does already exist,\nplease choose a different name:")).arg(s)
                     );
                  if (s.isEmpty())
                        return;
                  }
            else
                  break;
            }
      if (Workspace::currentWorkspace->dirty())
            Workspace::currentWorkspace->save();
      Workspace::currentWorkspace = Workspace::createNewWorkspace(s);
      preferences.workspace = Workspace::currentWorkspace->name();
      preferences.dirty     = true;
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
      preferences.dirty = true;
      Workspace* workspace = 0;
      foreach(Workspace* p, Workspace::workspaces()) {
            if (p->name() == a->text()) {
                  workspace = p;
                  break;
                  }
            }
      if (!workspace)
            return;
      Workspace::workspaces().removeOne(workspace);
      QFile f(workspace->path());
      f.remove();
      delete workspace;
      Workspace::currentWorkspace = Workspace::workspaces().first();
      preferences.workspace = Workspace::currentWorkspace->name();
      }

//---------------------------------------------------------
//   changeWorkspace
//---------------------------------------------------------

void MuseScore::changeWorkspace(QAction* a)
      {
      preferences.workspace = a->text();
      preferences.dirty = true;
      foreach(Workspace* p, Workspace::workspaces()) {
            if (p->name() == a->text()) {
                  changeWorkspace(p);
                  return;
                  }
            }
      qDebug("   workspace not found");
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
      foreach(Workspace* p, Workspace::workspaces()) {
            if (p->name() == preferences.workspace) {
                  currentWorkspace = p;
                  break;
                  }
            }
      if (currentWorkspace == 0) {
            currentWorkspace = new Workspace;
            currentWorkspace->setName("default");
            Workspace::workspaces().append(currentWorkspace);
            }
      }

//---------------------------------------------------------
//   writeFailed
//---------------------------------------------------------

static void writeFailed(const QString& _path)
      {
      QString s = qApp->translate("Workspace", "Open Workspace File\n") + _path
         + qApp->translate("Workspace", "\nfailed: ");
      QMessageBox::critical(mscore, qApp->translate("Workspace", "MuseScore: Writing Workspace file"), s);
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
      Xml xml(&cbuf);
      xml.header();
      xml.stag("container");
      xml.stag("rootfiles");
      xml.stag(QString("rootfile full-path=\"%1\"").arg(Xml::xmlString("workspace.xml")));
      xml.etag();
      foreach (ImageStoreItem* ip, imageStore) {
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
      foreach(ImageStoreItem* ip, imageStore) {
            if (!ip->isUsed(gscore))
                  continue;
            QString dstPath = QString("Pictures/") + ip->hashName();
            f.addFile(dstPath, ip->buffer());
            }
      {
      QBuffer cbuf;
      cbuf.open(QIODevice::ReadWrite);
      Xml xml(&cbuf);
      xml.clipboardmode = true;
      xml.header();
      xml.stag("museScore version=\"" MSC_VERSION "\"");
      xml.stag("Workspace");
      // xml.tag("name", _name);
      PaletteBox* pb = mscore->getPaletteBox();
      pb->write(xml);
      xml.etag();
      xml.etag();
      f.addFile("workspace.xml", cbuf.data());
      cbuf.close();
      }

      if (f.status() != MQZipWriter::NoError)
            writeFailed(_path);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Workspace::read()
      {
      if (_path.isEmpty() || !QFile(_path).exists()) {
            PaletteBox* paletteBox = mscore->getPaletteBox();
            paletteBox->clear();
            mscore->populatePalette();
            return;
            }
      QFileInfo fi(_path);
      _readOnly = !fi.isWritable();

      MQZipReader f(_path);
      QByteArray ba = f.fileData("META-INF/container.xml");

      XmlReader e(ba, _path);

      // extract first rootfile
      QString rootfile = "";
      QList<QString> images;
      while (e.readNextStartElement()) {
            if (e.name() != "container")
                  e.unknown();
            while (e.readNextStartElement()) {
                  if (e.name() != "rootfiles") {
                        e.unknown();
                        break;
                        }
                  while (e.readNextStartElement()) {
                        const QStringRef& tag(e.name());
                        if (tag == "rootfile") {
                              if (rootfile.isEmpty())
                                    rootfile = e.attribute("full-path");
                              e.readNext();
                              e.readNext();
                              }
                        else if (tag == "file")
                              images.append(e.readElementText());
                        else
                              e.unknown();
                        }
                  }
            }
      //
      // load images
      //
      foreach(const QString& s, images)
            imageStore.add(s, f.fileData(s));

      if (rootfile.isEmpty()) {
            qDebug("can't find rootfile in: %s", qPrintable(_path));
            return;
            }

      ba = f.fileData(rootfile);
      e.clear();
      e.addData(ba);

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
            else
                  e.unknown();
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
            QStringList path;
            path << mscoreGlobalShare + "workspaces";
            path << dataPath + "/workspaces";
            QStringList nameFilters;
            nameFilters << "*.workspace";

            foreach(QString s, path) {
                  QDir dir(s);
                  QStringList pl = dir.entryList(nameFilters, QDir::Files, QDir::Name);

                  foreach (QString entry, pl) {
                        Workspace* p = 0;
                        QFileInfo fi(s + "/" + entry);
                        QString name(fi.baseName());
                        foreach(Workspace* w, _workspaces) {
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
      foreach (Palette* p, pl)
            p->setSystemPalette(false);

      _workspaces.append(p);
      return p;
      }

//---------------------------------------------------------
//   writeBuiltinWorkspace
//---------------------------------------------------------

void Workspace::writeBuiltinWorkspace()
      {
      PaletteBox* paletteBox = mscore->getPaletteBox();
      paletteBox->clear();
      mscore->populatePalette();

      Workspace ws;
      ws.setName("advanced");
      ws.setPath("advanced.workspace");
      ws.write();
      }
}

