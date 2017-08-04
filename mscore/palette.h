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

#include "ui_palette.h"
#include "ui_cellproperties.h"
#include "libmscore/sym.h"

namespace Ms {

class Element;
class Sym;
class XmlWriter;
class XmlReader;
class Palette;

//---------------------------------------------------------
//   PaletteCell
//---------------------------------------------------------

struct PaletteCell {
      ~PaletteCell();

      Element* element { 0 };
      QString name;           // used for tool tip
      QString tag;

      bool drawStaff { false };
      double x       { 0.0   };
      double y       { 0.0   };
      double xoffset { 0.0   };
      double yoffset { 0.0   };      // in spatium units of "gscore"
      qreal mag      { 1.0   };
      bool readOnly  { false };
      };

//---------------------------------------------------------
//   PaletteProperties
//---------------------------------------------------------

class PaletteProperties : public QDialog, private Ui::PaletteProperties {
      Q_OBJECT

      Palette* palette;
      virtual void accept();
      virtual void hideEvent(QHideEvent*);
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
      virtual void hideEvent(QHideEvent*);
   public:
      PaletteCellProperties(PaletteCell* p, QWidget* parent = 0);
      };

//---------------------------------------------------------
//    PaletteScrollArea
//---------------------------------------------------------

class PaletteScrollArea : public QScrollArea {
      Q_OBJECT
      bool _restrictHeight;

      virtual void resizeEvent(QResizeEvent*);

   protected:
      virtual void keyPressEvent(QKeyEvent* event) override;

   public:
      PaletteScrollArea(Palette* w, QWidget* parent = 0);
      bool restrictHeight() const { return _restrictHeight; }
      void setRestrictHeight(bool val) { _restrictHeight = val; }
      };

//---------------------------------------------------------
//   Palette
//---------------------------------------------------------

class Palette : public QWidget {
      Q_OBJECT

      QString _name;
      QList<PaletteCell*> cells;
      QList<PaletteCell*> dragCells;  // used for filter & backup

      int hgrid;
      int vgrid;
      int currentIdx;
      int dragIdx;
      int selectedIdx;
      QPoint dragStartPosition;

      qreal extraMag;
      bool _drawGrid;
      bool _selectable;
      bool _disableDoubleClick { false };
      bool _readOnly;
      bool _systemPalette;
      qreal _yOffset;                // in spatium units of "gscore"
      bool filterActive { false };   // bool if filter is active

      bool _moreElements;
      bool _showContextMenu { true };

      virtual void paintEvent(QPaintEvent*) override;
      virtual void mousePressEvent(QMouseEvent*) override;
      virtual void mouseDoubleClickEvent(QMouseEvent*) override;
      virtual void mouseMoveEvent(QMouseEvent*) override;
      virtual void leaveEvent(QEvent*) override;
      virtual bool event(QEvent*) override;
      virtual void resizeEvent(QResizeEvent*) override;
      void applyPaletteElement(PaletteCell* cell);

      virtual void dragEnterEvent(QDragEnterEvent*) override;
      virtual void dragMoveEvent(QDragMoveEvent*) override;
      virtual void dropEvent(QDropEvent*) override;
      virtual void contextMenuEvent(QContextMenuEvent*) override;

      int idx2(const QPoint&) const;
      QRect idxRect(int) const;

      const QList<PaletteCell*>* ccp() const { return filterActive ? &dragCells : &cells; }
      QPixmap pixmap(int cellIdx) const;


   private slots:
      void actionToggled(bool val);

   signals:
      void boxClicked(int);
      void changed();
      void displayMore(const QString& paletteName);

   public:
      Palette(QWidget* parent = 0);
      virtual ~Palette();

      void nextPaletteElement();
      void prevPaletteElement();
      void applyPaletteElement();
      PaletteCell* append(Element*, const QString& name, QString tag = QString(),
         qreal mag = 1.0);
      PaletteCell* add(int idx, Element*, const QString& name,
         const QString tag = QString(), qreal mag = 1.0);

      void emitChanged()             { emit changed(); }
      void setGrid(int, int);
      Element* element(int idx);
      void setDrawGrid(bool val)     { _drawGrid = val; }
      bool drawGrid() const          { return _drawGrid; }
      bool read(const QString& path);
      void write(const QString& path);
      void read(XmlReader&);
      void write(XmlWriter&) const;
      bool read(QFile*);
      void clear();
      void setSelectable(bool val)   { _selectable = val;  }
      bool selectable() const        { return _selectable; }
      int getSelectedIdx() const     { return selectedIdx; }
      void setSelected(int idx)      { selectedIdx = idx;  }
      bool readOnly() const          { return _readOnly;   }
      void setReadOnly(bool val);
      void setDisableDoubleClick(bool val) { _disableDoubleClick = val; }

      bool systemPalette() const     { return _systemPalette; }
      void setSystemPalette(bool val);

      void setMag(qreal val);
      qreal mag() const              { return extraMag;    }
      void setYOffset(qreal val)     { _yOffset = val;     }
      qreal yOffset() const          { return _yOffset;        }
      int columns() const            { return width() / hgrid; }
      int rows() const;
      int size() const               { return filterActive ? dragCells.size() : cells.size(); }
      PaletteCell* cellAt(int index) const { return ccp()->value(index); }
      void setCellReadOnly(int c, bool v)  { cells[c]->readOnly = v;   }
      QString name() const           { return _name;        }
      void setName(const QString& s) { _name = s;           }
      int gridWidth() const          { return hgrid;        }
      int gridHeight() const         { return vgrid;        }
      bool moreElements() const      { return _moreElements; }
      void setMoreElements(bool val);
      bool filter(const QString& text);
      void setShowContextMenu(bool val) { _showContextMenu = val; }

      int getCurrentIdx() { return currentIdx; }
      void setCurrentIdx(int i) { currentIdx = i; }
      bool isFilterActive() { return filterActive == true; }
      QList<PaletteCell*> getDragCells() { return dragCells; }
      virtual int heightForWidth(int) const;
      virtual QSize sizeHint() const;
      int idx(const QPoint&) const;
      };


} // namespace Ms
#endif
