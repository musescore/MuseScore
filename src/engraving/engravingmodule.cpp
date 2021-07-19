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
#include "engravingmodule.h"

#include "modularity/ioc.h"

#ifndef NO_ENGRAVING_QFONTENGINE
#include "engraving/draw/qfontprovider.h"
#endif
#include "engraving/draw/qimageconverter.h"

#include "engraving/style/defaultstyle.h"

#include "engraving/libmscore/mscore.h"
#include "engraving/libmscore/score.h"
#include "engraving/libmscore/scorefont.h"

#include "compat/scoreaccess.h"

#include "engraving/internal/engravingconfiguration.h"

using namespace mu::engraving;
using namespace mu::modularity;

static std::shared_ptr<EngravingConfiguration> s_configuration = std::make_shared<EngravingConfiguration>();

static void engraving_init_qrc()
{
    Q_INIT_RESOURCE(engraving);
}

std::string EngravingModule::moduleName() const
{
    return "engraving";
}

void EngravingModule::registerExports()
{
#ifndef NO_ENGRAVING_QFONTENGINE
    ioc()->registerExport<draw::IFontProvider>(moduleName(), new draw::QFontProvider());
#endif

    ioc()->registerExport<draw::IImageConverter>(moduleName(), new draw::QImageConverter());
    ioc()->registerExport<IEngravingConfiguration>(moduleName(), s_configuration);
}

void EngravingModule::resolveImports()
{
}

void EngravingModule::registerResources()
{
    engraving_init_qrc();
}

void EngravingModule::registerUiTypes()
{
    Ms::MScore::registerUiTypes();
}

void EngravingModule::onInit(const framework::IApplication::RunMode&)
{
    Ms::MScore::init(); // initialize libmscore

    DefaultStyle::instance()->init(s_configuration->defaultStyleFilePath(),
                                   s_configuration->partStyleFilePath());

    Ms::MScore::setNudgeStep(0.1); // cursor key (default 0.1)
    Ms::MScore::setNudgeStep10(1.0); // Ctrl + cursor key (default 1.0)
    Ms::MScore::setNudgeStep50(0.01); // Alt  + cursor key (default 0.01)

    Ms::gscore = compat::ScoreAccess::createMasterScore();
    Ms::gscore->setPaletteMode(true);
    Ms::gscore->setMovements(new Ms::Movements());
    Ms::gscore->setStyle(DefaultStyle::baseStyle());

    Ms::gscore->style().set(Ms::Sid::MusicalTextFont, QString("Leland Text"));
    Ms::ScoreFont* scoreFont = Ms::ScoreFont::fontByName("Leland");
    Ms::gscore->setScoreFont(scoreFont);
    Ms::gscore->setNoteHeadWidth(scoreFont->width(Ms::SymId::noteheadBlack, Ms::gscore->spatium()) / Ms::SPATIUM20);

    //! NOTE And some initialization in the `Notation::init()`
}
