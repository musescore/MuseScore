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

#ifndef __PALETTE_BOX__
#define __PALETTE_BOX__

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

   private slots:
      void paletteCmd(int, int);
      void closeAll();
      void displayMore(const QString& paletteName);

   signals:
      void changed();

   public:
      PaletteBox(QWidget* parent = 0);
      void addPalette(Palette*);
      void write(Xml&);
      bool read(XmlReader&);
      void clear();
      QList<Palette*> palettes() const;
      };



} // namespace Ms
#endif

