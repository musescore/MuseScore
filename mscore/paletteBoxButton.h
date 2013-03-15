//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __PALETTE_BOX_BUTTON_H__
#define __PALETTE_BOX_BUTTON_H__

class Palette;

enum PaletteCommand {
      PALETTE_DELETE,
      PALETTE_SAVE,
      PALETTE_LOAD,
      PALETTE_EDIT,
      PALETTE_UP,
      PALETTE_DOWN,
      PALETTE_NEW
      };

//---------------------------------------------------------
//   PaletteBoxButton
//---------------------------------------------------------

class PaletteBoxButton : public QToolButton {
      Q_OBJECT

      Palette* palette;
      QAction* editAction;

      int id;

      virtual void changeEvent(QEvent*);
      virtual void paintEvent(QPaintEvent*);
      virtual void contextMenuEvent(QContextMenuEvent*);
      virtual QSize sizeHint() const;

   private slots:
      void deleteTriggered()     { emit paletteCmd(PALETTE_DELETE, id);  }
      void saveTriggered()       { emit paletteCmd(PALETTE_SAVE, id);    }
      void loadTriggered()       { emit paletteCmd(PALETTE_LOAD, id);    }
      void propertiesTriggered() { emit paletteCmd(PALETTE_EDIT, id);    }
      void upTriggered()         { emit paletteCmd(PALETTE_UP, id);      }
      void downTriggered()       { emit paletteCmd(PALETTE_DOWN, id);    }
      void newTriggered()        { emit paletteCmd(PALETTE_NEW, id);     }
      void enableEditing(bool);

   public slots:
      void showPalette(bool);

   signals:
      void paletteCmd(int, int);
      void closeAll();

   public:
      PaletteBoxButton(Palette*, QWidget* parent = 0);
      void setId(int v) { id = v; }
      };

#endif

