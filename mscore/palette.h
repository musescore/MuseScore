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
class Xml;
class XmlReader;
class Palette;

//---------------------------------------------------------
//   PaletteCell
//---------------------------------------------------------

struct PaletteCell {
      Element* element = 0;
      QString name;           // used for tool tip
      QString tag;
      bool drawStaff = false;
      double x = 0.0, y = 0.0;
      double xoffset = 0.0, yoffset = 0.0;      // in spatium units of "gscore"
      qreal mag = 1.0;
      bool readOnly = true;
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
//    PaletteScrollArea
//---------------------------------------------------------

class PaletteScrollArea : public QScrollArea {
      Q_OBJECT
      bool _restrictHeight;

      virtual void resizeEvent(QResizeEvent*);

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

      int hgrid, vgrid;
      int currentIdx;
      int selectedIdx;
      QPoint dragStartPosition;
      int dragSrcIdx;

      qreal extraMag;
      bool _drawGrid;
      bool _selectable;
      bool _readOnly;
      bool _systemPalette;
      qreal _yOffset;         // in spatium units of "gscore"

      bool _moreElements;

      void redraw(const QRect&);
      virtual void paintEvent(QPaintEvent*);
      virtual void mousePressEvent(QMouseEvent*);
      virtual void mouseDoubleClickEvent(QMouseEvent*);
      virtual void mouseMoveEvent(QMouseEvent*);
      virtual void leaveEvent(QEvent*);
      virtual bool event(QEvent*);
      virtual void resizeEvent(QResizeEvent*);

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
      void moreButtonClicked();
      void displayMore(const QString& paletteName);

   public:
      Palette(QWidget* parent = 0);
      virtual ~Palette();

      PaletteCell* append(Element*, const QString& name, QString tag = QString(),
         qreal mag = 1.0);
      PaletteCell* add(int idx, Element*, const QString& name,
         const QString tag = QString(), qreal mag = 1.0);
      PaletteCell* append(SymId sym);

      void emitChanged()             { emit changed(); }
      void setGrid(int, int);
      Element* element(int idx);
      void setDrawGrid(bool val)     { _drawGrid = val; }
      bool drawGrid() const          { return _drawGrid; }
      bool read(const QString& path);
      void write(const QString& path);
      void read(XmlReader&);
      void write(Xml&) const;
      bool read(QFile*);
      void clear();
      void setSelectable(bool val)   { _selectable = val;  }
      bool selectable() const        { return _selectable; }
      int getSelectedIdx() const     { return selectedIdx; }
      void setSelected(int idx)      { selectedIdx = idx;  }
      bool readOnly() const          { return _readOnly;   }
      void setReadOnly(bool val);

      bool systemPalette() const     { return _systemPalette; }
      void setSystemPalette(bool val);

      void setMag(qreal val)         { extraMag = val;     }
      qreal mag() const              { return extraMag;    }
      void setYOffset(qreal val)     { _yOffset = val;     }
      qreal yOffset() const          { return _yOffset;        }
      int columns() const            { return width() / hgrid; }
      int rows() const;
      int size() const               { return cells.size(); }
      void setCellReadOnly(int c, bool v) { cells[c]->readOnly = v; }
      QString name() const           { return _name;        }
      void setName(const QString& s) { _name = s;           }
      int gridWidth() const          { return hgrid;        }
      int gridHeight() const         { return vgrid;        }
      bool moreElements() const      { return _moreElements; }
      void setMoreElements(bool val);

      virtual int heightForWidth(int) const;
      virtual QSize sizeHint() const;
      };


} // namespace Ms
#endif
