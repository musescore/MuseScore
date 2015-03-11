//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: palette.cpp 5576 2012-04-24 19:15:22Z wschweer $
//
//  Copyright (C) 2011 Werner Schweer and others
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

class Xml;
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
      const int paletteStretch = 1000;

   private slots:
      void paletteCmd(PaletteCommand, int);
      void closeAll();
      void displayMore(const QString& paletteName);
      void workspaceSelected(int idx);
      void newWorkspaceClicked();

   signals:
      void changed();

   public:
      PaletteBox(QWidget* parent = 0);
      void addPalette(Palette*);
      void write(Xml&);
      bool read(XmlReader&);
      void clear();
      QList<Palette*> palettes() const;
      void updateWorkspaces();
      };

class PaletteBoxScrollArea : public QScrollArea {
       Q_OBJECT

    public:
      virtual QSize sizeHint() const;
      };

} // namespace Ms
#endif

