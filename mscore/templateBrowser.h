//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2014 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __TEMPLATEBROWSER_H__
#define __TEMPLATEBROWSER_H__

#include "ui_templateBrowser.h"
#include "scoreInfo.h"

class QTreeWidgetItem;

namespace Ms {

class TemplateItem;
class TemplateCategory;

//---------------------------------------------------------
//   TemplateBrowser
//---------------------------------------------------------

class TemplateBrowser : public QWidget, public Ui::TemplateBrowser
      {
      Q_OBJECT

      bool _stripNumbers  { false }; // remove number prefix from filenames
      bool _showPreview   { true  }; // show preview of templates
      bool _showCustomCategory  { false }; // show a custom category for user's own templates

      TemplateItem* genTemplateItem(QTreeWidgetItem*, const QFileInfo&);

   private slots:
      void scoreClicked();
      void handleItemActivated(QTreeWidgetItem* item);

   signals:
      void leave();
      void scoreSelected(QString);
      void scoreActivated(QString path);

   public:
      TemplateBrowser(QWidget* parent = 0);
      void setScores(QFileInfoList&);
      void setStripNumbers(bool val) { _stripNumbers = val; }
      void filter(const QString&);
      };
}

#endif
