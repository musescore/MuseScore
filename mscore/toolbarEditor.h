//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2016 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __TOOLBAREDITOR_H__
#define __TOOLBAREDITOR_H__

#include "ui_toolbarEditor.h"
#include "abstractdialog.h"

namespace Ms {

//---------------------------------------------------------
//   ToolbarEditor
//---------------------------------------------------------

class ToolbarEditor : public AbstractDialog, public Ui::ToolbarEditor {
      Q_OBJECT

      std::vector<std::list<const char*>*> *new_toolbars;
      void updateNewToolbar(int toolbar_to_update);

      void populateLists(const std::list<const char*>&, std::list<const char*>*);
      bool isSeparator(QListWidgetItem*) const;

      virtual void hideEvent(QHideEvent*);
      
   private slots:
      void toolbarChanged(int);
      void addAction();
      void removeAction();
      void upAction();
      void downAction();
      void accepted();
      
   protected:
      virtual void retranslate();

   public:
      ToolbarEditor(QWidget* parent = 0);
      void init();
      };

} // namespace Ms

#endif

