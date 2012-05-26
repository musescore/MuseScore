//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: palette.h 5395 2012-02-28 18:09:57Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer and others
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

#ifndef __PALETTE_H__
#define __PALETTE_H__

class Element;
class Sym;
class Xml;
class Palette;
class PaletteScrollArea;

#include "ui_palette.h"
#include "ui_cellproperties.h"

//---------------------------------------------------------
//   PaletteCell
//---------------------------------------------------------

struct PaletteCell {
      Element* element;
      QString name;           // used for tool tip
      QString tag;
      bool drawStaff;
      double x, y;
      double xoffset, yoffset;      // in spatium units of "gscore"
      qreal mag;
      bool readOnly;
      };

//---------------------------------------------------------
//   PaletteProperties
//---------------------------------------------------------

class PaletteProperties : public QDialog, private Ui::PaletteProperties {
      Q_OBJECT

      Palette* palette;
      virtual void accept();

   public:
      PaletteProperties(Palette* p, QWidget* parent = 0);
      };

//---------------------------------------------------------
//   PaletteCellProperties
//---------------------------------------------------------

class PaletteCellProperties : public QDialog, private Ui::PaletteCellProperties {
      Q_OBJECT

      PaletteCell* cell;
      virtual void accept();

   public:
      PaletteCellProperties(PaletteCell* p, QWidget* parent = 0);
      };

//---------------------------------------------------------
//   PaletteBoxButton
//---------------------------------------------------------

enum PaletteCommand {
      PALETTE_DELETE,
      PALETTE_SAVE,
      PALETTE_LOAD,
      PALETTE_EDIT,
      PALETTE_UP,
      PALETTE_DOWN,
      PALETTE_NEW
      };

class PaletteBoxButton : public QToolButton {
      Q_OBJECT

      friend class PaletteBox;

      Palette* palette;
      PaletteScrollArea* scrollArea;
      QAction* editAction;

      int id;

      virtual void changeEvent(QEvent*);
      virtual void paintEvent( QPaintEvent * );

   private slots:
      void deleteTriggered()     { emit paletteCmd(PALETTE_DELETE, id);  }
      void saveTriggered()       { emit paletteCmd(PALETTE_SAVE, id);    }
      void loadTriggered()       { emit paletteCmd(PALETTE_LOAD, id);    }
      void propertiesTriggered() { emit paletteCmd(PALETTE_EDIT, id);    }
      void upTriggered()         { emit paletteCmd(PALETTE_UP, id);      }
      void downTriggered()       { emit paletteCmd(PALETTE_DOWN, id);    }
      void newTriggered()        { emit paletteCmd(PALETTE_NEW, id);     }
      void beforePulldown();
      void enableEditing(bool);
      void showPalette(bool);

   signals:
      void paletteCmd(int, int);
      void closeAll();

   public:
      PaletteBoxButton(PaletteScrollArea*, Palette*, QWidget* parent = 0);
      void setId(int v) { id = v; }
      };

//---------------------------------------------------------
//   PaletteBox
//---------------------------------------------------------

class PaletteBox : public QDockWidget {
      Q_OBJECT

      QVBoxLayout* vbox;
      bool _dirty;

      virtual void closeEvent(QCloseEvent*);
      Palette* newPalette(const QString& name, int slot);

   private slots:
      void paletteCmd(int, int);
      void setDirty() { _dirty = true; }
      void contextMenu(const QPoint&);
      void closeAll();

   signals:
      void paletteVisible(bool);

   public:
      PaletteBox(QWidget* parent = 0);
      void addPalette(Palette*);
      bool dirty() const      { return _dirty; }
      void write(Xml&);
      bool read(QDomElement);
      void clear();
      };

//---------------------------------------------------------
//   PaletteScrollArea
//---------------------------------------------------------

class PaletteScrollArea : public QScrollArea {
      Q_OBJECT
      bool _restrictHeight;

      virtual void resizeEvent(QResizeEvent*);

   public:
      PaletteScrollArea(Palette* w, QWidget* parent = 0);
      bool restrictHeight() const      { return _restrictHeight; }
      void setRestrictHeight(bool val) { _restrictHeight = val;  }
      };

//---------------------------------------------------------
//   Palette
//---------------------------------------------------------

class Palette : public QWidget {
      Q_OBJECT

      QString _name;
      QString _tag;
      QList<PaletteCell*> cells;

      int hgrid, vgrid;
      int currentIdx;
      int selectedIdx;
      QPoint dragStartPosition;
      int dragSrcIdx;

      qreal extraMag;
      bool _drawGrid;
      bool _selectable;
      bool _readOnly;
      qreal _yOffset;         // in spatium units of "gscore"

      void redraw(const QRect&);
      virtual void paintEvent(QPaintEvent*);
      virtual void mousePressEvent(QMouseEvent*);
      virtual void mouseDoubleClickEvent(QMouseEvent*);
      virtual void mouseMoveEvent(QMouseEvent*);
      virtual void leaveEvent(QEvent*);
      virtual bool event(QEvent*);

      virtual void dragEnterEvent(QDragEnterEvent*);
      virtual void dragMoveEvent(QDragMoveEvent*);
      virtual void dropEvent(QDropEvent*);
      virtual void contextMenuEvent(QContextMenuEvent*);

      int idx(const QPoint&) const;
      QRect idxRect(int);
      void layoutCell(PaletteCell*);

   private slots:
      void actionToggled(bool val);

   signals:
      void startDragElement(Element*);
      void boxClicked(int);
      void changed();

   public:
      Palette(QWidget* parent = 0);
      ~Palette();

      PaletteCell* append(Element*, const QString& name, QString tag = QString(),
         qreal mag = 1.0);
      PaletteCell* add(int idx, Element*, const QString& name,
         const QString tag = QString(), qreal mag = 1.0);
      PaletteCell* append(int sym);

      void setGrid(int, int);
      Element* element(int idx)      { return cells[idx]->element; }
      void setDrawGrid(bool val)     { _drawGrid = val; }
      bool drawGrid() const          { return _drawGrid; }
      void read(const QString& path);
      void write(const QString& path);
      void read(QDomElement);
      void write(Xml&, const QString& name) const;
      bool read(QFile*);
      void clear();
      void setSelectable(bool val)   { _selectable = val;  }
      bool selectable() const        { return _selectable; }
      int getSelectedIdx() const     { return selectedIdx; }
      void setSelected(int idx)      { selectedIdx = idx;  }
      bool readOnly() const          { return _readOnly;   }
      void setReadOnly(bool val);
      void setMag(qreal val)         { extraMag = val;     }
      qreal mag() const              { return extraMag;    }
      void setYOffset(qreal val)     { _yOffset = val;     }
      qreal yOffset() const          { return _yOffset;        }
      int columns() const            { return width() / hgrid; }
      int rows() const;
      int size() const               { return cells.size(); }
      void setCellReadOnly(int c, bool v) { cells[c]->readOnly = v; }
      int heightForWidth(int) const;
      QString name() const           { return _name;        }
      void setName(const QString& s) { _name = s;           }
      int gridWidth() const          { return hgrid;        }
      int gridHeight() const         { return vgrid;        }
      virtual QSize sizeHint() const;
      };

#endif
