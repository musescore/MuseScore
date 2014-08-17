//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: stafftext.cpp -1   $
//
//  Copyright (C) 2008-2010 Werner Schweer and others
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

#include "libmscore/score.h"
#include "stafftextproperties.h"
#include "libmscore/stafftext.h"
#include "libmscore/system.h"
#include "libmscore/staff.h"

namespace Ms {

//---------------------------------------------------------
// initChannelCombo
//---------------------------------------------------------

static void initChannelCombo(QComboBox* cb, StaffText* st)
      {
      Part* part = st->staff()->part();
      int tick = static_cast<Segment*>(st->parent())->tick();
      foreach(const Channel& a, part->instr(tick)->channel()) {
            if (a.name.isEmpty() || a.name == "normal")
                  cb->addItem(QObject::tr("normal"));
            else
                  cb->addItem(a.name);
            }
      }

//---------------------------------------------------------
//   StaffTextProperties
//---------------------------------------------------------

StaffTextProperties::StaffTextProperties(StaffText* st, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      if (st->systemFlag())
            setWindowTitle(tr("MuseScore: System Text Properties"));
      else
            setWindowTitle(tr("MuseScore: Staff Text Properties"));
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      staffText = st;

#ifndef AEOLUS
	tabWidget->removeTab(2);
#endif

      vb[0][0] = voice1_1;
      vb[0][1] = voice1_2;
      vb[0][2] = voice1_3;
      vb[0][3] = voice1_4;

      vb[1][0] = voice2_1;
      vb[1][1] = voice2_2;
      vb[1][2] = voice2_3;
      vb[1][3] = voice2_4;

      vb[2][0] = voice3_1;
      vb[2][1] = voice3_2;
      vb[2][2] = voice3_3;
      vb[2][3] = voice3_4;

      vb[3][0] = voice4_1;
      vb[3][1] = voice4_2;
      vb[3][2] = voice4_3;
      vb[3][3] = voice4_4;

      channelCombo[0] = channelCombo1;
      channelCombo[1] = channelCombo2;
      channelCombo[2] = channelCombo3;
      channelCombo[3] = channelCombo4;

      //---------------------------------------------------
      // setup "switch channel"
      //---------------------------------------------------

      for (int i = 0; i < 4; ++i)
            initChannelCombo(channelCombo[i], st);

      Part* part = st->staff()->part();
      int tick = static_cast<Segment*>(st->parent())->tick();
      int n = part->instr(tick)->channel().size();
      int rows = 0;
      for (int voice = 0; voice < VOICES; ++voice) {
            if (st->channelName(voice).isEmpty())
                  continue;
            for (int i = 0; i < n; ++i) {
                  const Channel& a = part->instr(tick)->channel(i);
                  if (a.name != st->channelName(voice))
                        continue;
                  int row = 0;
                  for (row = 0; row < rows; ++row) {
                        if (channelCombo[row]->currentIndex() == i) {
                              vb[voice][row]->setChecked(true);
                              break;
                              }
                        }
                  if (row == rows) {
                        vb[voice][rows]->setChecked(true);
                        channelCombo[row]->setCurrentIndex(i);
                        ++rows;
                        }
                  break;
                  }
            }
      QSignalMapper* mapper = new QSignalMapper(this);
      for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                  connect(vb[col][row], SIGNAL(clicked()), mapper, SLOT(map()));
                  mapper->setMapping(vb[col][row], (col << 8) + row);
                  }
            }

      if (staffText->swing()) {
            setSwingBox->setChecked(true);
            if (st->swingParameters()->swingUnit == MScore::division/2) {
                  swingBox->setEnabled(true);
                  swingEighth->setChecked(true);
                  swingBox->setValue(st->swingParameters()->swingRatio);
                  }
            else if (st->swingParameters()->swingUnit == MScore::division/4) {
                  swingBox->setEnabled(true);
                  swingSixteenth->setChecked(true);
                  swingBox->setValue(st->swingParameters()->swingRatio);
                  }
            else if (st->swingParameters()->swingUnit == 0) {
                 swingBox->setEnabled(false);
                 SwingOff->setChecked(true);
                 swingBox->setValue(st->swingParameters()->swingRatio);
                 }
            }

      connect(mapper, SIGNAL(mapped(int)), SLOT(voiceButtonClicked(int)));
      connect(SwingOff, SIGNAL(toggled(bool)), SLOT(setSwingControls(bool)));
      connect(swingEighth, SIGNAL(toggled(bool)), SLOT(setSwingControls(bool)));
      connect(swingSixteenth, SIGNAL(toggled(bool)), SLOT(setSwingControls(bool)));

      //---------------------------------------------------
      //    setup midi actions
      //---------------------------------------------------

      QTreeWidgetItem* selectedItem = 0;
      for (int i = 0; i < n; ++i) {
            const Channel& a = part->instr(tick)->channel(i);
            QTreeWidgetItem* item = new QTreeWidgetItem(channelList);
            item->setData(0, Qt::UserRole, i);
            if (a.name.isEmpty() || a.name == "normal")
                  item->setText(0, tr("normal"));
            else
                  item->setText(0, a.name);
            item->setText(1, a.descr);
            if (i == 0)
                  selectedItem = item;
            }
      connect(channelList, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
         SLOT(channelItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)));
      connect(this, SIGNAL(accepted()), SLOT(saveValues()));
      channelList->setCurrentItem(selectedItem);

      //---------------------------------------------------
      //    setup aeolus stops
      //---------------------------------------------------

      changeStops->setChecked(staffText->setAeolusStops());

      for (int i = 0; i < 4; ++i) {
            for (int k = 0; k < 16; ++k)
                  stops[i][k] = 0;
            }
      stops[0][0]  = stop_3_0;
      stops[0][1]  = stop_3_1;
      stops[0][2]  = stop_3_2;
      stops[0][3]  = stop_3_3;
      stops[0][4]  = stop_3_4;
      stops[0][5]  = stop_3_5;
      stops[0][6]  = stop_3_6;
      stops[0][7]  = stop_3_7;
      stops[0][8]  = stop_3_8;
      stops[0][9]  = stop_3_9;
      stops[0][10] = stop_3_10;
      stops[0][11] = stop_3_11;

      stops[1][0]  = stop_2_0;
      stops[1][1]  = stop_2_1;
      stops[1][2]  = stop_2_2;
      stops[1][3]  = stop_2_3;
      stops[1][4]  = stop_2_4;
      stops[1][5]  = stop_2_5;
      stops[1][6]  = stop_2_6;
      stops[1][7]  = stop_2_7;
      stops[1][8]  = stop_2_8;
      stops[1][9]  = stop_2_9;
      stops[1][10] = stop_2_10;
      stops[1][11] = stop_2_11;
      stops[1][12] = stop_2_12;

      stops[2][0]  = stop_1_0;
      stops[2][1]  = stop_1_1;
      stops[2][2]  = stop_1_2;
      stops[2][3]  = stop_1_3;
      stops[2][4]  = stop_1_4;
      stops[2][5]  = stop_1_5;
      stops[2][6]  = stop_1_6;
      stops[2][7]  = stop_1_7;
      stops[2][8]  = stop_1_8;
      stops[2][9]  = stop_1_9;
      stops[2][10] = stop_1_10;
      stops[2][11] = stop_1_11;
      stops[2][12] = stop_1_12;
      stops[2][13] = stop_1_13;
      stops[2][14] = stop_1_14;
      stops[2][15] = stop_1_15;

      stops[3][0]  = stop_p_0;
      stops[3][1]  = stop_p_1;
      stops[3][2]  = stop_p_2;
      stops[3][3]  = stop_p_3;
      stops[3][4]  = stop_p_4;
      stops[3][5]  = stop_p_5;
      stops[3][6]  = stop_p_6;
      stops[3][7]  = stop_p_7;
      stops[3][8]  = stop_p_8;
      stops[3][9]  = stop_p_9;
      stops[3][10] = stop_p_10;
      stops[3][11] = stop_p_11;
      stops[3][12] = stop_p_12;
      stops[3][13] = stop_p_13;
      stops[3][14] = stop_p_14;
      stops[3][15] = stop_p_15;

      curTabIndex = tabWidget->currentIndex();
      connect(tabWidget, SIGNAL(currentChanged(int)), SLOT(tabChanged(int)));
      }

//---------------------------------------------------------
//   setSwingParameters
//---------------------------------------------------------

void StaffTextProperties::setSwingControls(bool checked)
      {
      if (!checked)
            return;
      if (SwingOff->isChecked())
            swingBox->setEnabled(false);
      else if (swingEighth->isChecked())
            swingBox->setEnabled(true);
      else if (swingSixteenth->isChecked())
            swingBox->setEnabled(true);
      }

//---------------------------------------------------------
//   tabChanged
//---------------------------------------------------------

void StaffTextProperties::tabChanged(int tab)
      {
      if (tab == 2) {
            for (int i = 0; i < 4; ++i) {
                  for (int k = 0; k < 16; ++k) {
                        if (stops[i][k])
                              stops[i][k]->setChecked(staffText->getAeolusStop(i, k));
                        }
                  }
            }
      if (curTabIndex == 2) {
            staffText->setSetAeolusStops(changeStops->isChecked());
            for (int i = 0; i < 4; ++i) {
                  for (int k = 0; k < 16; ++k) {
                        if (stops[i][k])
                              staffText->setAeolusStop(i, k, stops[i][k]->isChecked());
                        }
                  }
            }
      curTabIndex = tab;
      }

//---------------------------------------------------------
//   voiceButtonClicked
//---------------------------------------------------------

void StaffTextProperties::voiceButtonClicked(int val)
      {
      int ccol = val >> 8;
      int crow = val & 0xff;
      for (int row = 0; row < 4; ++row) {
            if (row == crow)
                  continue;
            vb[ccol][row]->setChecked(false);
            }
      }

//---------------------------------------------------------
//   saveChannel
//---------------------------------------------------------

void StaffTextProperties::saveChannel(int channel)
      {
      QList<ChannelActions>* ca = staffText->channelActions();
      int n = ca->size();
      for (int i = 0; i < n; ++i) {
            ChannelActions* a = &(*ca)[i];
            if (a->channel == channel) {
                  ca->removeAt(i);
                  break;
                  }
            }

      ChannelActions a;
      a.channel = channel;

//      QList<QTreeWidgetItem*> items = actionList->selectedItems();
      for (int i = 0; i < actionList->topLevelItemCount(); ++i) {
            QTreeWidgetItem* item = actionList->topLevelItem(i);
            if (item->isSelected())
                  a.midiActionNames.append(item->text(0));
            }
      ca->append(a);
      }

//---------------------------------------------------------
//   channelItemChanged
//---------------------------------------------------------

void StaffTextProperties::channelItemChanged(QTreeWidgetItem* item, QTreeWidgetItem* pitem)
      {
      if (pitem)
            saveChannel(pitem->data(0, Qt::UserRole).toInt());
      if (item == 0)
            return;

      actionList->clear();
      Part* part = staffText->staff()->part();

      int channelIdx      = item->data(0, Qt::UserRole).toInt();
      int tick = static_cast<Segment*>(staffText->parent())->tick();
      Channel& channel    = part->instr(tick)->channel(channelIdx);
      QString channelName = channel.name;

      foreach(const NamedEventList& e, part->instr(tick)->midiActions()) {
            QTreeWidgetItem* item = new QTreeWidgetItem(actionList);
            if (e.name.isEmpty() || e.name == "normal")
                  item->setText(0, tr("normal"));
            else
                  item->setText(0, e.name);
            item->setText(1, e.descr);
            }
      foreach(const NamedEventList& e, channel.midiActions) {
            QTreeWidgetItem* item = new QTreeWidgetItem(actionList);
            if (e.name.isEmpty() || e.name == "normal")
                  item->setText(0, tr("normal"));
            else
                  item->setText(0, e.name);
            item->setText(1, e.descr);
            }
      foreach(const ChannelActions& ca, *staffText->channelActions()) {
            if (ca.channel == channelIdx) {
                  foreach(QString s, ca.midiActionNames) {
                        QList<QTreeWidgetItem*> items = actionList->findItems(s, Qt::MatchExactly);
                        foreach(QTreeWidgetItem* item, items)
                              item->setSelected(true);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   saveValues
//---------------------------------------------------------

void StaffTextProperties::saveValues()
      {
      //
      // save channel switches
      //
      Part* part = staffText->staff()->part();
      for (int voice = 0; voice < VOICES; ++voice) {
            staffText->setChannelName(voice, QString());
            for (int row = 0; row < VOICES; ++row) {
                  if (vb[voice][row]->isChecked()) {
                        int idx     = channelCombo[row]->currentIndex();
                        int instrId = static_cast<Segment*>(staffText->parent())->tick();
                        staffText->setChannelName(voice, part->instr(instrId)->channel()[idx].name);
                        break;
                        }
                  }
            }

      QTreeWidgetItem* pitem = channelList->currentItem();
      if (pitem)
            saveChannel(pitem->data(0, Qt::UserRole).toInt());

      //
      // save Aeolus stops
      //
      staffText->setSetAeolusStops(changeStops->isChecked());
      if (changeStops->isChecked()) {
            for (int i = 0; i < 4; ++i) {
                  for (int k = 0; k < 16; ++k) {
                        if (stops[i][k])
                              staffText->setAeolusStop(i, k, stops[i][k]->isChecked());
                        }
                  }
            }
      if (setSwingBox->isChecked()) {
            staffText->setSwing(true);
            if (SwingOff->isChecked()) {
                  staffText->setSwingParameters(0, swingBox->value());
                  swingBox->setEnabled(false);
                  }
            else if (swingEighth->isChecked()) {
                  staffText->setSwingParameters(MScore::division/2, swingBox->value());
                  swingBox->setEnabled(true);
                  }
            else if (swingSixteenth->isChecked()) {
                  staffText->setSwingParameters(MScore::division/4, swingBox->value());
                  swingBox->setEnabled(true);
                  }
            }
      staffText->score()->updateChannel();
      staffText->score()->updateSwing();
      staffText->score()->setPlaylistDirty(true);
      }
}

