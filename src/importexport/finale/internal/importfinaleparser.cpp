/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
#include "importfinaleparser.h"

#include "internal/importfinalelogger.h"
#include "internal/finaletypesconv.h"

#include <vector>
#include <exception>

#include "musx/musx.h"

#include "global/stringutils.h"
#include "types/string.h"

#include "engraving/dom/linkedobjects.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measurenumber.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/utils.h"

#include "modularity/ioc.h"
#include "importexport/finale/ifinaleconfiguration.h"

#include "log.h"

using namespace mu::engraving;
using namespace muse;
using namespace musx::dom;
using namespace mu::iex::finale;

static std::shared_ptr<mu::iex::finale::IFinaleConfiguration> configuration()
{
    return muse::modularity::globalIoc()->resolve<mu::iex::finale::IFinaleConfiguration>("iex_finale");
}

namespace mu::iex::finale {
FinaleParser::FinaleParser(engraving::Score* score, const std::shared_ptr<musx::dom::Document>& doc, MusxEmbeddedGraphicsMap&& graphics,
                           FinaleLoggerPtr& logger)
    : m_score(score), m_doc(doc), m_logger(logger), m_embeddedGraphics(std::move(graphics))
{
    const std::vector<IEngravingFontPtr> fonts = engravingFonts()->fonts();
    for (const IEngravingFontPtr& font : fonts) {
        m_engravingFonts.emplace(muse::strings::toLower(font->name()), font);
    }
    if (configuration()) {
        m_importPositionsType = configuration()->importPositionsType();
        m_convertTextSymbols = configuration()->convertTextSymbols();
    }
    m_finaleOptions.init(*this); // this must come after initializing m_engravingFonts
}

void FinaleParser::parse()
{
    // set score metadata
    muse::Date creationDate(m_doc->getHeader()->created.year, m_doc->getHeader()->created.month, m_doc->getHeader()->created.day);
    m_score->setMetaTag(u"creationDate", creationDate.toString(muse::DateFormat::ISODate));
    const MusxInstanceList<texts::FileInfoText> fileInfoTexts = m_doc->getTexts()->getArray<texts::FileInfoText>();
    for (const MusxInstance<texts::FileInfoText>& fileInfoText : fileInfoTexts) {
        String metaTag = metaTagFromFileInfo(fileInfoText->getTextType());
        std::string fileInfoValue = musx::util::EnigmaString::trimTags(fileInfoText->text);
        if (!metaTag.empty() && !fileInfoValue.empty()) {
            m_score->setMetaTag(metaTag, String::fromStdString(fileInfoValue));
        }
    }

    // styles (first, so that spatium and other defaults are correct)
    importStyles();

    // scoremap
    importParts();
    importBrackets();
    importMeasures();
    importStaffItems();
    importPageLayout();
    // Requires clef/keysig/timesig segments to have been created (layout call needed for non-change keysigs)
    // And number of staff lines at ticks to have been set (no layout necessary)
    if (importCustomPositions()) {
        m_score->doLayout();
        repositionMeasureNumbersBelow();
    }
    importBarlines();
    // Requires system layout
    // rebaseSystemLeftMargins(); will require instrument names to be repositioned

    // entries (notes, rests & tuplets)
    mapLayers();
    importEntries();
    // Layout score (needed for offset calculations)
    m_score->doLayout();
    importEntryAdjustments();
    importArticulations();

    // Smart shapes (spanners)
    // Require entries and entry layout
    importSmartShapes();

    // Text
    importPageTexts();
    // Layout score (needed for offset calculations)
    if (importCustomPositions()) {
        m_score->doLayout();
        importTextExpressions();
        applyStaffStyles(); // Requires all score elements have been created
    } else {
        importTextExpressions();
        applyStaffStyles();
        repositionMeasureNumbersBelow();
        m_score->doLayout();
    }
    rebasePageTextOffsets();

    // Collect styles for spanners (requires they have been laid out)
    auto smap = m_score->spannerMap().map();
    for (auto it = smap.cbegin(); it != smap.cend(); ++it) {
        collectElementStyle((*it).second);
    }

    // Apply collected element styles
    for (auto [sid, value] : m_elementStyles) {
        if (value.isValid()) {
            m_score->style().set(sid, value);
        }
    }

    // Setup system object staves
    logger()->logInfo(String(u"Initialising system object staves"));
    for (staff_idx_t staffIdx : m_systemObjectStaves) {
        m_score->addSystemObjectStaff(m_score->staff(staffIdx));
    }
    std::vector<EngravingItem*> systemObjects = collectSystemObjects(m_score, m_score->staves());
    for (EngravingItem* e : systemObjects) {
        // cross-reference links with sys obj staves and add invisible linked clones as needed
        std::set<staff_idx_t> unusedStaves = m_systemObjectStaves;
        if (e->links()) {
            for (EngravingObject* scoreElement : *e->links()) {
                muse::remove(systemObjects, toEngravingItem(scoreElement));
                muse::remove(unusedStaves, toEngravingItem(scoreElement)->staffIdx());
            }
        } else {
            muse::remove(unusedStaves, e->staffIdx());
        }
        for (staff_idx_t staffIdx : unusedStaves) {
            EngravingItem* copy = e->clone();
            copy->setStaffIdx(staffIdx);
            copy->setVisible(false);
            copy->linkTo(e);
            if (!e->isSpanner()) {
                copy->setParent(e->parentItem());
            }
            m_score->addElement(copy);
        }
    }
    logger()->logInfo(String(u"Import complete. Opening file..."));
}

staff_idx_t FinaleParser::staffIdxFromAssignment(StaffCmper assign)
{
    switch (assign) {
    case -1: return 0;
    case -2: return m_score->nstaves() - 1;
    default: return muse::value(m_inst2Staff, assign, muse::nidx);
    }
}

staff_idx_t FinaleParser::staffIdxForRepeats(bool onlyTop, Cmper staffList, Cmper measureId,
                                             std::vector<std::pair<staff_idx_t, StaffCmper> >& links)
{
    if (onlyTop) {
        return 0;
    }
    std::vector<StaffCmper> list;
    if (partScore()) {
        if (const auto& l = m_doc->getOthers()->get<others::StaffListRepeatParts>(m_currentMusxPartId, staffList)) {
            list = l->values;
        }
    } else {
        if (const auto& l = m_doc->getOthers()->get<others::StaffListRepeatScore>(m_currentMusxPartId, staffList)) {
            list = l->values;
        }
    }
    for (StaffCmper musxStaffId : list) {
        if (const auto& musxStaff = others::StaffComposite::createCurrent(m_doc, m_currentMusxPartId, musxStaffId, measureId, 0)) {
            if (musxStaff->hideRepeats) {
                muse::remove(list, musxStaffId);
            }
        }
    }
    if (partScore()) {
        if (const auto& l = m_doc->getOthers()->get<others::StaffListRepeatPartsForced>(m_currentMusxPartId, staffList)) {
            muse::join(list, l->values);
        }
    } else {
        if (const auto& l = m_doc->getOthers()->get<others::StaffListRepeatScoreForced>(m_currentMusxPartId, staffList)) {
            muse::join(list, l->values);
        }
    }

    for (StaffCmper musxStaffId : list) {
        staff_idx_t idx = staffIdxFromAssignment(musxStaffId);
        std::pair<staff_idx_t, StaffCmper> pair = std::make_pair(idx, musxStaffId);
        if (idx == muse::nidx || muse::contains(links, pair)) {
            continue;
        }
        links.emplace_back(pair);
    }
    return !links.empty() ? muse::takeFirst(links).first : muse::nidx;
}

void setAndStyleProperty(EngravingObject* e, Pid id, PropertyValue v, bool inheritStyle)
{
    if (v.isValid()) {
        e->setProperty(id, v);
    }
    if (e->propertyFlags(id) == PropertyFlags::NOSTYLE) {
        return;
    }
    const bool canLeaveStyled = inheritStyle && (e->getProperty(id) == e->propertyDefault(id));
    e->setPropertyFlags(id, canLeaveStyled ? PropertyFlags::STYLED : PropertyFlags::UNSTYLED);
}
}
