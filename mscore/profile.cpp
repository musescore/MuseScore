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

#include "profile.h"
#include "musescore.h"
#include "libmscore/score.h"
#include "libmscore/imageStore.h"
#include "libmscore/xml.h"
#include "libmscore/qzipreader_p.h"
#include "libmscore/qzipwriter_p.h"
#include "preferences.h"
#include "palette.h"

static bool profilesRead = false;
static QList<Profile*> _profiles;
Profile* profile;

//---------------------------------------------------------
//   showProfileMenu
//---------------------------------------------------------

void MuseScore::showProfileMenu()
      {
      if (profiles == 0) {
            profiles = new QActionGroup(this);
            profiles->setExclusive(true);
            connect(profiles, SIGNAL(triggered(QAction*)), SLOT(changeProfile(QAction*)));
            }
      else {
            foreach(QAction* a, profiles->actions())
                  profiles->removeAction(a);
            }
      menuProfiles->clear();

      const QList<Profile*> pl = Profile::profiles();
//      QAction* active = 0;
      foreach (Profile* p, pl) {
            QAction* a = profiles->addAction(p->name());
            a->setCheckable(true);
            a->setData(p->path());
            if (a->text() == preferences.profile) {
//                  active = a;
                  a->setChecked(true);
                  }
            menuProfiles->addAction(a);
            }
      menuProfiles->addSeparator();
      QAction* a = new QAction(tr("New Profile"), this);
      connect(a, SIGNAL(triggered()), SLOT(createNewProfile()));
      menuProfiles->addAction(a);
      deleteProfileAction = new QAction(tr("Delete Profile"), this);
      connect(deleteProfileAction, SIGNAL(triggered()), SLOT(deleteProfile()));
      menuProfiles->addAction(deleteProfileAction);
      }

//---------------------------------------------------------
//   createNewProfile
//---------------------------------------------------------

void MuseScore::createNewProfile()
      {
      QString s = QInputDialog::getText(this, tr("MuseScore: Read Profile Name"),
         tr("Profile Name:"));
      if (s.isEmpty())
            return;
      for (;;) {
            bool notFound = true;
            foreach(Profile* p, Profile::profiles()) {
                  if (p->name() == s) {
                        notFound = false;
                        break;
                        }
                  }
            if (!notFound) {
                  s = QInputDialog::getText(this,
                     tr("MuseScore: Read Profile Name"),
                     QString(tr("'%1' does already exist,\nplease choose a different name:")).arg(s)
                     );
                  if (s.isEmpty())
                        return;
                  }
            else
                  break;
            }
      profile->save();
      profile = Profile::createNewProfile(s);
      preferences.profile = profile->name();
      }

//---------------------------------------------------------
//   deleteProfile
//---------------------------------------------------------

void MuseScore::deleteProfile()
      {
      if (!profiles)
            return;
      QAction* a = profiles->checkedAction();
      if (!a)
            return;
      preferences.dirty = true;
      Profile* profile = 0;
      foreach(Profile* p, Profile::profiles()) {
            if (p->name() == a->text()) {
                  profile = p;
                  break;
                  }
            }
      if (!profile)
            return;
      Profile::profiles().removeOne(profile);
      QFile f(profile->path());
      f.remove();
//TODO:??      delete profile;
      profile             = Profile::profiles().first();
      preferences.profile = profile->name();
      }

//---------------------------------------------------------
//   changeProfile
//---------------------------------------------------------

void MuseScore::changeProfile(QAction* a)
      {
      preferences.profile = a->text();
      preferences.dirty = true;
      foreach(Profile* p, Profile::profiles()) {
            if (p->name() == a->text()) {
                  changeProfile(p);
                  return;
                  }
            }
      qDebug("   profile not found");
      }

//---------------------------------------------------------
//   changeProfile
//---------------------------------------------------------

void MuseScore::changeProfile(Profile* p)
      {
      profile->save();
      p->read();
      profile = p;
      }

//---------------------------------------------------------
//   initProfile
//---------------------------------------------------------

void initProfile()
      {
      foreach(Profile* p, Profile::profiles()) {
            if (p->name() == preferences.profile) {
                  profile = p;
                  break;
                  }
            }
      if (profile == 0) {
            profile = new Profile;
            profile->setName("default");
            }
      }

//---------------------------------------------------------
//   writeFailed
//---------------------------------------------------------

static void writeFailed(const QString& _path)
      {
      QString s = mscore->tr("Open Profile File\n") + _path + mscore->tr("\nfailed: ");
      QMessageBox::critical(mscore, mscore->tr("MuseScore: Writing Profile file"), s);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Profile::write()
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
      xml.stag("Profile");
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

void Profile::read()
      {
      QZipReader f(_path);
      if (!f.exists()) {
            PaletteBox* paletteBox = mscore->getPaletteBox();
            paletteBox->clear();
            mscore->populatePalette();
            return;
            }
      QByteArray ba = f.fileData("META-INF/container.xml");
      QDomDocument doc;
      int line, column;
      QString err;
      if (!doc.setContent(ba, false, &err, &line, &column)) {
            QString error = QString("error reading profile container.xml at line %2 column %3: %4")
               .arg(line).arg(column).arg(err);
            QMessageBox::warning(0,
                  QWidget::tr("MuseScore: Load Style failed:"),
                  error,
                  QString::null, QWidget::tr("Quit"), QString::null, 0, 1);
            return;
            }
      // extract first rootfile
      QString rootfile = "";
      QList<QString> images;
      for (QDomElement e = doc.documentElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() != "container") {
                  domError(e);
                  continue;
                  }
            for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                  if (ee.tagName() != "rootfiles") {
                        domError(ee);
                        continue;
                        }
                  for (QDomElement eee = ee.firstChildElement(); !eee.isNull(); eee = eee.nextSiblingElement()) {
                        const QString& tag(eee.tagName());
                        const QString& val(eee.text());

                        if (tag == "rootfile") {
                              if (rootfile.isEmpty())
                                    rootfile = eee.attribute(QString("full-path"));
                              }
                        else if (tag == "file")
                              images.append(val);
                        else
                              domError(eee);
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
      if (!doc.setContent(ba, false, &err, &line, &column)) {
            QString error = QString("error reading profile %1 at line %2 column %3: %4")
               .arg(_path).arg(line).arg(column).arg(err);
            QMessageBox::warning(0,
                  QWidget::tr("MuseScore: Load Style failed:"),
                  error,
                  QString::null, QWidget::tr("Quit"), QString::null, 0, 1);
            return;
            }

      docName = _path;
      for (QDomElement e = doc.documentElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "museScore") {
                  QString version = e.attribute(QString("version"));
                  QStringList sl = version.split('.');
                  // _mscVersion = sl[0].toInt() * 100 + sl[1].toInt();
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull();  ee = ee.nextSiblingElement()) {
                        QString tag(ee.tagName());
                        QString val(ee.text());
                        if (tag == "Profile")
                              read(ee);
                        else
                              domError(ee);
                        }
                  }
            }
      }

void Profile::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());

            if (tag == "name")
                  _name = val;
            else if (tag == "PaletteBox") {
                  PaletteBox* paletteBox = mscore->getPaletteBox();
                  paletteBox->clear();
                  paletteBox->read(e);
                  }
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   save
//---------------------------------------------------------

void Profile::save()
      {
      PaletteBox* pb = mscore->getPaletteBox();
      if (pb && pb->dirty())
            write();
      }

//---------------------------------------------------------
//   profiles
//---------------------------------------------------------

QList<Profile*>& Profile::profiles()
      {
      if (!profilesRead) {
            QString s = dataPath + "/profiles";
            QDir dir(s);
            QStringList nameFilters;
            nameFilters << "*.profile";
            QStringList pl = dir.entryList(nameFilters, QDir::Files, QDir::Name);

            foreach (QString s, pl) {
                  Profile* p = new Profile;
                  p->setPath(dataPath + "/profiles/" + s);
                  p->setName(QFileInfo(s).baseName());
                  _profiles.append(p);
                  }
            profilesRead = true;
            }
      return _profiles;
      }

//---------------------------------------------------------
//   createNewProfile
//---------------------------------------------------------

Profile* Profile::createNewProfile(const QString& name)
      {
      Profile* p = new Profile(*profile);
      p->setName(name);
      p->setPath("");
      p->setDirty(false);
      p->write();
      _profiles.append(p);
      return p;
      }

