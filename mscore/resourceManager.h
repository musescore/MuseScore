//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef RESOURCE_H
#define RESOURCE_H

#include "ui_resourceManager.h"
#include "downloadUtils.h"

namespace Ms {

class ResourceManager : public QDialog, public Ui::Resource
      {
      Q_OBJECT

      virtual void hideEvent(QHideEvent*);
      QByteArray txt;
      void displayLanguages();
      void displayExtensions();
      bool verifyFile(QString path, QString hash);
      bool verifyLanguageFile(QString filename, QString hash);
      
   public:
      explicit ResourceManager(QWidget *parent = 0);
      void selectLanguagesTab();
      void selectExtensionsTab();
      
      static inline QString baseAddr() { return "http://extensions.musescore.org/3.6/"; }

   private:
      QMap <QPushButton *, QString> languageButtonMap; 	// QPushButton -> filename
      QMap <QPushButton *, QString> languageButtonHashMap;// QPushButton -> hash of the file

   private slots:
      void downloadLanguage();
      void downloadExtension();
      void uninstallExtension();
      };

class ExtensionFileSize : public QTableWidgetItem
      {
      int _size;

   public:
      ExtensionFileSize(const int i);
      int getSize() const { return _size; }
      bool operator<(const QTableWidgetItem& nextItem) const;

      };

class LanguageFileSize : public QTableWidgetItem
      {
      double _size;

   public:
      LanguageFileSize(const double d);
      double getSize() const { return _size; }
      bool operator<(const QTableWidgetItem& nextItem) const;

      };

}
#endif // RESOURCE_H
