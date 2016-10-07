//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: palettebox.cpp 5576 2012-04-24 19:15:22Z wschweer $
//
//  Copyright (C) 2011-2016 Werner Schweer and others
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
#include "workspace.h"

namespace Ms {

//---------------------------------------------------------
//   PaletteBox
//---------------------------------------------------------

PaletteBox::PaletteBox(QWidget* parent)
   : QDockWidget(tr("Palettes"), parent)
      {
      setContextMenuPolicy(Qt::ActionsContextMenu);
      setObjectName("palette-box");
      setAllowedAreas(Qt::DockWidgetAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea));

      singlePaletteAction = new QAction(this);
      singlePaletteAction->setCheckable(true);
      singlePaletteAction->setChecked(preferences.singlePalette);
      addAction(singlePaletteAction);
      connect(singlePaletteAction, SIGNAL(toggled(bool)), SLOT(setSinglePalette(bool)));

      QWidget* w = new QWidget(this);
      w->setContextMenuPolicy(Qt::NoContextMenu);
      QVBoxLayout* vl = new QVBoxLayout(w);
      vl->setMargin(0);
      QHBoxLayout* hl = new QHBoxLayout;
      hl->setContentsMargins(5,0,5,0);

      workspaceList = new QComboBox;
      hl->addWidget(workspaceList);
      addWorkspaceButton = new QToolButton;

      addWorkspaceButton->setMinimumHeight(27);
      hl->addWidget(addWorkspaceButton);

      setWidget(w);

      searchBox = new QLineEdit(this);
      searchBox->setPlaceholderText(tr("Filter"));
      searchBox->setClearButtonEnabled(true);
      connect(searchBox, SIGNAL(textChanged(const QString&)), this, SLOT(filterPalettes(const QString&)));
      QHBoxLayout* hlSearch = new QHBoxLayout;
      hlSearch->setContentsMargins(5,0,5,0);
      hlSearch->addWidget(searchBox);

      PaletteBoxScrollArea* sa = new PaletteBoxScrollArea;
      sa->setFocusPolicy(Qt::NoFocus);
      sa->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
      sa->setContextMenuPolicy(Qt::CustomContextMenu);
      sa->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      sa->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
      sa->setWidgetResizable(true);
      sa->setFrameShape(QFrame::NoFrame);
      vl->addWidget(sa);
      vl->addLayout(hlSearch);
      vl->addLayout(hl);

      QWidget* paletteList = new QWidget;
      sa->setWidget(paletteList);
      vbox = new QVBoxLayout;
      paletteList->setLayout(vbox);
      vbox->setMargin(0);
      vbox->setSpacing(1);
      vbox->addStretch();
      paletteList->show();

      connect(addWorkspaceButton, SIGNAL(clicked()), SLOT(newWorkspaceClicked()));
      connect(workspaceList, SIGNAL(activated(int)), SLOT(workspaceSelected(int)));
      retranslate();
      }

//---------------------------------------------------------
//   retranslate
//---------------------------------------------------------

void PaletteBox::retranslate()
      {
      setWindowTitle(tr("Palettes"));
      singlePaletteAction->setText(tr("Single Palette"));
      workspaceList->setToolTip(tr("Select workspace"));
      addWorkspaceButton->setText(tr("+"));
      addWorkspaceButton->setToolTip(tr("Add new workspace"));
      updateWorkspaces();
      }

//---------------------------------------------------------
//   retransfilterPaletteslate
//---------------------------------------------------------

void PaletteBox::filterPalettes(const QString& text)
      {
      for (int i = 0; i < vbox->count(); i++) {
            QWidgetItem* wi = static_cast<QWidgetItem*>(vbox->itemAt(i));
            PaletteBoxButton* b = static_cast<PaletteBoxButton*>(wi->widget());
            i++;
            wi = static_cast<QWidgetItem*>(vbox->itemAt(i));
            if (!wi) return;
            Palette* p = static_cast<Palette*>(wi->widget());
            bool f = p->filter(text);
            b->setVisible(!f);
            if (b->isVisible()) {
                 if (text.isEmpty())
                      b->showPalette(false);
                 else
                      b->showPalette(true);
                 }
            else
                 b->showPalette(false);
            }
      }

//---------------------------------------------------------
//   workspaceSelected
//---------------------------------------------------------

void PaletteBox::workspaceSelected(int idx)
      {
      Workspace* w = Workspace::workspaces().at(idx);
      preferences.workspace = w->name();
      preferences.dirty = true;
      mscore->changeWorkspace(w);
      }

//---------------------------------------------------------
//   newWorkspaceClicked
//---------------------------------------------------------

void PaletteBox::newWorkspaceClicked()
      {
      mscore->createNewWorkspace();
      updateWorkspaces();
      }

//---------------------------------------------------------
//   updateWorkspaces
//---------------------------------------------------------

void PaletteBox::updateWorkspaces()
      {
      workspaceList->clear();
      const QList<Workspace*> pl = Workspace::workspaces();
      int idx = 0;
      int curIdx = -1;
      for (Workspace* p : pl) {
            workspaceList->addItem(qApp->translate("Ms::Workspace", p->name().toUtf8()), p->path());
            if (p->name() == preferences.workspace)
                  curIdx = idx;
            ++idx;
            }
      if (curIdx != -1)
            workspaceList->setCurrentIndex(curIdx);
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
      vbox->insertWidget(slotIdx+1, w, paletteStretch);

      connect(b, SIGNAL(paletteCmd(PaletteCommand,int)), SLOT(paletteCmd(PaletteCommand,int)));
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
      vbox->insertWidget(slot+1, p, paletteStretch);
      connect(b, SIGNAL(paletteCmd(PaletteCommand, int)), SLOT(paletteCmd(PaletteCommand, int)));
      connect(p, SIGNAL(changed()), Workspace::currentWorkspace, SLOT(setDirty()));
      for (int i = 0; i < (vbox->count() - 1) / 2; ++i)
            static_cast<PaletteBoxButton*>(vbox->itemAt(i * 2)->widget())->setId(i*2);
      return p;
      }

//---------------------------------------------------------
//   paletteCmd
//---------------------------------------------------------

void PaletteBox::paletteCmd(PaletteCommand cmd, int slot)
      {
      QLayoutItem* item   = vbox->itemAt(slot);
      PaletteBoxButton* b = static_cast<PaletteBoxButton*>(item->widget());
      Palette* palette    = static_cast<Palette*>(vbox->itemAt(slot+1)->widget());

      switch(cmd) {
            case PaletteCommand::PDELETE:
                  {
                  QMessageBox::StandardButton reply;
                  reply = QMessageBox::question(0,
                             QWidget::tr("Are you sure?"),
                             QWidget::tr("Do you really want to delete the '%1' palette?").arg(palette->name()),
                             QMessageBox::Yes | QMessageBox::No,
                             QMessageBox::Yes
                             );
                  if (reply == QMessageBox::Yes) {
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
                  }
                  break;

            case PaletteCommand::SAVE:
                  {
                  QString path = mscore->getPaletteFilename(false, palette->name());
                  if (!path.isEmpty())
                        palette->write(path);
                  }
                  break;

            case PaletteCommand::LOAD:
                  {
                  QString path = mscore->getPaletteFilename(true);
                  if (!path.isEmpty()) {
                        QFileInfo fi(path);
                        Palette* palette = newPalette(fi.completeBaseName(), slot);
                        palette->read(path);
                        }
                  }
                  emit changed();
                  break;

            case PaletteCommand::NEW:
                  palette = newPalette(tr("new Palette"), slot);
                  item   = vbox->itemAt(slot);
                  b = static_cast<PaletteBoxButton*>(item->widget());
                  // fall through

            case PaletteCommand::EDIT:
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

            case PaletteCommand::UP:
                  if (slot) {
                        QLayoutItem* i1 = vbox->itemAt(slot);
                        QLayoutItem* i2 = vbox->itemAt(slot+1);
                        vbox->removeItem(i1);
                        vbox->removeItem(i2);
                        vbox->insertWidget(slot-2, i2->widget(), paletteStretch);
                        vbox->insertWidget(slot-2, i1->widget());
                        delete i1;
                        delete i2;
                        for (int i = 0; i < (vbox->count() - 1) / 2; ++i)
                              static_cast<PaletteBoxButton*>(vbox->itemAt(i * 2)->widget())->setId(i*2);
                        emit changed();
                        }
                  break;

            case PaletteCommand::DOWN:
                  if (slot < (vbox->count() - 3)) {
                        QLayoutItem* i1 = vbox->itemAt(slot);
                        QLayoutItem* i2 = vbox->itemAt(slot+1);
                        vbox->removeItem(i1);
                        vbox->removeItem(i2);
                        vbox->insertWidget(slot+2, i2->widget(), paletteStretch);
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
                  connect(p, SIGNAL(displayMore(const QString&)), mscore, SLOT(showMasterPalette(const QString&)));
                  }
            else
                  e.unknown();
            }
      return true;
      }

//---------------------------------------------------------
//   sizeHint
//---------------------------------------------------------

QSize PaletteBoxScrollArea::sizeHint() const
      {
      return QSize(170 * guiScaling, 170 * guiScaling);
      }

//---------------------------------------------------------
//   setSinglePalette
//---------------------------------------------------------

void PaletteBox::setSinglePalette(bool val)
      {
      preferences.singlePalette = val;
      preferences.dirty = true;
      }

//---------------------------------------------------------
//   changeEvent
//---------------------------------------------------------

void PaletteBox::changeEvent(QEvent *event)
      {
      QDockWidget::changeEvent(event);
      if (event->type() == QEvent::LanguageChange)
            retranslate();
      }
}

