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

#ifndef NO_ENGRAVING_INTERNAL
#include "engraving/infrastructure/internal/engravingconfiguration.h"
#include "engraving/infrastructure/internal/qfontprovider.h"
#include "engraving/infrastructure/internal/qimageprovider.h"
#endif

#include "engraving/style/defaultstyle.h"

#include "engraving/libmscore/mscore.h"
#include "engraving/libmscore/masterscore.h"
#include "engraving/libmscore/scorefont.h"

#include "engraving/accessibility/accessibleitem.h"

#include "compat/scoreaccess.h"

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
#ifndef NO_ENGRAVING_INTERNAL
    ioc()->registerExport<draw::IFontProvider>(moduleName(), new draw::QFontProvider());
    ioc()->registerExport<draw::IImageProvider>(moduleName(), new draw::QImageProvider());
    ioc()->registerExport<IEngravingConfiguration>(moduleName(), s_configuration);
#endif
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
    s_configuration->init();

    Ms::MScore::init(); // initialize libmscore

    DefaultStyle::instance()->init(s_configuration->defaultStyleFilePath(),
                                   s_configuration->partStyleFilePath());

    Ms::MScore::setNudgeStep(0.1); // cursor key (default 0.1)
    Ms::MScore::setNudgeStep10(1.0); // Ctrl + cursor key (default 1.0)
    Ms::MScore::setNudgeStep50(0.01); // Alt  + cursor key (default 0.01)

    AccessibleItem::enabled = false;
    Ms::gpaletteScore = compat::ScoreAccess::createMasterScore();
    AccessibleItem::enabled = true;
    if (Ms::EngravingObject::elementsProvider()) {
        Ms::EngravingObject::elementsProvider()->unreg(Ms::gpaletteScore);
    }

    Ms::gpaletteScore->setStyle(DefaultStyle::baseStyle());

    Ms::gpaletteScore->style().set(Ms::Sid::MusicalTextFont, QString("Leland Text"));
    Ms::ScoreFont* scoreFont = Ms::ScoreFont::fontByName("Leland");
    Ms::gpaletteScore->setScoreFont(scoreFont);
    Ms::gpaletteScore->setNoteHeadWidth(scoreFont->width(Ms::SymId::noteheadBlack, Ms::gpaletteScore->spatium()) / Ms::SPATIUM20);

    //! NOTE And some initialization in the `Notation::init()`
}

void EngravingModule::onDestroy()
{
    delete Ms::gpaletteScore;
    Ms::gpaletteScore = nullptr;
}
