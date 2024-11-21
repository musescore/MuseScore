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

#include <QDir>

#include "timedialog.h"

#include "translation.h"

#include "palettewidget.h"
#include "internal/palettecreator.h"

#include "engraving/dom/factory.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/timesig.h"
#include "engraving/compat/dummyelement.h"

using namespace mu::palette;
using namespace mu::engraving;

TimeDialog::TimeDialog(QWidget* parent)
    : QWidget(parent, Qt::WindowFlags(Qt::Dialog | Qt::Window))
{
    setupUi(this);
    setWindowTitle(muse::qtrc("palette", "Time signatures"));

    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QLayout* l = new QVBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);
    frame->setLayout(l);

    sp = new PaletteWidget(this);
    sp->setPalette(PaletteCreator::newTimePalette());
    sp->setReadOnly(false);
    sp->setSelectable(true);

    connect(zNominal, &QSpinBox::editingFinished, this, &TimeDialog::zChanged);
    connect(nNominal, &QComboBox::currentIndexChanged, this, &TimeDialog::nChanged);
    connect(sp, &PaletteWidget::boxClicked, this, &TimeDialog::paletteChanged);
    connect(sp, &PaletteWidget::changed, this, &TimeDialog::setDirty);
    connect(addButton, &QPushButton::clicked, this, &TimeDialog::addClicked);
    connect(zText, &QLineEdit::textChanged, this, &TimeDialog::textChanged);
    connect(nText, &QLineEdit::textChanged, this, &TimeDialog::textChanged);

    _timePalette = new PaletteScrollArea(sp);
    QSizePolicy policy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _timePalette->setSizePolicy(policy);
    _timePalette->setRestrictHeight(false);

    l->addWidget(_timePalette);

    _dirty = false;

    if (configuration()->useFactorySettings() || !sp->readFromFile(configuration()->timeSignaturesDirPath().toQString())) {
        Fraction sig(4, 4);
        groups->setSig(sig, Groups::endings(sig), zText->text(), nText->text());
    }
    for (int i = 0; i < sp->actualCellCount(); ++i) { // cells can be changed
        sp->setCellReadOnly(i, false);
    }

    ElementPtr el = sp->elementForCellAt(2);

    engravingRender()->layoutItem(el.get());

    sp->setSelected(2);
    paletteChanged(2);

    //! NOTE: It is necessary for the correct start of navigation in the dialog
    setFocus();
}

bool TimeDialog::dirty() const
{
    return _dirty;
}

bool TimeDialog::showTimePalette() const
{
    return _timePalette->isVisible();
}

//---------------------------------------------------------
//   addClicked
//---------------------------------------------------------

void TimeDialog::addClicked()
{
    auto ts = mu::engraving::Factory::makeTimeSig(gpaletteScore->dummy()->segment());
    ts->setSig(Fraction(zNominal->value(), denominator()));
    ts->setGroups(groups->groups());

    // check for special text
    if ((QString("%1").arg(zNominal->value()) != zText->text())
        || (QString("%1").arg(denominator()) != nText->text())) {
        ts->setNumeratorString(zText->text());
        ts->setDenominatorString(nText->text());
    }
    // extend palette:
    sp->appendElement(ts, "");
    sp->setSelected(sp->actualCellCount() - 1);
    _dirty = true;

    paletteProvider()->addCustomItemRequested().send(ts);
}

//---------------------------------------------------------
//   showTimePalette
//---------------------------------------------------------

void TimeDialog::setShowTimePalette(bool val)
{
    _timePalette->setVisible(val);
}

//---------------------------------------------------------
//   save
//---------------------------------------------------------

void TimeDialog::save()
{
    QDir dir;
    dir.mkpath(configuration()->timeSignaturesDirPath().toQString());
    sp->writeToFile(configuration()->timeSignaturesDirPath().toQString());
}

//---------------------------------------------------------
//   zChanged
//---------------------------------------------------------

void TimeDialog::zChanged()
{
    int numerator = zNominal->value();
    int denominator = this->denominator();

    Fraction sig(numerator, denominator);

    // Update beam groups view
    groups->setSig(sig, Groups::endings(sig), zText->text(), nText->text());
}

//---------------------------------------------------------
//   nChanged
//---------------------------------------------------------

void TimeDialog::nChanged(int val)
{
    Q_UNUSED(val);
    Fraction sig(zNominal->value(), denominator());
    groups->setSig(sig, Groups::endings(sig), zText->text(), nText->text());
}

//---------------------------------------------------------
//   denominator2Idx
//---------------------------------------------------------

int TimeDialog::denominator2Idx(int denominator) const
{
    int val = 4;
    switch (denominator) {
    case 1:  val = 0;
        break;
    case 2:  val = 1;
        break;
    case 4:  val = 2;
        break;
    case 8:  val = 3;
        break;
    case 16: val = 4;
        break;
    case 32: val = 5;
        break;
    case 64: val = 6;
        break;
    case 128: val = 7;
        break;
    }
    return val;
}

//---------------------------------------------------------
//   denominator
//---------------------------------------------------------

int TimeDialog::denominator() const
{
    int val = 4;
    switch (nNominal->currentIndex()) {
    case 0: val = 1;
        break;
    case 1: val = 2;
        break;
    case 2: val = 4;
        break;
    case 3: val = 8;
        break;
    case 4: val = 16;
        break;
    case 5: val = 32;
        break;
    case 6: val = 64;
        break;
    case 7: val = 128;
        break;
    }
    return val;
}

//---------------------------------------------------------
//   paletteChanged
//---------------------------------------------------------

void TimeDialog::paletteChanged(int idx)
{
    ElementPtr element = sp->elementForCellAt(idx);
    const std::shared_ptr<TimeSig> timeSig = std::dynamic_pointer_cast<TimeSig>(element);

    if (!timeSig || timeSig->type() != ElementType::TIMESIG) {
        zNominal->setEnabled(false);
        nNominal->setEnabled(false);
        zText->setEnabled(false);
        nText->setEnabled(false);
        groups->setEnabled(false);
        addButton->setEnabled(false);
        return;
    }

    zNominal->setEnabled(true);
    nNominal->setEnabled(true);
    zText->setEnabled(true);
    nText->setEnabled(true);
    groups->setEnabled(true);
    addButton->setEnabled(true);

    Fraction sig(timeSig->sig());
    Groups g = timeSig->groups();
    if (g.empty()) {
        g = Groups::endings(sig);
    }

    zNominal->setValue(sig.numerator());
    nNominal->setCurrentIndex(denominator2Idx(sig.denominator()));
    zText->setText(timeSig->numeratorString());
    nText->setText(timeSig->denominatorString());
    groups->setSig(sig, g, zText->text(), nText->text());
}

//---------------------------------------------------------
//   textChanged
//---------------------------------------------------------

void TimeDialog::textChanged()
{
    Fraction sig(zNominal->value(), denominator());
    groups->setSig(sig, Groups::endings(sig), zText->text(), nText->text());
}

void TimeDialog::setDirty()
{
    _dirty = true;
}
