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

#include "palette.h"
#include "palettebox.h"
#include "musescore.h"
#include "preferences.h"
#include "libmscore/xml.h"
#include "paletteBoxButton.h"
#include "workspace.h"

namespace Ms {

//---------------------------------------------------------
//   PaletteBox
//---------------------------------------------------------

PaletteBox::PaletteBox(QWidget* parent)
   : QDockWidget(tr("Palettes"), parent)
      {
      setObjectName("palette-box");
      setAllowedAreas(Qt::DockWidgetAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea));

      PaletteBoxScrollArea* sa = new PaletteBoxScrollArea;
      sa->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
      sa->setContextMenuPolicy(Qt::CustomContextMenu);
      sa->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      sa->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
      sa->setWidgetResizable(true);
      sa->setFrameShape(QFrame::NoFrame);
      setWidget(sa);

      QWidget* paletteList = new QWidget;
      sa->setWidget(paletteList);
      vbox = new QVBoxLayout;
      paletteList->setLayout(vbox);
      vbox->setMargin(0);
      vbox->setSpacing(1);
      vbox->addStretch();
      paletteList->show();
      }

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void PaletteBox::clear()
      {
      int n = vbox->count() - 1;    // do not delete stretch item
      while (n--) {
            QLayoutItem* item = vbox->takeAt(0);
            if (item->widget())
                  item->widget()->hide();
            delete item;
            }
      vbox->invalidate();
      }

//---------------------------------------------------------
//   addPalette
//---------------------------------------------------------

void PaletteBox::addPalette(Palette* w)
      {
      PaletteBoxButton* b = new PaletteBoxButton(w);

      int slotIdx = vbox->count() - 1;    // insert before stretch
      b->setId(slotIdx);

      vbox->insertWidget(slotIdx, b);
      vbox->insertWidget(slotIdx+1, w, 1000);

      connect(b, SIGNAL(paletteCmd(int,int)), SLOT(paletteCmd(int,int)));
      connect(b, SIGNAL(closeAll()), SLOT(closeAll()));
      connect(w, SIGNAL(changed()), SIGNAL(changed()));
      }

//---------------------------------------------------------
//   newPalette
//---------------------------------------------------------

Palette* PaletteBox::newPalette(const QString& name, int slot)
      {
      Palette* p = new Palette;
      p->setReadOnly(false);
      p->setName(name);
      PaletteBoxButton* b = new PaletteBoxButton(p);
      vbox->insertWidget(slot, b);
      vbox->insertWidget(slot+1, p, 1000);
      connect(b, SIGNAL(paletteCmd(int,int)), SLOT(paletteCmd(int,int)));
      connect(p, SIGNAL(changed()), Workspace::currentWorkspace, SLOT(setDirty()));
      for (int i = 0; i < (vbox->count() - 1) / 2; ++i)
            static_cast<PaletteBoxButton*>(vbox->itemAt(i * 2)->widget())->setId(i*2);
      return p;
      }

//---------------------------------------------------------
//   paletteCmd
//---------------------------------------------------------

void PaletteBox::paletteCmd(int cmd, int slot)
      {
      QLayoutItem* item   = vbox->itemAt(slot);
      PaletteBoxButton* b = static_cast<PaletteBoxButton*>(item->widget());
      Palette* palette    = static_cast<Palette*>(vbox->itemAt(slot+1)->widget());

      switch(cmd) {
            case PALETTE_DELETE:
                  {
                  vbox->removeItem(item);
                  b->deleteLater();      // this is the button widget
                  delete item;
                  item = vbox->itemAt(slot);
                  vbox->removeItem(item);
                  delete item->widget();
                  delete item;
                  for (int i = 0; i < (vbox->count() - 1) / 2; ++i)
                        static_cast<PaletteBoxButton*>(vbox->itemAt(i * 2)->widget())->setId(i*2);
                  emit changed();
                  }
                  break;
            case PALETTE_SAVE:
                  {
                  QString path = mscore->getPaletteFilename(false);
                  if (!path.isEmpty())
                        palette->write(path);
                  }
                  break;

            case PALETTE_LOAD:
                  {
                  QString path = mscore->getPaletteFilename(true);
                  if (!path.isEmpty()) {
                        QFileInfo fi(path);
                        Palette* palette = newPalette(fi.baseName(), slot);
                        palette->read(path);
                        }
                  }
                  emit changed();
                  break;

            case PALETTE_NEW:
                  palette = newPalette(tr("new Palette"), slot);
                  item   = vbox->itemAt(slot);
                  b = static_cast<PaletteBoxButton*>(item->widget());
                  // fall through

            case PALETTE_EDIT:
                  {
                  PaletteProperties pp(palette, 0);
                  int rv = pp.exec();
                  if (rv == 1) {
                        emit changed();
                        b->setText(palette->name());
                        palette->update();
                        }
                  }
                  emit changed();
                  break;

            case PALETTE_UP:
                  if (slot) {
                        QLayoutItem* i1 = vbox->itemAt(slot);
                        QLayoutItem* i2 = vbox->itemAt(slot+1);
                        vbox->removeItem(i1);
                        vbox->removeItem(i2);
                        vbox->insertWidget(slot-2, i2->widget());
                        vbox->insertWidget(slot-2, i1->widget());
                        delete i1;
                        delete i2;
                        for (int i = 0; i < (vbox->count() - 1) / 2; ++i)
                              static_cast<PaletteBoxButton*>(vbox->itemAt(i * 2)->widget())->setId(i*2);
                        emit changed();
                        }
                  break;

            case PALETTE_DOWN:
                  if (slot < (vbox->count() - 3)) {
                        QLayoutItem* i1 = vbox->itemAt(slot);
                        QLayoutItem* i2 = vbox->itemAt(slot+1);
                        vbox->removeItem(i1);
                        vbox->removeItem(i2);
                        vbox->insertWidget(slot+2, i2->widget());
                        vbox->insertWidget(slot+2, i1->widget());
                        delete i1;
                        delete i2;
                        for (int i = 0; i < (vbox->count() - 1) / 2; ++i)
                              static_cast<PaletteBoxButton*>(vbox->itemAt(i * 2)->widget())->setId(i*2);
                        emit changed();
                        }
                  break;

            }
      }

//---------------------------------------------------------
//   closeAll
//---------------------------------------------------------

void PaletteBox::closeAll()
      {
      for (int i = 0; i < (vbox->count() - 1); i += 2) {
            PaletteBoxButton* b = static_cast<PaletteBoxButton*> (vbox->itemAt(i)->widget() );
            b->showPalette(false);
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void PaletteBox::write(Xml& xml)
      {
      xml.stag("PaletteBox");
      for (int i = 0; i < (vbox->count() - 1); i += 2) {
            Palette* palette = static_cast<Palette*>(vbox->itemAt(i+1)->widget());
            palette->write(xml);
            }
      xml.etag();
      }

//---------------------------------------------------------
//   palettes
//---------------------------------------------------------

QList<Palette*> PaletteBox::palettes()const
      {
      QList<Palette*> pl;
      for (int i = 0; i < (vbox->count() - 1); i += 2)
            pl.append(static_cast<Palette*>(vbox->itemAt(i+1)->widget()));
      return pl;
      }

//---------------------------------------------------------
//   read
//    return false on error
//---------------------------------------------------------

bool PaletteBox::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "Palette") {
                  Palette* p = new Palette();
                  QString name = e.attribute("name");
                  p->setName(name);
                  p->read(e);
                  addPalette(p);
                  connect(p, SIGNAL(displayMore(const QString&)),
                     SLOT(displayMore(const QString&)));
                  }
            else
                  e.unknown();
            }
      return true;
      }

//---------------------------------------------------------
//   displayMore
//---------------------------------------------------------

void PaletteBox::displayMore(const QString& s)
      {
      mscore->showMasterPalette(s);
      }
}

