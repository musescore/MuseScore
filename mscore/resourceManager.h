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

public:
    explicit ResourceManager(QWidget *parent = 0);
    QByteArray txt;
    void displayLanguages();
    void displayPlugins();
    bool verifyFile(QString path, QString hash);
    bool verifyLanguageFile(QString filename, QString hash);

private:
    QMap <QPushButton *, QString> buttonMap; 	// QPushButton -> filename
    QMap <QPushButton *, QString> buttonHashMap;// QPushButton -> hash of the file
    QString baseAddr;

public slots:
    void download();
   };

}
#endif // RESOURCE_H
