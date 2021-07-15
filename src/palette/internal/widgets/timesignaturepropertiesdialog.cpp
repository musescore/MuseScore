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

#include "libmscore/timesig.h"
#include "libmscore/mcursor.h"
#include "libmscore/durationtype.h"
#include "libmscore/score.h"
#include "libmscore/chord.h"
#include "libmscore/measure.h"
#include "libmscore/part.h"
#include "libmscore/scorefont.h"

#include "commonscene/exampleview.h"
#include "ui/view/musicalsymbolcodes.h"

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

    QFont musicalFont = QFont(QString::fromStdString(uiConfiguration()->musicalFontFamily()));

    fourfourButton->setFont(musicalFont);
    fourfourButton->setText(noteIconToString(MusicalSymbolCodes::Code::TIMESIG_COMMON));
    allaBreveButton->setFont(musicalFont);
    allaBreveButton->setText(noteIconToString(MusicalSymbolCodes::Code::TIMESIG_CUT));

    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    //! FIXME
    m_timesig = new TimeSig();

    zText->setText(m_timesig->numeratorString());
    nText->setText(m_timesig->denominatorString());
    // set validators for numerator and denominator strings
    // which only accept '+', '(', ')', digits and some time symb conventional representations
    QRegExp rx("[0-9+CO()\\x00A2\\x00D8\\x00BD\\x00BC]*");
    QValidator* validator = new QRegExpValidator(rx, this);
    zText->setValidator(validator);
    nText->setValidator(validator);

    Fraction nominal = m_timesig->sig() / m_timesig->stretch();
    nominal.reduce();
    zNominal->setValue(nominal.numerator());
    nNominal->setValue(nominal.denominator());
    Fraction sig(m_timesig->sig());
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
    switch (m_timesig->timeSigType()) {
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
    otherCombo->setFont(musicalFont);

    for (ProlatioTable pt : prolatioList) {
        const QString& str = scoreFont->toString(pt.id);
        if (str.size() > 0) {
            otherCombo->addItem(noteIconToString(pt.icon), int(pt.id));
            // if time sig matches this symbol string, set as selected

            if (m_timesig->timeSigType() == TimeSigType::NORMAL && m_timesig->denominatorString().isEmpty()
                && m_timesig->numeratorString() == str) {
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

    Groups g = m_timesig->groups();
    if (g.empty()) {
        g = Groups::endings(m_timesig->sig());         // initialize with default
    }
    groups->setSig(m_timesig->sig(), g, m_timesig->numeratorString(), m_timesig->denominatorString());
}

TimeSignaturePropertiesDialog::TimeSignaturePropertiesDialog(const TimeSignaturePropertiesDialog& other)
    : QDialog(other.parentWidget())
{
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
    m_timesig->setSig(actual, ts);
    m_timesig->setStretch(nominal / actual);

    if (zText->text() != m_timesig->numeratorString()) {
        m_timesig->setNumeratorString(zText->text());
    }
    if (nText->text() != m_timesig->denominatorString()) {
        m_timesig->setDenominatorString(nText->text());
    }

    if (otherButton->isChecked()) {
        ScoreFont* scoreFont = m_timesig->score()->scoreFont();
        SymId symId = (SymId)(otherCombo->itemData(otherCombo->currentIndex()).toInt());
        // ...and set numerator to font string for symbol and denominator to empty string
        m_timesig->setNumeratorString(scoreFont->toString(symId));
        m_timesig->setDenominatorString(QString());
    }

    Groups g = groups->groups();
    m_timesig->setGroups(g);
    QDialog::accept();
}
}
