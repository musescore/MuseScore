//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
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

static bool profilesRead = false;
static QList<Workspace*> _profiles;
Workspace* profile;

//---------------------------------------------------------
//   showWorkspaceMenu
//---------------------------------------------------------

void MuseScore::showWorkspaceMenu()
      {
      if (profiles == 0) {
            profiles = new QActionGroup(this);
            profiles->setExclusive(true);
            connect(profiles, SIGNAL(triggered(QAction*)), SLOT(changeWorkspace(QAction*)));
            }
      else {
            foreach(QAction* a, profiles->actions())
                  profiles->removeAction(a);
            }
      menuWorkspaces->clear();

      const QList<Workspace*> pl = Workspace::profiles();
//      QAction* active = 0;
      foreach (Workspace* p, pl) {
            QAction* a = profiles->addAction(p->name());
            a->setCheckable(true);
            a->setData(p->path());
            if (a->text() == preferences.profile) {
//                  active = a;
                  a->setChecked(true);
                  }
            menuWorkspaces->addAction(a);
            }
      menuWorkspaces->addSeparator();
      QAction* a = new QAction(tr("New Workspace"), this);
      connect(a, SIGNAL(triggered()), SLOT(createNewWorkspace()));
      menuWorkspaces->addAction(a);
      deleteWorkspaceAction = new QAction(tr("Delete Workspace"), this);
      connect(deleteWorkspaceAction, SIGNAL(triggered()), SLOT(deleteWorkspace()));
      menuWorkspaces->addAction(deleteWorkspaceAction);
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
            foreach(Workspace* p, Workspace::profiles()) {
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
      profile->save();
      profile = Workspace::createNewWorkspace(s);
      preferences.profile = profile->name();
      }

//---------------------------------------------------------
//   deleteWorkspace
//---------------------------------------------------------

void MuseScore::deleteWorkspace()
      {
      if (!profiles)
            return;
      QAction* a = profiles->checkedAction();
      if (!a)
            return;
      preferences.dirty = true;
      Workspace* profile = 0;
      foreach(Workspace* p, Workspace::profiles()) {
            if (p->name() == a->text()) {
                  profile = p;
                  break;
                  }
            }
      if (!profile)
            return;
      Workspace::profiles().removeOne(profile);
      QFile f(profile->path());
      f.remove();
//TODO:??      delete profile;
      profile             = Workspace::profiles().first();
      preferences.profile = profile->name();
      }

//---------------------------------------------------------
//   changeWorkspace
//---------------------------------------------------------

void MuseScore::changeWorkspace(QAction* a)
      {
      preferences.profile = a->text();
      preferences.dirty = true;
      foreach(Workspace* p, Workspace::profiles()) {
            if (p->name() == a->text()) {
                  changeWorkspace(p);
                  return;
                  }
            }
      qDebug("   profile not found");
      }

//---------------------------------------------------------
//   changeWorkspace
//---------------------------------------------------------

void MuseScore::changeWorkspace(Workspace* p)
      {
      profile->save();
      p->read();
      profile = p;
      }

//---------------------------------------------------------
//   initWorkspace
//---------------------------------------------------------

void initWorkspace()
      {
      foreach(Workspace* p, Workspace::profiles()) {
            if (p->name() == preferences.profile) {
                  profile = p;
                  break;
                  }
            }
      if (profile == 0) {
            profile = new Workspace;
            profile->setName("default");
            }
      }

//---------------------------------------------------------
//   writeFailed
//---------------------------------------------------------

static void writeFailed(const QString& _path)
      {
      QString s = mscore->tr("Open Workspace File\n") + _path + mscore->tr("\nfailed: ");
      QMessageBox::critical(mscore, mscore->tr("MuseScore: Writing Workspace file"), s);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Workspace::write()
      {
      QString ext(".profile");
      if (_path.isEmpty()) {
            QDir dir;
            dir.mkpath(dataPath);
            _path = dataPath + "/profiles";
            dir.mkpath(_path);
            _path += "/" + _name + ext;
            }
      QZipWriter f(_path);
      f.setCompressionPolicy(QZipWriter::NeverCompress);
      f.setCreationPermissions(
         QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner
         | QFile::ReadUser | QFile::WriteUser | QFile::ExeUser
         | QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup
         | QFile::ReadOther | QFile::WriteOther | QFile::ExeOther);

      if (f.status() != QZipWriter::NoError) {
            writeFailed(_path);
            return;
            }

      QBuffer cbuf;
      cbuf.open(QIODevice::ReadWrite);
      Xml xml(&cbuf);
      xml.header();
      xml.stag("container");
      xml.stag("rootfiles");
      xml.stag(QString("rootfile full-path=\"%1\"").arg(Xml::xmlString("profile.xml")));
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
      f.addDirectory("META-INF");
      f.addDirectory("Pictures");
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
      xml.tag("name", _name);
      PaletteBox* pb = mscore->getPaletteBox();
      pb->write(xml);
      xml.etag();
      xml.etag();
      f.addFile("profile.xml", cbuf.data());
      cbuf.close();
      }

      f.close();
      if (f.status() != QZipWriter::NoError)
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
      QZipReader f(_path);
      QByteArray ba = f.fileData("META-INF/container.xml");

      XmlReader e(ba);

      // extract first rootfile
      QString rootfile = "";
      QList<QString> images;
      while (e.readNextStartElement()) {
            if (e.name() != "container") {
                  e.unknown();
                  break;
                  }
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
//                  QString version = e.attribute("version");
//                  QStringList sl = version.split('.');
                  // _mscVersion = sl[0].toInt() * 100 + sl[1].toInt();
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
                  _name = e.readElementText();
            else if (tag == "PaletteBox") {
                  PaletteBox* paletteBox = mscore->getPaletteBox();
                  paletteBox->clear();
                  paletteBox->read(e);
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
      PaletteBox* pb = mscore->getPaletteBox();
      if (pb && pb->dirty())
            write();
      }

//---------------------------------------------------------
//   profiles
//---------------------------------------------------------

QList<Workspace*>& Workspace::profiles()
      {
      if (!profilesRead) {
            QString s = dataPath + "/profiles";
            QDir dir(s);
            QStringList nameFilters;
            nameFilters << "*.profile";
            QStringList pl = dir.entryList(nameFilters, QDir::Files, QDir::Name);

            foreach (QString s, pl) {
                  Workspace* p = new Workspace;
                  p->setPath(dataPath + "/profiles/" + s);
                  p->setName(QFileInfo(s).baseName());
                  _profiles.append(p);
                  }
            profilesRead = true;
            }
      return _profiles;
      }

//---------------------------------------------------------
//   createNewWorkspace
//---------------------------------------------------------

Workspace* Workspace::createNewWorkspace(const QString& name)
      {
      Workspace* p = new Workspace(*profile);
      p->setName(name);
      p->setPath("");
      p->setDirty(false);
      p->write();
      _profiles.append(p);
      return p;
      }

