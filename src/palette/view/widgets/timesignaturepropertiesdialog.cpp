/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "timesignaturepropertiesdialog.h"

#include <QRegularExpression>

#include "engraving/iengravingfont.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/score.h"
#include "engraving/dom/timesig.h"

#include "ui/view/musicalsymbolcodes.h"
#include "ui/view/widgetstatestore.h"

static const QString TIME_SIGNATURE_PROPERTIES_DIALOG_NAME("TimeSignaturePropertiesDialog");

using namespace mu::palette;
using namespace muse::ui;
using namespace mu::notation;
using namespace mu::engraving;

//---------------------------------------------------------
//    TimeSigProperties
//---------------------------------------------------------

TimeSignaturePropertiesDialog::TimeSignaturePropertiesDialog(QWidget* parent)
    : QDialog(parent)
{
    setObjectName(TIME_SIGNATURE_PROPERTIES_DIALOG_NAME);
    setupUi(this);

    QString musicalFontFamily = QString::fromStdString(uiConfiguration()->musicalFontFamily());
    int musicalFontSize = uiConfiguration()->musicalFontSize();

    QString radioButtonStyle = QString("QRadioButton { font-family: %1; font-size: %2px }")
                               .arg(musicalFontFamily)
                               .arg(musicalFontSize);

    fourfourButton->setStyleSheet(radioButtonStyle);
    fourfourButton->setText(musicalSymbolToString(MusicalSymbolCodes::Code::TIMESIG_COMMON));
    fourfourButton->setMaximumHeight(30);
    allaBreveButton->setStyleSheet(radioButtonStyle);
    allaBreveButton->setText(musicalSymbolToString(MusicalSymbolCodes::Code::TIMESIG_CUT));
    allaBreveButton->setMaximumHeight(30);

    setWindowFlag(Qt::WindowContextHelpButtonHint, false);

    const INotationInteractionPtr interaction = notation() ? notation()->interaction() : nullptr;
    EngravingItem* element = interaction ? interaction->hitElementContext().element : nullptr;
    m_originTimeSig = element ? toTimeSig(element) : nullptr;

    if (!m_originTimeSig) {
        return;
    }

    m_editedTimeSig = m_originTimeSig->clone();

    zText->setText(m_editedTimeSig->numeratorString());
    nText->setText(m_editedTimeSig->denominatorString());
    // set validators for numerator and denominator strings
    // which only accept '+', '*' (or 'x'), '(', ')', digits and some time symb conventional representations
    QRegularExpression regex("[0-9+COXx()\\*\\x00A2\\x00D7\\x00D8\\x00BD\\x00BC]*");
    QValidator* validator = new QRegularExpressionValidator(regex, this);
    zText->setValidator(validator);
    nText->setValidator(validator);

    Fraction nominal = m_editedTimeSig->sig() / m_editedTimeSig->stretch();
    nominal.reduce();

    // TODO: fix https://musescore.org/en/node/42341
    // for now, editing of actual (local) time sig is disabled in dialog
    // but more importantly, the dialog should make it clear that this is "local" change only
    // and not normally the right way to add 7/4 to a score
    switch (m_editedTimeSig->timeSigType()) {
    case TimeSigType::NORMAL:
        textButton->setChecked(true);
        break;
    case TimeSigType::FOUR_FOUR:
        fourfourButton->setChecked(true);
        break;
    case TimeSigType::ALLA_BREVE:
        allaBreveButton->setChecked(true);
        break;
    case TimeSigType::CUT_BACH:
//                  cutBachButton->setChecked(true);
//                  break;
    case TimeSigType::CUT_TRIPLE:
//                  cutTripleButton->setChecked(true);
        break;
    }

    static const SymIdList prolatioList = {
        SymId::mensuralProlation1,  // tempus perfectum, prol. perfecta
        SymId::mensuralProlation2,  // tempus perfectum, prol. imperfecta
        SymId::mensuralProlation3,  // tempus perfectum, prol. imperfecta, dimin.
        SymId::mensuralProlation4,  // tempus perfectum, prol. perfecta, dimin.
        SymId::mensuralProlation5,  // tempus imperf. prol. perfecta
        SymId::mensuralProlation7,  // tempus imperf., prol. imperfecta, reversed
        SymId::mensuralProlation8,  // tempus imperf., prol. perfecta, dimin.
        SymId::mensuralProlation10, // tempus imperf., prol imperfecta, dimin., reversed
        SymId::mensuralProlation11, // tempus inperf., prol. perfecta, reversed
    };

    IEngravingFontPtr symbolFont = gpaletteScore->engravingFont();

    otherCombo->clear();
    otherCombo->setStyleSheet(QString("QComboBox { font-family: \"%1 Text\"; font-size: %2px; max-height: 30px } ")
                              .arg(QString::fromStdString(symbolFont->family())).arg(musicalFontSize));

    int idx = 0;
    for (SymId prolatio : prolatioList) {
        const QString& str = symbolFont->toString(prolatio);
        if (str.size() > 0) {
            otherCombo->addItem(str, int(prolatio));

            // if time sig matches this symbol string, set as selected
            if (m_editedTimeSig->timeSigType() == TimeSigType::NORMAL && m_editedTimeSig->denominatorString().isEmpty()
                && m_editedTimeSig->numeratorString() == str) {
                textButton->setChecked(false);
                otherButton->setChecked(true);
                otherCombo->setCurrentIndex(idx);

                // set the custom text fields to empty
                zText->setText(QString());
                nText->setText(QString());
            }
        }
        idx++;
    }

    Groups g = m_editedTimeSig->groups();
    if (g.empty()) {
        g = Groups::endings(m_editedTimeSig->sig()); // initialize with default
    }
    groups->setSig(m_editedTimeSig->sig(), g, m_editedTimeSig->numeratorString(), m_editedTimeSig->denominatorString());
}

TimeSignaturePropertiesDialog::~TimeSignaturePropertiesDialog()
{
    delete m_editedTimeSig;
}

void TimeSignaturePropertiesDialog::showEvent(QShowEvent* event)
{
    WidgetStateStore::restoreGeometry(this);
    QDialog::showEvent(event);
}

void TimeSignaturePropertiesDialog::hideEvent(QHideEvent* event)
{
    WidgetStateStore::saveGeometry(this);
    QDialog::hideEvent(event);
}

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void TimeSignaturePropertiesDialog::accept()
{
    auto notation = this->notation();
    if (!notation || !m_editedTimeSig || !m_originTimeSig) {
        return;
    }

    TimeSigType ts = TimeSigType::NORMAL;
    if (textButton->isChecked() || otherButton->isChecked()) {
        ts = TimeSigType::NORMAL;
    } else if (fourfourButton->isChecked()) {
        ts = TimeSigType::FOUR_FOUR;
    } else if (allaBreveButton->isChecked()) {
        ts = TimeSigType::ALLA_BREVE;
    }

    m_editedTimeSig->setProperty(mu::engraving::Pid::TIMESIG_TYPE, static_cast<int>(ts));

    if (zText->text() != m_editedTimeSig->numeratorString()) {
        m_editedTimeSig->setNumeratorString(zText->text());
    }
    if (nText->text() != m_editedTimeSig->denominatorString()) {
        m_editedTimeSig->setDenominatorString(nText->text());
    }

    if (otherButton->isChecked()) {
        IEngravingFontPtr symbolFont = m_editedTimeSig->score()->engravingFont();
        SymId symId = (SymId)(otherCombo->itemData(otherCombo->currentIndex()).toInt());
        // ...and set numerator to font string for symbol and denominator to empty string
        m_editedTimeSig->setNumeratorString(symbolFont->toString(symId));
        m_editedTimeSig->setDenominatorString(QString());
    }

    Groups g = groups->groups();
    m_editedTimeSig->setGroups(g);

    notation->undoStack()->prepareChanges(TranslatableString("undoableAction", "Edit time signature properties"));

    // Change linked mmr timesigs too
    for (EngravingObject* obj : m_originTimeSig->linkList()) {
        TimeSig* timeSig = toTimeSig(obj);
        if (timeSig == m_originTimeSig || (timeSig->track() == m_originTimeSig->track() && timeSig->score() == m_originTimeSig->score())) {
            timeSig->undoChangeProperty(Pid::TIMESIG_TYPE, int(m_editedTimeSig->timeSigType()));
            timeSig->undoChangeProperty(Pid::SHOW_COURTESY, m_editedTimeSig->showCourtesySig());
            timeSig->undoChangeProperty(Pid::NUMERATOR_STRING, m_editedTimeSig->numeratorString());
            timeSig->undoChangeProperty(Pid::DENOMINATOR_STRING, m_editedTimeSig->denominatorString());
            timeSig->undoChangeProperty(Pid::GROUP_NODES, g.nodes());
        }
    }

    if (m_editedTimeSig->sig() != m_originTimeSig->sig()) {
        notation->interaction()->addTimeSignature(m_originTimeSig->measure(), m_originTimeSig->staffIdx(), m_editedTimeSig);
    }

    m_originTimeSig->triggerLayoutAll();
    notation->undoStack()->commitChanges();
    notation->notationChanged().notify();

    QDialog::accept();
}

INotationPtr TimeSignaturePropertiesDialog::notation() const
{
    return globalContext()->currentNotation();
}
