/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include "dynamicpopupmodel.h"

#include "engraving/dom/dynamic.h"
#include "engraving/dom/factory.h"
#include "engraving/types/symnames.h"

#include "log.h"

using namespace mu::notation;
using namespace mu::engraving;

static const QList<QList<DynamicPopupModel::PageItem> > DYN_POPUP_PAGES = {
    {   // Page 1
        { DynamicType::PP,     30, 1.5, DynamicPopupModel::Dynamic },
        { DynamicType::P,      21, 1.0, DynamicPopupModel::Dynamic },
        { DynamicType::MP,     29, 0.5, DynamicPopupModel::Dynamic },
        { DynamicType::MF,     30, 0.5, DynamicPopupModel::Dynamic },
        { DynamicType::F,      23, 2.5, DynamicPopupModel::Dynamic },
        { DynamicType::FF,     30, 2.5, DynamicPopupModel::Dynamic },
    },
    {   // Page 2
        { DynamicType::FFF,    38, 2.5, DynamicPopupModel::Dynamic },
        { DynamicType::FFFF,   45, 2.5, DynamicPopupModel::Dynamic },
        { DynamicType::FFFFF,  53, 2.5, DynamicPopupModel::Dynamic },
    },
    {   // Page 3
        { DynamicType::FP,     30, 2.5, DynamicPopupModel::Dynamic },
        { DynamicType::PF,     32, 2.0, DynamicPopupModel::Dynamic },
        { DynamicType::SF,     25, 0.5, DynamicPopupModel::Dynamic },
        { DynamicType::SFZ,    29, 0.5, DynamicPopupModel::Dynamic },
        { DynamicType::SFF,    33, 0.5, DynamicPopupModel::Dynamic },
    },
    {   // Page 4
        { DynamicType::SFFZ,   37, 0.5, DynamicPopupModel::Dynamic },
        { DynamicType::SFP,    33, 0.5, DynamicPopupModel::Dynamic },
        { DynamicType::RFZ,    30, 0.5, DynamicPopupModel::Dynamic },
        { DynamicType::RF,     26, 0.5, DynamicPopupModel::Dynamic },
        { DynamicType::FZ,     26, 2.5, DynamicPopupModel::Dynamic },
    },
    {   // Page 5
        { DynamicType::PPPPPP, 74, 2.0, DynamicPopupModel::Dynamic },
        { DynamicType::FFFFFF, 60, 2.5, DynamicPopupModel::Dynamic },
    },
    {   // Page 6
        { DynamicType::PPPPP,  64, 2.0, DynamicPopupModel::Dynamic },
        { DynamicType::PPPP,   52, 2.0, DynamicPopupModel::Dynamic },
        { DynamicType::PPP,    44, 2.0, DynamicPopupModel::Dynamic },
    },
    {   // Page 7 - Hairpins
        { DynamicType::OTHER,  62, 0.0, DynamicPopupModel::Crescendo },
        { DynamicType::OTHER,  62, 0.0, DynamicPopupModel::Decrescendo },
    },
};

DynamicPopupModel::DynamicPopupModel(QObject* parent)
    : AbstractElementPopupModel(PopupModelType::TYPE_DYNAMIC, parent)
{
}

QString DynamicPopupModel::fontFamily() const
{
    IF_ASSERT_FAILED(m_item) {
        return QString();
    }

    return QString::fromStdString(m_item->score()->engravingFont()->family());
}

QVariantList DynamicPopupModel::pages() const
{
    return m_pages;
}

QString DynamicPopupModel::xmlTextToQString(const std::string& text, IEngravingFontPtr engravingFont) const
{
    const std::string startTag = "<sym>";
    const std::string endTag = "</sym>";

    String str;
    size_t pos = 0;

    while (true) {
        size_t startPos = text.find(startTag, pos);
        if (startPos == std::string::npos) {
            break;
        }
        size_t symNameStart = startPos + startTag.length();
        size_t endPos = text.find(endTag, symNameStart);
        if (endPos == std::string::npos) {
            break;
        }
        SymId id = SymNames::symIdByName(text.substr(symNameStart, endPos - symNameStart));
        str += engravingFont->toString(id);
        pos = endPos + endTag.length();
    }

    return str.toQString();
}

void DynamicPopupModel::init()
{
    AbstractElementPopupModel::init();

    m_pages.clear();

    if (!m_item) {
        return;
    }

    IEngravingFontPtr engravingFont = m_item->score()->engravingFont();

    for (const QList<DynamicPopupModel::PageItem>& page : DYN_POPUP_PAGES) {
        QVariantList variantPage;
        for (const DynamicPopupModel::PageItem& item : page) {
            QVariantMap variantMap {
                { "text", xmlTextToQString(Dynamic::dynamicText(item.dynType).toStdString(), engravingFont) },
                { "width", item.width },
                { "offset", item.offset },
                { "type", item.itemType }
            };
            variantPage.append(variantMap);
        }
        m_pages.append(QVariant::fromValue(variantPage));
    }

    emit pagesChanged();
}

void DynamicPopupModel::addOrChangeDynamic(int page, int index)
{
    IF_ASSERT_FAILED(m_item && m_item->isDynamic()) {
        return;
    }

    beginCommand(TranslatableString::untranslatable("Add dynamic"));
    m_item->undoChangeProperty(Pid::TEXT, Dynamic::dynamicText(DYN_POPUP_PAGES[page][index].dynType));
    m_item->undoChangeProperty(Pid::DYNAMIC_TYPE, DYN_POPUP_PAGES[page][index].dynType);
    endCommand();

    updateNotation();
}

void DynamicPopupModel::addHairpinToDynamic(ItemType itemType)
{
    IF_ASSERT_FAILED(m_item && m_item->isDynamic()) {
        return;
    }

    beginCommand(TranslatableString("undoableAction", "Add hairpin"));

    Hairpin* pin = Factory::createHairpin(m_item->score()->dummy()->segment());
    if (itemType == ItemType::Crescendo) {
        pin->setHairpinType(HairpinType::CRESC_HAIRPIN);
    } else if (itemType == ItemType::Decrescendo) {
        pin->setHairpinType(HairpinType::DECRESC_HAIRPIN);
    }

    m_item->score()->addHairpinToDynamic(pin, toDynamic(m_item));

    endCommand();
    updateNotation();
}
