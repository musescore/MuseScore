//=============================================================================
//  Zerberus
//  Zample player
//
//  Copyright (C) 2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "zerberusgui.h"

#include "mscore/preferences.h"

//---------------------------------------------------------
//   SfzListDialog
//---------------------------------------------------------

SfzListDialog::SfzListDialog()
   : QDialog(0)
      {
      list = new QListWidget;
      QVBoxLayout* layout = new QVBoxLayout;
      layout->addWidget(list);
      setLayout(layout);
      connect(list, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(itemSelected(QListWidgetItem*)));
      }

void SfzListDialog::add(const QString& name, const QString& path)
      {
      QListWidgetItem* item = new QListWidgetItem;
      item->setText(name);
      item->setData(Qt::UserRole, path);
      list->addItem(item);
      }

//---------------------------------------------------------
//   itemSelected
//---------------------------------------------------------

void SfzListDialog::itemSelected(QListWidgetItem* item)
      {
      _idx = list->row(item);
      accept();
      }

QString SfzListDialog::name()
      {
      if (_idx == -1)
            return QString();
      return list->item(_idx)->text();
      }

QString SfzListDialog::path()
      {
      if (_idx == -1)
            return QString();
      return list->item(_idx)->data(Qt::UserRole).toString();
      }

//---------------------------------------------------------
//   Zerberusgui
//---------------------------------------------------------

ZerberusGui::ZerberusGui()
   : SynthesizerGui()
      {
      setupUi(this);
      }

//---------------------------------------------------------
//   addClicked
//---------------------------------------------------------

void ZerberusGui::addClicked()
      {
      QString path = preferences.sfPath;
      QStringList pl = path.split(":");
      QStringList nameFilters { "*.sfz", "*.SFZ"  };

      SfzListDialog ld;
      foreach(const QString& s, pl) {
            QString ss(s);
            if (!s.isEmpty() && s[0] == '~')
                  ss = QDir::homePath() + s.mid(1);
            QDir dir(ss);
            foreach(const QString& s, dir.entryList(nameFilters, QDir::Files)) {
                  ld.add(s, dir.absolutePath() + "/" + s);
                  }
            }
      ld.exec();
      QString sfName = ld.name();
      QString sfPath = ld.path();

      int n = files->count();
      QStringList sl;
      for (int i = 0; i < n; ++i) {
            QListWidgetItem* item = files->item(i);
            sl.append(item->text());
            }

      if (sl.contains(sfPath)) {
            QMessageBox::warning(this,
            tr("MuseScore"),
            QString(tr("Soundfont %1 already loaded")).arg(sfPath));
            }
      else {
            bool loaded = zerberus()->addSoundFont(sfPath);
            if (!loaded) {
                  QMessageBox::warning(this,
                  tr("MuseScore"),
                  QString(tr("cannot load soundfont %1")).arg(sfPath));
                  }
            else {
                  files->insertItem(0, sfName);
                  }
            }
      }

//---------------------------------------------------------
//   removeClicked
//---------------------------------------------------------

void ZerberusGui::removeClicked()
      {
      int row = files->currentRow();
      if (row >= 0) {
            QString s(files->item(row)->text());
            zerberus()->removeSoundFont(s);
            delete files->takeItem(row);
            }
      }

