//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: palettebox.h 5576 2012-04-24 19:15:22Z wschweer $
//
//  Copyright (C) 2011-2016 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#ifndef __PALETTE_BOX_H__
#define __PALETTE_BOX_H__

#include "paletteBoxButton.h"

namespace Ms {

class XmlWriter;
class XmlReader;
class Palette;

//---------------------------------------------------------
//   PaletteBox
//---------------------------------------------------------

class PaletteBox : public QDockWidget {
      Q_OBJECT

      QVBoxLayout* vbox;
      Palette* newPalette(const QString& name, int slot);
      QComboBox* workspaceList;
      QLineEdit* _searchBox;
      const int paletteStretch = 1000;
      QAction* singlePaletteAction;
      QToolButton* addWorkspaceButton;
      bool keyboardNavigation = false;

   private slots:
      void paletteCmd(PaletteCommand, int);
      void closeAll();
      void workspaceSelected(int idx);
      void newWorkspaceClicked();
      void setSinglePalette(bool);
      void filterPalettes(const QString& text);

   signals:
      void changed();

   private:
      void changeEvent(QEvent *event);
      void retranslate();

   public:
      PaletteBox(QWidget* parent = 0);
      void addPalette(Palette*);
      void write(XmlWriter&);
      bool read(XmlReader&);
      void clear();
      QList<Palette*> palettes() const;
      void updateWorkspaces();
      QLineEdit* searchBox() { return _searchBox; }
      bool noSelection();
      void mousePressEvent(QMouseEvent* ev, Palette* p1);
      void navigation(QKeyEvent *event);
      bool eventFilter(QObject* obj, QEvent *event);
      void setKeyboardNavigation(bool val) { keyboardNavigation = val; }
      bool getKeyboardNavigation() { return keyboardNavigation; }
      void selectWorkspace(QString path);
      };

//---------------------------------------------------------
//   PaletteBoxScrollArea
//---------------------------------------------------------

class PaletteBoxScrollArea : public QScrollArea {
       Q_OBJECT

    public:
      virtual QSize sizeHint() const;
      };

} // namespace Ms
#endif

