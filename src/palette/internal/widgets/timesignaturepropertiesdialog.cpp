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

#include "timesignaturepropertiesdialog.h"

#include "engraving/libmscore/timesig.h"
#include "engraving/libmscore/mcursor.h"
#include "engraving/libmscore/durationtype.h"
#include "engraving/libmscore/score.h"
#include "engraving/libmscore/chord.h"
#include "engraving/libmscore/measure.h"
#include "engraving/libmscore/part.h"
#include "engraving/libmscore/scorefont.h"

#include "commonscene/exampleview.h"
#include "ui/view/musicalsymbolcodes.h"
#include "global/widgetstatestore.h"

static QString TIME_SIGNATURE_PROPERTIES_DIALOG_NAME("TimeSignaturePropertiesDialog");

using namespace mu::ui;
using namespace mu::notation;

namespace Ms {
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

    QString radioButtonStyle = QString("QRadioButton { font-family: %1; font-size: %2pt }")
                               .arg(musicalFontFamily)
                               .arg(musicalFontSize);

    fourfourButton->setStyleSheet(radioButtonStyle);
    fourfourButton->setText(musicalSymbolToString(MusicalSymbolCodes::Code::TIMESIG_COMMON));
    fourfourButton->setMaximumHeight(30);
    allaBreveButton->setStyleSheet(radioButtonStyle);
    allaBreveButton->setText(musicalSymbolToString(MusicalSymbolCodes::Code::TIMESIG_CUT));
    allaBreveButton->setMaximumHeight(30);

    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    const INotationInteractionPtr interaction = notation() ? notation()->interaction() : nullptr;
    const INotationSelectionPtr selection = interaction ? interaction->selection() : nullptr;
    Element* selectedElement = selection ? selection->element() : nullptr;
    m_originTimeSig = selectedElement ? toTimeSig(selectedElement) : nullptr;

    if (!m_originTimeSig) {
        return;
    }

    m_editedTimeSig = m_originTimeSig->clone();

    zText->setText(m_editedTimeSig->numeratorString());
    nText->setText(m_editedTimeSig->denominatorString());
    // set validators for numerator and denominator strings
    // which only accept '+', '(', ')', digits and some time symb conventional representations
    QRegExp rx("[0-9+CO()\\x00A2\\x00D8\\x00BD\\x00BC]*");
    QValidator* validator = new QRegExpValidator(rx, this);
    zText->setValidator(validator);
    nText->setValidator(validator);

    Fraction nominal = m_editedTimeSig->sig() / m_editedTimeSig->stretch();
    nominal.reduce();
    zNominal->setValue(nominal.numerator());
    nNominal->setValue(nominal.denominator());
    Fraction sig(m_editedTimeSig->sig());
    zActual->setValue(sig.numerator());
    nActual->setValue(sig.denominator());
    zNominal->setEnabled(false);
    nNominal->setEnabled(false);

    // TODO: fix https://musescore.org/en/node/42341
    // for now, editing of actual (local) time sig is disabled in dialog
    // but more importantly, the dialog should make it clear that this is "local" change only
    // and not normally the right way to add 7/4 to a score
    zActual->setEnabled(false);
    nActual->setEnabled(false);
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

    // set ID's of other symbols
    struct ProlatioTable {
        SymId id;
        MusicalSymbolCodes::Code icon;
    };

    //! FIXME
    static const std::vector<ProlatioTable> prolatioList = {
//        { SymId::mensuralProlation1,  Icon::timesig_prolatio01_ICON }, // tempus perfectum, prol. perfecta
//        { SymId::mensuralProlation2,  Icon::timesig_prolatio02_ICON }, // tempus perfectum, prol. imperfecta
//        { SymId::mensuralProlation3,  Icon::timesig_prolatio03_ICON }, // tempus perfectum, prol. imperfecta, dimin.
//        { SymId::mensuralProlation4,  Icon::timesig_prolatio04_ICON }, // tempus perfectum, prol. perfecta, dimin.
//        { SymId::mensuralProlation5,  Icon::timesig_prolatio05_ICON }, // tempus imperf. prol. perfecta
//        { SymId::mensuralProlation7,  Icon::timesig_prolatio07_ICON }, // tempus imperf., prol. imperfecta, reversed
//        { SymId::mensuralProlation8,  Icon::timesig_prolatio08_ICON }, // tempus imperf., prol. perfecta, dimin.
//        { SymId::mensuralProlation10, Icon::timesig_prolatio10_ICON }, // tempus imperf., prol imperfecta, dimin., reversed
//        { SymId::mensuralProlation11, Icon::timesig_prolatio11_ICON }, // tempus inperf., prol. perfecta, reversed
    };

    ScoreFont* scoreFont = gscore->scoreFont();
    int idx = 0;

    otherCombo->clear();

    for (ProlatioTable pt : prolatioList) {
        const QString& str = scoreFont->toString(pt.id);
        if (str.size() > 0) {
            otherCombo->addItem(musicalSymbolToString(pt.icon), int(pt.id));
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
        g = Groups::endings(m_editedTimeSig->sig());         // initialize with default
    }
    groups->setSig(m_editedTimeSig->sig(), g, m_editedTimeSig->numeratorString(), m_editedTimeSig->denominatorString());

    WidgetStateStore::restoreGeometry(this);
}

TimeSignaturePropertiesDialog::TimeSignaturePropertiesDialog(const TimeSignaturePropertiesDialog& other)
    : QDialog(other.parentWidget())
{
}

TimeSignaturePropertiesDialog::~TimeSignaturePropertiesDialog()
{
    delete m_editedTimeSig;
}

void TimeSignaturePropertiesDialog::hideEvent(QHideEvent* event)
{
    WidgetStateStore::saveGeometry(this);
    QDialog::hideEvent(event);
}

int TimeSignaturePropertiesDialog::static_metaTypeId()
{
    return QMetaType::type(TIME_SIGNATURE_PROPERTIES_DIALOG_NAME.toStdString().c_str());
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

    Fraction actual(zActual->value(), nActual->value());
    Fraction nominal(zNominal->value(), nNominal->value());
    m_editedTimeSig->setSig(actual, ts);
    m_editedTimeSig->setStretch(nominal / actual);

    if (zText->text() != m_editedTimeSig->numeratorString()) {
        m_editedTimeSig->setNumeratorString(zText->text());
    }
    if (nText->text() != m_editedTimeSig->denominatorString()) {
        m_editedTimeSig->setDenominatorString(nText->text());
    }

    if (otherButton->isChecked()) {
        ScoreFont* scoreFont = m_editedTimeSig->score()->scoreFont();
        SymId symId = (SymId)(otherCombo->itemData(otherCombo->currentIndex()).toInt());
        // ...and set numerator to font string for symbol and denominator to empty string
        m_editedTimeSig->setNumeratorString(scoreFont->toString(symId));
        m_editedTimeSig->setDenominatorString(QString());
    }

    Groups g = groups->groups();
    m_editedTimeSig->setGroups(g);

    notation->undoStack()->prepareChanges();

    m_originTimeSig->undoChangeProperty(Pid::TIMESIG_TYPE, int(m_editedTimeSig->timeSigType()));
    m_originTimeSig->undoChangeProperty(Pid::SHOW_COURTESY, m_editedTimeSig->showCourtesySig());
    m_originTimeSig->undoChangeProperty(Pid::NUMERATOR_STRING, m_editedTimeSig->numeratorString());
    m_originTimeSig->undoChangeProperty(Pid::DENOMINATOR_STRING, m_editedTimeSig->denominatorString());
    m_originTimeSig->undoChangeProperty(Pid::GROUPS, QVariant::fromValue<Groups>(g));

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
}
