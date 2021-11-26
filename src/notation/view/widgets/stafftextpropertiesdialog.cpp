/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "stafftextpropertiesdialog.h"

#include <QSignalMapper>
#include <QToolButton>

#include "engraving/libmscore/score.h"
#include "engraving/libmscore/stafftext.h"
#include "engraving/libmscore/system.h"
#include "engraving/libmscore/staff.h"
#include "engraving/libmscore/segment.h"

#include "ui/view/widgetstatestore.h"

using namespace mu::notation;
using namespace mu::ui;

static const QString STAFF_TEXT_PROPERTIES_DIALOG_NAME("StaffTextPropertiesDialog");

namespace Ms {
//---------------------------------------------------------
// initChannelCombo
//---------------------------------------------------------

static void initChannelCombo(QComboBox* cb, StaffTextBase* st)
{
    Part* part = st->staff()->part();
    Fraction tick = static_cast<Segment*>(st->explicitParent())->tick();
    for (const Channel* a : part->instrument(tick)->channel()) {
        QString name = a->name();
        if (a->name().isEmpty()) {
            name = Channel::DEFAULT_NAME;
        }
        cb->addItem(qApp->translate("InstrumentsXML", name.toUtf8().data()));
    }
}

//---------------------------------------------------------
//   StaffTextPropertiesDialog
//---------------------------------------------------------

StaffTextPropertiesDialog::StaffTextPropertiesDialog(QWidget* parent)
    : QDialog(parent)
{
    setObjectName(STAFF_TEXT_PROPERTIES_DIALOG_NAME);
    setupUi(this);

    const INotationPtr notation = globalContext()->currentNotation();
    const INotationInteractionPtr interaction = notation ? notation->interaction() : nullptr;
    EngravingItem* element = interaction ? interaction->hitElementContext().element : nullptr;
    StaffTextBase* st = element && element->isStaffTextBase() ? Ms::toStaffTextBase(element) : nullptr;

    if (!st) {
        return;
    }

    m_originStaffText = st;

    if (st->systemFlag()) {
        setWindowTitle(tr("System Text Properties"));
        tabWidget->removeTab(tabWidget->indexOf(tabAeolusStops));     // Aeolus settings  for staff text only
        //if (!enableExperimental) tabWidget->removeTab(tabWidget->indexOf(tabMIDIAction));
        tabWidget->removeTab(tabWidget->indexOf(tabChangeChannel));     // Channel switching  for staff text only
        tabWidget->removeTab(tabWidget->indexOf(tabCapoSettings));     // Capos for staff text only
    } else {
        setWindowTitle(tr("Staff Text Properties"));
        //tabWidget->removeTab(tabWidget->indexOf(tabSwingSettings)); // Swing settings for system text only, could be disabled here, if desired
#ifndef AEOLUS
        tabWidget->removeTab(tabWidget->indexOf(tabAeolusStops));
#endif
        //if (!enableExperimental) tabWidget->removeTab(tabWidget->indexOf(tabMIDIAction));
    }

    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    m_staffText = static_cast<StaffTextBase*>(st->clone());

    m_vb[0][0] = voice1_1;
    m_vb[0][1] = voice1_2;
    m_vb[0][2] = voice1_3;
    m_vb[0][3] = voice1_4;

    m_vb[1][0] = voice2_1;
    m_vb[1][1] = voice2_2;
    m_vb[1][2] = voice2_3;
    m_vb[1][3] = voice2_4;

    m_vb[2][0] = voice3_1;
    m_vb[2][1] = voice3_2;
    m_vb[2][2] = voice3_3;
    m_vb[2][3] = voice3_4;

    m_vb[3][0] = voice4_1;
    m_vb[3][1] = voice4_2;
    m_vb[3][2] = voice4_3;
    m_vb[3][3] = voice4_4;

    m_channelCombo[0] = channelCombo1;
    m_channelCombo[1] = channelCombo2;
    m_channelCombo[2] = channelCombo3;
    m_channelCombo[3] = channelCombo4;

    //---------------------------------------------------
    // setup "switch channel"
    //---------------------------------------------------

    for (int i = 0; i < VOICES; ++i) {
        initChannelCombo(m_channelCombo[i], m_staffText);
    }

    Part* part = m_staffText->staff()->part();
    Fraction tick = static_cast<Segment*>(st->explicitParent())->tick();
    int n = part->instrument(tick)->channel().size();
    int rows = 0;
    for (int voice = 0; voice < VOICES; ++voice) {
        if (m_staffText->channelName(voice).isEmpty()) {
            continue;
        }
        for (int i = 0; i < n; ++i) {
            const Channel* a = part->instrument(tick)->channel(i);
            if (a->name() != m_staffText->channelName(voice)) {
                continue;
            }
            int row = 0;
            for (row = 0; row < rows; ++row) {
                if (m_channelCombo[row]->currentIndex() == i) {
                    m_vb[voice][row]->setChecked(true);
                    break;
                }
            }
            if (row == rows) {
                m_vb[voice][rows]->setChecked(true);
                m_channelCombo[row]->setCurrentIndex(i);
                ++rows;
            }
            break;
        }
    }

    for (int i = 0; i < VOICES; ++i) {
        auto channel = m_channelCombo[i];
        channel->setAccessibleName(channelLabel->text() + channel->currentText());
    }

    QColor voiceUncheckedColor = QColor(uiConfiguration()->currentTheme().values[BUTTON_COLOR].toString());
    QList<QColor> voicesColors;
    for (int voice = 0; voice < VOICES; ++voice) {
        voicesColors << configuration()->selectionColor(voice);
    }

    QSignalMapper* mapper = new QSignalMapper(this);
    for (int row = 0; row < VOICES; ++row) {
        for (int col = 0; col < VOICES; ++col) {
            auto button = m_vb[col][row];

            mapper->setMapping(button, (col << 8) + row);
            button->setAccessibleName(voiceLabel->text() + button->text());

            connect(button, SIGNAL(clicked()), mapper, SLOT(map()));

            connect(button, &QToolButton::toggled, this, [=](){
                QColor color = button->isChecked() ? voicesColors[col] : voiceUncheckedColor;
                QPalette palette;
                palette.setColor(QPalette::Button, color);
                button->setPalette(palette);
            });
        }
    }

    if (m_staffText->swing()) {
        setSwingBox->setChecked(true);
        if (m_staffText->swingParameters()->swingUnit == Constant::division / 2) {
            swingBox->setEnabled(true);
            swingEighth->setChecked(true);
            swingBox->setValue(m_staffText->swingParameters()->swingRatio);
        } else if (m_staffText->swingParameters()->swingUnit == Constant::division / 4) {
            swingBox->setEnabled(true);
            swingSixteenth->setChecked(true);
            swingBox->setValue(m_staffText->swingParameters()->swingRatio);
        } else if (m_staffText->swingParameters()->swingUnit == 0) {
            swingBox->setEnabled(false);
            swingOff->setChecked(true);
            swingBox->setValue(m_staffText->swingParameters()->swingRatio);
        }
    }

    connect(mapper, &QSignalMapper::mappedInt, this, &StaffTextPropertiesDialog::voiceButtonClicked);
    connect(swingOff, &QRadioButton::toggled, this, &StaffTextPropertiesDialog::setSwingControls);
    connect(swingEighth, &QRadioButton::toggled, this, &StaffTextPropertiesDialog::setSwingControls);
    connect(swingSixteenth, &QRadioButton::toggled, this, &StaffTextPropertiesDialog::setSwingControls);

    //---------------------------------------------------
    //    setup capo
    //      Note that capo is stored as an int, where 0 = no change,
    //      1 = remove capo, and everyother number (n) = pitch increase
    //      of n-1 semitones.
    //---------------------------------------------------

    if (m_staffText->capo() != 0) {
        setCapoBox->setChecked(true);
        fretList->setCurrentIndex(m_staffText->capo() - 1);
    }

    //---------------------------------------------------
    //    setup midi actions
    //---------------------------------------------------

    QTreeWidgetItem* selectedItem = 0;
    for (int i = 0; i < n; ++i) {
        const Channel* a = part->instrument(tick)->channel(i);
        QTreeWidgetItem* item = new QTreeWidgetItem(channelList);
        item->setData(0, Qt::UserRole, i);
        QString name = a->name();
        if (a->name().isEmpty()) {
            name = Channel::DEFAULT_NAME;
        }
        item->setText(0, qApp->translate("InstrumentsXML", name.toUtf8().data()));
        item->setText(1, qApp->translate("InstrumentsXML", a->descr().toUtf8().data()));
        if (i == 0) {
            selectedItem = item;
        }
    }
    connect(channelList, &QTreeWidget::currentItemChanged,
            this, &StaffTextPropertiesDialog::channelItemChanged);
    connect(this, &QDialog::accepted, this, &StaffTextPropertiesDialog::saveValues);
    channelList->setCurrentItem(selectedItem);

    //---------------------------------------------------
    //    setup aeolus m_stops
    //---------------------------------------------------

    changeStops->setChecked(m_staffText->setAeolusStops());

    for (int i = 0; i < 4; ++i) {
        for (int k = 0; k < 16; ++k) {
            m_stops[i][k] = 0;
        }
    }
    m_stops[0][0]  = stop_3_0;
    m_stops[0][1]  = stop_3_1;
    m_stops[0][2]  = stop_3_2;
    m_stops[0][3]  = stop_3_3;
    m_stops[0][4]  = stop_3_4;
    m_stops[0][5]  = stop_3_5;
    m_stops[0][6]  = stop_3_6;
    m_stops[0][7]  = stop_3_7;
    m_stops[0][8]  = stop_3_8;
    m_stops[0][9]  = stop_3_9;
    m_stops[0][10] = stop_3_10;
    m_stops[0][11] = stop_3_11;

    m_stops[1][0]  = stop_2_0;
    m_stops[1][1]  = stop_2_1;
    m_stops[1][2]  = stop_2_2;
    m_stops[1][3]  = stop_2_3;
    m_stops[1][4]  = stop_2_4;
    m_stops[1][5]  = stop_2_5;
    m_stops[1][6]  = stop_2_6;
    m_stops[1][7]  = stop_2_7;
    m_stops[1][8]  = stop_2_8;
    m_stops[1][9]  = stop_2_9;
    m_stops[1][10] = stop_2_10;
    m_stops[1][11] = stop_2_11;
    m_stops[1][12] = stop_2_12;

    m_stops[2][0]  = stop_1_0;
    m_stops[2][1]  = stop_1_1;
    m_stops[2][2]  = stop_1_2;
    m_stops[2][3]  = stop_1_3;
    m_stops[2][4]  = stop_1_4;
    m_stops[2][5]  = stop_1_5;
    m_stops[2][6]  = stop_1_6;
    m_stops[2][7]  = stop_1_7;
    m_stops[2][8]  = stop_1_8;
    m_stops[2][9]  = stop_1_9;
    m_stops[2][10] = stop_1_10;
    m_stops[2][11] = stop_1_11;
    m_stops[2][12] = stop_1_12;
    m_stops[2][13] = stop_1_13;
    m_stops[2][14] = stop_1_14;
    m_stops[2][15] = stop_1_15;

    m_stops[3][0]  = stop_p_0;
    m_stops[3][1]  = stop_p_1;
    m_stops[3][2]  = stop_p_2;
    m_stops[3][3]  = stop_p_3;
    m_stops[3][4]  = stop_p_4;
    m_stops[3][5]  = stop_p_5;
    m_stops[3][6]  = stop_p_6;
    m_stops[3][7]  = stop_p_7;
    m_stops[3][8]  = stop_p_8;
    m_stops[3][9]  = stop_p_9;
    m_stops[3][10] = stop_p_10;
    m_stops[3][11] = stop_p_11;
    m_stops[3][12] = stop_p_12;
    m_stops[3][13] = stop_p_13;
    m_stops[3][14] = stop_p_14;
    m_stops[3][15] = stop_p_15;

    m_curTabIndex = tabWidget->currentIndex();
    connect(tabWidget, &QTabWidget::currentChanged, this, &StaffTextPropertiesDialog::tabChanged);

    WidgetStateStore::restoreGeometry(this);
}

StaffTextPropertiesDialog::StaffTextPropertiesDialog(const StaffTextPropertiesDialog& other)
    : QDialog(other.parentWidget())
{
}

StaffTextPropertiesDialog::~StaffTextPropertiesDialog()
{
    delete m_staffText;
}

void StaffTextPropertiesDialog::hideEvent(QHideEvent* event)
{
    WidgetStateStore::saveGeometry(this);
    QDialog::hideEvent(event);
}

int StaffTextPropertiesDialog::static_metaTypeId()
{
    return QMetaType::type(STAFF_TEXT_PROPERTIES_DIALOG_NAME.toStdString().c_str());
}

//---------------------------------------------------------
//   setSwingControls
//---------------------------------------------------------

void StaffTextPropertiesDialog::setSwingControls(bool checked)
{
    if (!checked) {
        return;
    }
    if (swingOff->isChecked()) {
        swingBox->setEnabled(false);
    } else if (swingEighth->isChecked()) {
        swingBox->setEnabled(true);
    } else if (swingSixteenth->isChecked()) {
        swingBox->setEnabled(true);
    }
}

//---------------------------------------------------------
//   tabChanged
//---------------------------------------------------------

void StaffTextPropertiesDialog::tabChanged(int tab)
{
    if (tab == 2) {
        for (int i = 0; i < 4; ++i) {
            for (int k = 0; k < 16; ++k) {
                if (m_stops[i][k]) {
                    m_stops[i][k]->setChecked(m_staffText->getAeolusStop(i, k));
                }
            }
        }
    }
    if (m_curTabIndex == 2) {
        m_staffText->setSetAeolusStops(changeStops->isChecked());
        for (int i = 0; i < 4; ++i) {
            for (int k = 0; k < 16; ++k) {
                if (m_stops[i][k]) {
                    m_staffText->setAeolusStop(i, k, m_stops[i][k]->isChecked());
                }
            }
        }
    }
    m_curTabIndex = tab;
}

//---------------------------------------------------------
//   voiceButtonClicked
//---------------------------------------------------------

void StaffTextPropertiesDialog::voiceButtonClicked(int val)
{
    int ccol = val >> 8;
    int crow = val & 0xff;
    for (int row = 0; row < VOICES; ++row) {
        if (row == crow) {
            continue;
        }
        m_vb[ccol][row]->setChecked(false);
    }
}

//---------------------------------------------------------
//   saveChannel
//---------------------------------------------------------

void StaffTextPropertiesDialog::saveChannel(int channel)
{
    QList<ChannelActions>* ca = m_staffText->channelActions();
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

    for (int i = 0; i < actionList->topLevelItemCount(); ++i) {
        QTreeWidgetItem* item = actionList->topLevelItem(i);
        if (item->isSelected()) {
            a.midiActionNames.append(item->text(0));
        }
    }
    ca->append(a);
}

//---------------------------------------------------------
//   channelItemChanged
//---------------------------------------------------------

void StaffTextPropertiesDialog::channelItemChanged(QTreeWidgetItem* item, QTreeWidgetItem* pitem)
{
    if (pitem) {
        saveChannel(pitem->data(0, Qt::UserRole).toInt());
    }
    if (item == 0) {
        return;
    }

    actionList->clear();
    Part* part = m_staffText->staff()->part();

    int channelIdx      = item->data(0, Qt::UserRole).toInt();
    Fraction tick = static_cast<Segment*>(m_staffText->explicitParent())->tick();
    Channel* channel    = part->instrument(tick)->channel(channelIdx);
    QString channelName = channel->name();

    for (const NamedEventList& e : part->instrument(tick)->midiActions()) {
        QTreeWidgetItem* ti = new QTreeWidgetItem(actionList);
        QString name = e.name;
        if (e.name.isEmpty()) {
            name = Channel::DEFAULT_NAME;
        }
        ti->setText(0, qApp->translate("InstrumentsXML", name.toUtf8().data()));
        ti->setData(0, Qt::UserRole, name);
        ti->setText(1, qApp->translate("InstrumentsXML", e.descr.toUtf8().data()));
    }
    for (const NamedEventList& e : channel->midiActions) {
        QTreeWidgetItem* ti = new QTreeWidgetItem(actionList);
        QString name = e.name;
        if (e.name.isEmpty()) {
            name = Channel::DEFAULT_NAME;
        }
        ti->setText(0, qApp->translate("InstrumentsXML", name.toUtf8().data()));
        ti->setData(0, Qt::UserRole, name);
        ti->setText(1, qApp->translate("InstrumentsXML", e.descr.toUtf8().data()));
    }
    for (const ChannelActions& ca : *m_staffText->channelActions()) {
        if (ca.channel == channelIdx) {
            for (QString s : ca.midiActionNames) {
                QList<QTreeWidgetItem*> items;
                for (int i = 0; i < actionList->topLevelItemCount(); i++) {
                    QTreeWidgetItem* ti = actionList->topLevelItem(i);
                    if (ti->data(0, Qt::UserRole) == s) {
                        ti->setSelected(true);
                    }
                }
            }
        }
    }
}

//---------------------------------------------------------
//   saveValues
//---------------------------------------------------------

void StaffTextPropertiesDialog::saveValues()
{
    //
    // save channel switches
    //
    Part* part = m_staffText->staff()->part();
    for (int voice = 0; voice < VOICES; ++voice) {
        m_staffText->setChannelName(voice, QString());
        for (int row = 0; row < VOICES; ++row) {
            if (m_vb[voice][row]->isChecked()) {
                int idx     = m_channelCombo[row]->currentIndex();
                Fraction instrId = static_cast<Segment*>(m_staffText->explicitParent())->tick();
                m_staffText->setChannelName(voice, part->instrument(instrId)->channel(idx)->name());
                break;
            }
        }
    }

    QTreeWidgetItem* pitem = channelList->currentItem();
    if (pitem) {
        saveChannel(pitem->data(0, Qt::UserRole).toInt());
    }

    //
    // save Aeolus m_stops
    //
    m_staffText->setSetAeolusStops(changeStops->isChecked());
    if (changeStops->isChecked()) {
        for (int i = 0; i < 4; ++i) {
            for (int k = 0; k < 16; ++k) {
                if (m_stops[i][k]) {
                    m_staffText->setAeolusStop(i, k, m_stops[i][k]->isChecked());
                }
            }
        }
    }
    if (setSwingBox->isChecked()) {
        m_staffText->setSwing(true);
        if (swingOff->isChecked()) {
            m_staffText->setSwingParameters(0, swingBox->value());
            swingBox->setEnabled(false);
        } else if (swingEighth->isChecked()) {
            m_staffText->setSwingParameters(Constant::division / 2, swingBox->value());
            swingBox->setEnabled(true);
        } else if (swingSixteenth->isChecked()) {
            m_staffText->setSwingParameters(Constant::division / 4, swingBox->value());
            swingBox->setEnabled(true);
        }
    }

    if (setCapoBox->isChecked()) {
        m_staffText->setCapo(fretList->currentIndex() + 1);
    } else {
        m_staffText->setCapo(0);
    }

    Score* score = m_originStaffText->score();
    StaffTextBase* nt = toStaffTextBase(m_staffText->clone());
    nt->setScore(score);
    score->undoChangeElement(m_originStaffText, nt);
    score->masterScore()->updateChannel();
    score->updateCapo();
    score->updateSwing();
    score->setPlaylistDirty();
}
}
