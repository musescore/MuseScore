/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#include "propertiespanelabstractmodel.h"

#include "engraving/dom/barline.h"
#include "engraving/dom/dynamic.h"
#include "engraving/dom/property.h"
#include "engraving/dom/score.h"
#include "engraving/dom/tempotext.h"

#include "notation/inotationinteraction.h" // IWYU pragma: keep
#include "notation/inotationstyle.h"
#include "notation/inotationundostack.h"

#include "modularity/ioc.h"
#include "shortcuts/shortcutstypes.h"

#include "types/texttypes.h"

#include "log.h"

using namespace mu::propertiespanel;
using namespace mu::notation;
using namespace mu::engraving;

static const QMap<mu::engraving::ElementType, PropertiesPanelModelType> NOTATION_ELEMENT_MODEL_TYPES = {
    { mu::engraving::ElementType::NOTE, PropertiesPanelModelType::TYPE_NOTE },
    { mu::engraving::ElementType::STEM, PropertiesPanelModelType::TYPE_NOTE },
    { mu::engraving::ElementType::NOTEDOT, PropertiesPanelModelType::TYPE_NOTE },
    { mu::engraving::ElementType::NOTEHEAD, PropertiesPanelModelType::TYPE_NOTE },
    { mu::engraving::ElementType::NOTELINE, PropertiesPanelModelType::TYPE_NOTE },
    { mu::engraving::ElementType::SHADOW_NOTE, PropertiesPanelModelType::TYPE_NOTE },
    { mu::engraving::ElementType::HOOK, PropertiesPanelModelType::TYPE_NOTE },
    { mu::engraving::ElementType::BEAM, PropertiesPanelModelType::TYPE_NOTE },
    { mu::engraving::ElementType::GLISSANDO, PropertiesPanelModelType::TYPE_GLISSANDO },
    { mu::engraving::ElementType::GLISSANDO_SEGMENT, PropertiesPanelModelType::TYPE_GLISSANDO },
    { mu::engraving::ElementType::VIBRATO, PropertiesPanelModelType::TYPE_VIBRATO },
    { mu::engraving::ElementType::VIBRATO_SEGMENT, PropertiesPanelModelType::TYPE_VIBRATO },
    { mu::engraving::ElementType::SLUR, PropertiesPanelModelType::TYPE_SLUR },
    { mu::engraving::ElementType::SLUR_SEGMENT, PropertiesPanelModelType::TYPE_SLUR },
    { mu::engraving::ElementType::HAMMER_ON_PULL_OFF, PropertiesPanelModelType::TYPE_HAMMER_ON_PULL_OFF },
    { mu::engraving::ElementType::HAMMER_ON_PULL_OFF_SEGMENT, PropertiesPanelModelType::TYPE_HAMMER_ON_PULL_OFF },
    { mu::engraving::ElementType::TIE, PropertiesPanelModelType::TYPE_TIE },
    { mu::engraving::ElementType::TIE_SEGMENT, PropertiesPanelModelType::TYPE_TIE },
    { mu::engraving::ElementType::LAISSEZ_VIB, PropertiesPanelModelType::TYPE_LAISSEZ_VIB },
    { mu::engraving::ElementType::LAISSEZ_VIB_SEGMENT, PropertiesPanelModelType::TYPE_LAISSEZ_VIB },
    { mu::engraving::ElementType::PARTIAL_TIE, PropertiesPanelModelType::TYPE_PARTIAL_TIE },
    { mu::engraving::ElementType::PARTIAL_TIE_SEGMENT, PropertiesPanelModelType::TYPE_PARTIAL_TIE },
    { mu::engraving::ElementType::TEMPO_TEXT, PropertiesPanelModelType::TYPE_TEMPO },
    { mu::engraving::ElementType::FERMATA, PropertiesPanelModelType::TYPE_FERMATA },
    { mu::engraving::ElementType::LAYOUT_BREAK, PropertiesPanelModelType::TYPE_SECTIONBREAK },
    { mu::engraving::ElementType::BAR_LINE, PropertiesPanelModelType::TYPE_BARLINE },
    { mu::engraving::ElementType::PLAY_COUNT_TEXT, PropertiesPanelModelType::TYPE_PLAY_COUNT_TEXT },
    { mu::engraving::ElementType::MARKER, PropertiesPanelModelType::TYPE_MARKER },
    { mu::engraving::ElementType::JUMP, PropertiesPanelModelType::TYPE_JUMP },
    { mu::engraving::ElementType::KEYSIG, PropertiesPanelModelType::TYPE_KEYSIGNATURE },
    { mu::engraving::ElementType::ACCIDENTAL, PropertiesPanelModelType::TYPE_ACCIDENTAL },
    { mu::engraving::ElementType::FRET_DIAGRAM, PropertiesPanelModelType::TYPE_FRET_DIAGRAM },
    { mu::engraving::ElementType::PEDAL, PropertiesPanelModelType::TYPE_PEDAL },
    { mu::engraving::ElementType::PEDAL_SEGMENT, PropertiesPanelModelType::TYPE_PEDAL },
    { mu::engraving::ElementType::SPACER, PropertiesPanelModelType::TYPE_SPACER },
    { mu::engraving::ElementType::CLEF, PropertiesPanelModelType::TYPE_CLEF },
    { mu::engraving::ElementType::HAIRPIN, PropertiesPanelModelType::TYPE_HAIRPIN },
    { mu::engraving::ElementType::HAIRPIN_SEGMENT, PropertiesPanelModelType::TYPE_HAIRPIN },
    { mu::engraving::ElementType::OTTAVA, PropertiesPanelModelType::TYPE_OTTAVA },
    { mu::engraving::ElementType::OTTAVA_SEGMENT, PropertiesPanelModelType::TYPE_OTTAVA },
    { mu::engraving::ElementType::VOLTA, PropertiesPanelModelType::TYPE_VOLTA },
    { mu::engraving::ElementType::VOLTA_SEGMENT, PropertiesPanelModelType::TYPE_VOLTA },
    { mu::engraving::ElementType::STAFFTYPE_CHANGE, PropertiesPanelModelType::TYPE_STAFF_TYPE_CHANGES },
    { mu::engraving::ElementType::TBOX, PropertiesPanelModelType::TYPE_TEXT_FRAME },// text frame
    { mu::engraving::ElementType::VBOX, PropertiesPanelModelType::TYPE_VERTICAL_FRAME },// vertical frame
    { mu::engraving::ElementType::HBOX, PropertiesPanelModelType::TYPE_HORIZONTAL_FRAME },// horizontal frame
    { mu::engraving::ElementType::FBOX, PropertiesPanelModelType::TYPE_FRET_FRAME },// fret diagram legend
    { mu::engraving::ElementType::ARTICULATION, PropertiesPanelModelType::TYPE_ARTICULATION },
    { mu::engraving::ElementType::TAPPING,      PropertiesPanelModelType::TYPE_TAPPING },
    { mu::engraving::ElementType::TAPPING_HALF_SLUR_SEGMENT, PropertiesPanelModelType::TYPE_TAPPING },
    { mu::engraving::ElementType::ORNAMENT, PropertiesPanelModelType::TYPE_ORNAMENT },
    { mu::engraving::ElementType::TRILL, PropertiesPanelModelType::TYPE_ORNAMENT },
    { mu::engraving::ElementType::TRILL_SEGMENT, PropertiesPanelModelType::TYPE_ORNAMENT },
    { mu::engraving::ElementType::IMAGE, PropertiesPanelModelType::TYPE_IMAGE },
    { mu::engraving::ElementType::HARMONY, PropertiesPanelModelType::TYPE_CHORD_SYMBOL },
    { mu::engraving::ElementType::AMBITUS, PropertiesPanelModelType::TYPE_AMBITUS },
    { mu::engraving::ElementType::BRACKET, PropertiesPanelModelType::TYPE_BRACKET },
    { mu::engraving::ElementType::TIMESIG, PropertiesPanelModelType::TYPE_TIME_SIGNATURE },
    { mu::engraving::ElementType::MMREST, PropertiesPanelModelType::TYPE_MMREST },
    { mu::engraving::ElementType::BEND, PropertiesPanelModelType::TYPE_BEND },
    { mu::engraving::ElementType::GUITAR_BEND, PropertiesPanelModelType::TYPE_BEND },
    { mu::engraving::ElementType::GUITAR_BEND_SEGMENT, PropertiesPanelModelType::TYPE_BEND },
    { mu::engraving::ElementType::GUITAR_BEND_HOLD, PropertiesPanelModelType::TYPE_BEND },
    { mu::engraving::ElementType::GUITAR_BEND_HOLD_SEGMENT, PropertiesPanelModelType::TYPE_BEND },
    { mu::engraving::ElementType::TREMOLOBAR, PropertiesPanelModelType::TYPE_TREMOLOBAR },
    { mu::engraving::ElementType::TREMOLO_SINGLECHORD, PropertiesPanelModelType::TYPE_TREMOLO },
    { mu::engraving::ElementType::TREMOLO_TWOCHORD, PropertiesPanelModelType::TYPE_TREMOLO },
    { mu::engraving::ElementType::MEASURE_REPEAT, PropertiesPanelModelType::TYPE_MEASURE_REPEAT },
    { mu::engraving::ElementType::TUPLET, PropertiesPanelModelType::TYPE_TUPLET },
    { mu::engraving::ElementType::TEXTLINE, PropertiesPanelModelType::TYPE_TEXT_LINE },
    { mu::engraving::ElementType::TEXTLINE_SEGMENT, PropertiesPanelModelType::TYPE_TEXT_LINE },
    { mu::engraving::ElementType::PALM_MUTE, PropertiesPanelModelType::TYPE_TEXT_LINE },
    { mu::engraving::ElementType::PALM_MUTE_SEGMENT, PropertiesPanelModelType::TYPE_TEXT_LINE },
    { mu::engraving::ElementType::LET_RING, PropertiesPanelModelType::TYPE_TEXT_LINE },
    { mu::engraving::ElementType::LET_RING_SEGMENT, PropertiesPanelModelType::TYPE_TEXT_LINE },
    { mu::engraving::ElementType::WHAMMY_BAR, PropertiesPanelModelType::TYPE_TEXT_LINE },
    { mu::engraving::ElementType::WHAMMY_BAR_SEGMENT, PropertiesPanelModelType::TYPE_TEXT_LINE },
    { mu::engraving::ElementType::NOTELINE, PropertiesPanelModelType::TYPE_NOTELINE },
    { mu::engraving::ElementType::NOTELINE_SEGMENT, PropertiesPanelModelType::TYPE_NOTELINE },
    { mu::engraving::ElementType::GRADUAL_TEMPO_CHANGE, PropertiesPanelModelType::TYPE_GRADUAL_TEMPO_CHANGE },
    { mu::engraving::ElementType::GRADUAL_TEMPO_CHANGE_SEGMENT, PropertiesPanelModelType::TYPE_GRADUAL_TEMPO_CHANGE },
    { mu::engraving::ElementType::INSTRUMENT_NAME, PropertiesPanelModelType::TYPE_INSTRUMENT_NAME },
    { mu::engraving::ElementType::LYRICS, PropertiesPanelModelType::TYPE_LYRICS },
    { mu::engraving::ElementType::LYRICSLINE, PropertiesPanelModelType::TYPE_LYRICS_LINE },
    { mu::engraving::ElementType::LYRICSLINE_SEGMENT, PropertiesPanelModelType::TYPE_LYRICS_LINE },
    { mu::engraving::ElementType::PARTIAL_LYRICSLINE, PropertiesPanelModelType::TYPE_PARTIAL_LYRICS_LINE },
    { mu::engraving::ElementType::PARTIAL_LYRICSLINE_SEGMENT, PropertiesPanelModelType::TYPE_PARTIAL_LYRICS_LINE },
    { mu::engraving::ElementType::REST, PropertiesPanelModelType::TYPE_REST },
    { mu::engraving::ElementType::DYNAMIC, PropertiesPanelModelType::TYPE_DYNAMIC },
    { mu::engraving::ElementType::EXPRESSION, PropertiesPanelModelType::TYPE_EXPRESSION },
    { mu::engraving::ElementType::STRING_TUNINGS, PropertiesPanelModelType::TYPE_STRING_TUNINGS },
    { mu::engraving::ElementType::SYMBOL, PropertiesPanelModelType::TYPE_SYMBOL },
    { mu::engraving::ElementType::SYSTEM_DIVIDER, PropertiesPanelModelType::TYPE_SYMBOL },
    { mu::engraving::ElementType::CHORD_BRACKET, PropertiesPanelModelType::TYPE_CHORD_BRACKET },
};

static const QMap<mu::engraving::HairpinType, PropertiesPanelModelType> HAIRPIN_ELEMENT_MODEL_TYPES = {
    { mu::engraving::HairpinType::CRESC_HAIRPIN, PropertiesPanelModelType::TYPE_HAIRPIN },
    { mu::engraving::HairpinType::DIM_HAIRPIN, PropertiesPanelModelType::TYPE_HAIRPIN },
    { mu::engraving::HairpinType::CRESC_LINE, PropertiesPanelModelType::TYPE_CRESCENDO },
    { mu::engraving::HairpinType::DIM_LINE, PropertiesPanelModelType::TYPE_DIMINUENDO },
};

static const QMap<mu::engraving::LayoutBreakType, PropertiesPanelModelType> LAYOUT_BREAK_ELEMENT_MODEL_TYPES = {
    { mu::engraving::LayoutBreakType::SECTION, PropertiesPanelModelType::TYPE_SECTIONBREAK }
};

static const QMap<mu::engraving::TempoTextType, PropertiesPanelModelType> TEMPO_TEXT_ELEMENT_MODEL_TYPES = {
    { mu::engraving::TempoTextType::NORMAL, PropertiesPanelModelType::TYPE_TEMPO },
    { mu::engraving::TempoTextType::A_TEMPO, PropertiesPanelModelType::TYPE_A_TEMPO },
    { mu::engraving::TempoTextType::TEMPO_PRIMO, PropertiesPanelModelType::TYPE_TEMPO_PRIMO },
};

QString PropertiesPanelAbstractModel::shortcutsForActionCode(std::string code) const
{
    std::vector<std::string> shortcuts = shortcutsRegister()->shortcut(code).sequences;
    return muse::shortcuts::sequencesToNativeText(shortcuts);
}

PropertiesPanelAbstractModel::PropertiesPanelAbstractModel(QObject* parent, const muse::modularity::ContextPtr& iocCtx,
                                                           IElementRepositoryService* repository,
                                                           mu::engraving::ElementType elementType)
    : QObject(parent), muse::Contextable(iocCtx), m_repository(repository), m_elementType(elementType)
{
    if (!m_repository) {
        return;
    }

    m_repository->elementsUpdated().onReceive(this, [this](const QList<mu::engraving::EngravingItem*>&) {
        updateProperties();
    });

    connect(this, &PropertiesPanelAbstractModel::requestReloadPropertyItems, this, &PropertiesPanelAbstractModel::updateProperties);
}

void PropertiesPanelAbstractModel::init()
{
    onCurrentNotationChanged();
}

bool PropertiesPanelAbstractModel::isSystemObjectBelowBottomStaff() const
{
    return m_isSystemObjectBelowBottomStaff;
}

MeasurementUnits PropertiesPanelAbstractModel::measurementUnits() const
{
    return m_measurementUnits;
}

bool PropertiesPanelAbstractModel::shouldUpdateOnScoreChange() const
{
    return m_shouldUpdateOnScoreChange;
}

bool PropertiesPanelAbstractModel::shouldUpdateWhenEmpty() const
{
    return false;
}

bool PropertiesPanelAbstractModel::shouldUpdateOnEmptyPropertyAndStyleIdSets() const
{
    return false;
}

void PropertiesPanelAbstractModel::updateIsSystemObjectBelowBottomStaff()
{
    bool soBelowBottomStaff = false;
    for (EngravingItem* item : m_elementList) {
        if (item->isSystemObjectBelowBottomStaff()) {
            soBelowBottomStaff = true;
            break;
        }
    }

    if (m_isSystemObjectBelowBottomStaff != soBelowBottomStaff) {
        m_isSystemObjectBelowBottomStaff = soBelowBottomStaff;
        emit isSystemObjectBelowBottomStaffChanged(m_isSystemObjectBelowBottomStaff);
    }
}

void PropertiesPanelAbstractModel::updatemeasurementUnits()
{
    MeasurementUnits result = MeasurementUnits::UNITS_UNKNOWN;

    for (EngravingItem* item : m_elementList) {
        MeasurementUnits itemUnits = item->offsetIsSpatiumDependent() ? MeasurementUnits::UNITS_SPATIUM : MeasurementUnits::UNITS_MM;
        if (result != MeasurementUnits::UNITS_UNKNOWN && itemUnits != result) {
            result = MeasurementUnits::UNITS_UNKNOWN;
            break;
        } else {
            result = itemUnits;
        }
    }

    if (m_measurementUnits != result) {
        m_measurementUnits = result;
        emit measurementUnitsChanged(m_measurementUnits);
    }
}

QString PropertiesPanelAbstractModel::title() const
{
    return m_title;
}

int PropertiesPanelAbstractModel::icon() const
{
    return static_cast<int>(m_icon);
}

PropertiesPanelSectionType PropertiesPanelAbstractModel::sectionType() const
{
    return m_sectionType;
}

PropertiesPanelModelType PropertiesPanelAbstractModel::modelType() const
{
    return m_modelType;
}

ElementKey PropertiesPanelAbstractModel::makeKey(const EngravingItem* item)
{
    switch (item->type()) {
    case ElementType::TEMPO_TEXT: {
        const auto tempoText = toTempoText(item);
        return ElementKey{ ElementType::TEMPO_TEXT, static_cast<int>(tempoText->tempoTextType()) };
    }
    default:
        return ElementKey{ item->type(), item->subtype() };
    }
}

PropertiesPanelModelType PropertiesPanelAbstractModel::modelTypeByElementKey(const ElementKey& elementKey)
{
    if (elementKey.type == mu::engraving::ElementType::HAIRPIN || elementKey.type == mu::engraving::ElementType::HAIRPIN_SEGMENT) {
        return HAIRPIN_ELEMENT_MODEL_TYPES.value(static_cast<mu::engraving::HairpinType>(elementKey.subtype),
                                                 PropertiesPanelModelType::TYPE_UNDEFINED);
    }

    if (elementKey.type == mu::engraving::ElementType::LAYOUT_BREAK) {
        return LAYOUT_BREAK_ELEMENT_MODEL_TYPES.value(static_cast<mu::engraving::LayoutBreakType>(elementKey.subtype),
                                                      PropertiesPanelModelType::TYPE_UNDEFINED);
    }

    if (elementKey.type == mu::engraving::ElementType::TEMPO_TEXT) {
        return TEMPO_TEXT_ELEMENT_MODEL_TYPES.value(static_cast<mu::engraving::TempoTextType>(elementKey.subtype),
                                                    PropertiesPanelModelType::TYPE_UNDEFINED);
    }

    return NOTATION_ELEMENT_MODEL_TYPES.value(elementKey.type, PropertiesPanelModelType::TYPE_UNDEFINED);
}

PropertiesPanelModelTypeSet PropertiesPanelAbstractModel::modelTypesByElementKeys(const ElementKeySet& elementKeySet)
{
    PropertiesPanelModelTypeSet types;

    for (const ElementKey& key : elementKeySet) {
        types << modelTypeByElementKey(key);
    }

    return types;
}

static bool isPureDynamics(const QList<mu::engraving::EngravingItem*>& selectedElementList)
{
    if (selectedElementList.empty()) {
        return false;
    }

    for (const EngravingItem* item : selectedElementList) {
        if (!item->isTextBase()) {
            continue;
        }

        if (!item->isDynamic()) {
            return false;
        }

        const Dynamic* dynamic = toDynamic(item);
        if (dynamic->hasCustomText()) {
            return false;
        }
    }

    return true;
}

static bool barlineWithPlayText(const QList<mu::engraving::EngravingItem*>& selectedElementList)
{
    if (selectedElementList.empty()) {
        return false;
    }

    for (const EngravingItem* item : selectedElementList) {
        if (!item->isBarLine()) {
            continue;
        }

        Segment* seg = toBarLine(item)->segment();
        PlayCountText* playCountText = toPlayCountText(seg->findAnnotation(ElementType::PLAY_COUNT_TEXT, 0, 0));

        if (playCountText) {
            return true;
        }
    }

    return false;
}

static bool hasValidTextLineBaseSegment(const QList<mu::engraving::EngravingItem*>& selectedElementList)
{
    if (selectedElementList.empty()) {
        return false;
    }

    for (const EngravingItem* item : selectedElementList) {
        if (!item->isTextLineBaseSegment()) {
            continue;
        }
        const TextLineBaseSegment* tlbs = toTextLineBaseSegment(item);
        const TextLineBase* tlb = tlbs ? tlbs->textLineBase() : nullptr;
        if (!tlb) {
            continue;
        }
        if (!tlb->beginText().empty() || !tlb->continueText().empty() || !tlb->endText().empty()) {
            return true;
        }
    }

    return false;
}

PropertiesPanelSectionTypeSet PropertiesPanelAbstractModel::sectionTypesByElementKeys(const ElementKeySet& elementKeySet, bool isRange,
                                                                                      const QList<mu::engraving::EngravingItem*>&
                                                                                      selectedElementList)
{
    PropertiesPanelSectionTypeSet types;

    for (const ElementKey& key : elementKeySet) {
        if (NOTATION_ELEMENT_MODEL_TYPES.contains(key.type)
            && (modelTypeByElementKey(key) != PropertiesPanelModelType::TYPE_UNDEFINED)) {
            types << PropertiesPanelSectionType::SECTION_NOTATION;
        }

        // Don't show the "Text" propertiespanel panel for "pure" dynamics (i.e. without custom text)
        if ((TEXT_ELEMENT_TYPES.contains(key.type) && !isPureDynamics(selectedElementList)) || barlineWithPlayText(selectedElementList)) {
            types << PropertiesPanelSectionType::SECTION_TEXT;
        }

        // Look for a TextLineBaseSegment with begin, continue, or end text...
        if (hasValidTextLineBaseSegment(selectedElementList)) {
            types << PropertiesPanelSectionType::SECTION_TEXT;
        }

        if (key.type != mu::engraving::ElementType::INSTRUMENT_NAME) {
            types << PropertiesPanelSectionType::SECTION_GENERAL;
        }
    }

    if (isRange) {
        types << PropertiesPanelSectionType::SECTION_MEASURES;
        types << PropertiesPanelSectionType::SECTION_EMPTY_STAVES;
    }

    if (showPartsSection(selectedElementList)) {
        types << PropertiesPanelSectionType::SECTION_PARTS;
    }

    return types;
}

bool PropertiesPanelAbstractModel::showPartsSection(const QList<EngravingItem*>& selectedElementList)
{
    static const std::unordered_set<ElementType> noAvailableChangePartsSettingsTypes {
        ElementType::LAYOUT_BREAK,
        ElementType::ACCIDENTAL,
        ElementType::SOUND_FLAG
    };

    for (EngravingItem* element : selectedElementList) {
        if ((!element->score()->isMaster() && !muse::contains(noAvailableChangePartsSettingsTypes, element->type()))
            || element->canBeExcludedFromOtherParts()) {
            return true;
        }
    }

    return false;
}

bool PropertiesPanelAbstractModel::isEmpty() const
{
    return m_elementList.isEmpty();
}

void PropertiesPanelAbstractModel::setTitle(QString title)
{
    if (m_title == title) {
        return;
    }

    m_title = title;
    emit titleChanged();
}

void PropertiesPanelAbstractModel::setIcon(muse::ui::IconCode::Code icon)
{
    m_icon = icon;
}

void PropertiesPanelAbstractModel::setSectionType(PropertiesPanelSectionType sectionType)
{
    m_sectionType = sectionType;
}

void PropertiesPanelAbstractModel::setModelType(PropertiesPanelModelType modelType)
{
    m_modelType = modelType;
}

void PropertiesPanelAbstractModel::onPropertyValueChanged(const mu::engraving::Pid pid, const QVariant& newValue)
{
    setPropertyValue(m_elementList, pid, newValue);
    loadProperties();
}

void PropertiesPanelAbstractModel::setPropertyValue(const QList<engraving::EngravingItem*>& items, const mu::engraving::Pid pid,
                                                    const QVariant& newValue)
{
    if (items.empty()) {
        return;
    }

    beginCommand(TranslatableString("undoableAction", "Edit %1").arg(propertyUserName(pid)));

    for (mu::engraving::EngravingItem* item : items) {
        IF_ASSERT_FAILED(item) {
            continue;
        }

        mu::engraving::PropertyFlags ps = item->propertyFlags(pid);

        if (ps == mu::engraving::PropertyFlags::STYLED) {
            ps = mu::engraving::PropertyFlags::UNSTYLED;
        }

        PropertyValue propValue = valueToElementUnits(pid, newValue, item);
        item->undoChangeProperty(pid, propValue, ps);
    }

    updateNotation();
    endCommand();
}

void PropertiesPanelAbstractModel::onPropertyValueReset(const mu::engraving::Pid pid)
{
    resetPropertyValue(m_elementList, pid);
    loadProperties();
}

void PropertiesPanelAbstractModel::resetPropertyValue(const QList<engraving::EngravingItem*>& items, const mu::engraving::Pid pid)
{
    if (items.empty()) {
        return;
    }

    beginCommand(TranslatableString("undoableAction", "Reset %1").arg(propertyUserName(pid)));

    for (mu::engraving::EngravingItem* item : items) {
        IF_ASSERT_FAILED(item) {
            continue;
        }

        item->undoResetProperty(pid);
    }

    updateNotation();
    endCommand();
}

void PropertiesPanelAbstractModel::updateProperties()
{
    requestElements();

    emit isEmptyChanged();

    if (!isEmpty()) {
        loadProperties();
    }
}

void PropertiesPanelAbstractModel::requestElements()
{
    if (m_elementType != mu::engraving::ElementType::INVALID) {
        m_elementList = m_repository->findElementsByType(m_elementType);
    }
}

void PropertiesPanelAbstractModel::onCurrentNotationChanged()
{
}

void PropertiesPanelAbstractModel::onNotationChanged(const mu::engraving::PropertyIdSet&, const mu::engraving::StyleIdSet&)
{
}

mu::engraving::Sid PropertiesPanelAbstractModel::styleIdByPropertyId(const mu::engraving::Pid pid) const
{
    mu::engraving::Sid result = mu::engraving::Sid::NOSTYLE;

    for (const mu::engraving::EngravingItem* element : m_elementList) {
        result = element->getPropertyStyle(pid);

        if (result != mu::engraving::Sid::NOSTYLE) {
            break;
        }
    }

    return result;
}

mu::engraving::PropertyIdSet PropertiesPanelAbstractModel::propertyIdSetFromStyleIdSet(const StyleIdSet& styleIdSet) const
{
    if (styleIdSet.empty()) {
        return PropertyIdSet();
    }

    PropertyIdSet result;

    for (const mu::engraving::EngravingItem* element : m_elementList) {
        const mu::engraving::ElementStyle* style = element->styledProperties();
        if (!style) {
            continue;
        }

        for (const StyledProperty& property : *style) {
            if (muse::contains(styleIdSet, property.sid)) {
                result.insert(property.pid);
            }
        }
    }

    return result;
}

bool PropertiesPanelAbstractModel::updateStyleValue(const mu::engraving::Sid& sid, const QVariant& newValue)
{
    PropertyValue newVal = PropertyValue::fromQVariant(newValue, mu::engraving::MStyle::valueType(sid));
    if (style() && style()->styleValue(sid) != newVal) {
        beginCommand(TranslatableString("undoableAction", "Edit style"));
        style()->setStyleValue(sid, newVal);
        endCommand();
        return true;
    }

    return false;
}

QVariant PropertiesPanelAbstractModel::styleValue(const mu::engraving::Sid& sid) const
{
    return style() ? style()->styleValue(sid).toQVariant() : QVariant();
}

PropertyValue PropertiesPanelAbstractModel::valueToElementUnits(const mu::engraving::Pid& pid, const QVariant& value,
                                                                const mu::engraving::EngravingItem* element) const
{
    if (mu::engraving::Pid::VERSE == pid) {
        return value.toInt() - 1;
    }

    auto toPoint = [](const QVariant& v) {
        return PointF::fromQPointF(v.value<QPointF>());
    };

    P_TYPE type = mu::engraving::propertyType(pid);
    switch (type) {
    case P_TYPE::POINT: {
        if (pid == Pid::OFFSET ? element->offsetIsSpatiumDependent() : element->sizeIsSpatiumDependent()) {
            return toPoint(value) * element->spatium();
        } else {
            return toPoint(value) * mu::engraving::DPMM;
        }
    }

    case P_TYPE::ABSOLUTE:
        return element->absoluteFromSpatium(Spatium(value.toReal()));

    case P_TYPE::SPATIUM:
        return Spatium(value.toReal());

    case P_TYPE::TEMPO:
        return BeatsPerSecond::fromBPM(BeatsPerMinute(value.toReal()));

    case P_TYPE::INT_VEC: {
        bool ok = true;
        std::vector<int> res;

        for (const QString& str : value.toString().split(',', Qt::SkipEmptyParts)) {
            if (int i = str.simplified().toInt(&ok); ok) {
                res.push_back(i);
            }
        }

        return res;
    } break;

    case P_TYPE::COLOR:
        return Color::fromQColor(value.value<QColor>());

    default:
        return PropertyValue::fromQVariant(value, type);
    }
}

QVariant PropertiesPanelAbstractModel::valueFromElementUnits(const mu::engraving::Pid& pid, const PropertyValue& value,
                                                             const mu::engraving::EngravingItem* element) const
{
    if (mu::engraving::Pid::VERSE == pid) {
        return value.toInt() + 1;
    }

    switch (value.type()) {
    case P_TYPE::POINT: {
        if (pid == Pid::OFFSET ? element->offsetIsSpatiumDependent() : element->sizeIsSpatiumDependent()) {
            return value.value<PointF>().toQPointF() / element->spatium();
        } else {
            return value.value<PointF>().toQPointF() / mu::engraving::DPMM;
        }
    }

    case P_TYPE::ABSOLUTE:
        return Spatium::fromAbsolute(value.toReal(), element->spatium()).val();

    case P_TYPE::SPATIUM:
        return value.value<Spatium>().val();

    case P_TYPE::TEMPO:
        return value.value<BeatsPerSecond>().toBPM().val;

    case P_TYPE::DIRECTION_V:
        return static_cast<int>(value.value<mu::engraving::DirectionV>());

    case P_TYPE::INT_VEC: {
        QStringList strList;

        for (const int i : value.value<std::vector<int> >()) {
            strList << QString::number(i);
        }

        return strList.join(",");
    }
    case P_TYPE::COLOR:
        return value.value<muse::draw::Color>().toQColor();
    default:
        return value.toQVariant();
    }
}

void PropertiesPanelAbstractModel::setElementType(mu::engraving::ElementType type)
{
    m_elementType = type;
}

PropertyItem* PropertiesPanelAbstractModel::buildPropertyItem(const mu::engraving::Pid& propertyId,
                                                              std::function<void(const mu::engraving::Pid propertyId,
                                                                                 const QVariant& newValue)> onPropertyChangedCallBack,
                                                              std::function<void(const mu::engraving::Sid styleId,
                                                                                 const QVariant& newValue)> onStyleChangedCallBack,
                                                              std::function<void(const mu::engraving::Pid propertyId)>
                                                              onPropertyResetCallBack)
{
    PropertyItem* newPropertyItem = new PropertyItem(propertyId, this);

    initPropertyItem(newPropertyItem, onPropertyChangedCallBack, onStyleChangedCallBack, onPropertyResetCallBack);

    return newPropertyItem;
}

PointFPropertyItem* PropertiesPanelAbstractModel::buildPointFPropertyItem(const mu::engraving::Pid& propertyId,
                                                                          std::function<void(const mu::engraving::Pid propertyId,
                                                                                             const QVariant& newValue)>
                                                                          onPropertyChangedCallBack,
                                                                          std::function<void(const mu::engraving::Pid propertyId)>
                                                                          onPropertyResetCallBack)
{
    PointFPropertyItem* newPropertyItem = new PointFPropertyItem(propertyId, this);

    initPropertyItem(newPropertyItem, onPropertyChangedCallBack, nullptr, onPropertyResetCallBack);

    return newPropertyItem;
}

void PropertiesPanelAbstractModel::initPropertyItem(PropertyItem* propertyItem,
                                                    std::function<void(const mu::engraving::Pid propertyId,
                                                                       const QVariant& newValue)> onPropertyChangedCallBack,
                                                    std::function<void(const mu::engraving::Sid styleId,
                                                                       const QVariant& newValue)> onStyleChangedCallBack,
                                                    std::function<void(const mu::engraving::Pid propertyId)> onPropertyResetCallBack)
{
    auto propertyCallback = onPropertyChangedCallBack;
    if (!propertyCallback) {
        propertyCallback = [this](const mu::engraving::Pid propertyId, const QVariant& newValue) {
            onPropertyValueChanged(propertyId, newValue);
        };
    }

    auto styleCallback = onStyleChangedCallBack;
    if (!styleCallback) {
        styleCallback = [this](const mu::engraving::Sid styleId, const QVariant& newValue) {
            updateStyleValue(styleId, newValue);

            emit requestReloadPropertyItems();
        };
    }

    auto resetCallback = onPropertyResetCallBack;
    if (!resetCallback) {
        resetCallback = [this](const mu::engraving::Pid propertyId) {
            onPropertyValueReset(propertyId);
        };
    }

    connect(propertyItem, &PropertyItem::propertyModified, this, propertyCallback);
    connect(propertyItem, &PropertyItem::resetToDefaultRequested, this, resetCallback);
    connect(propertyItem, &PropertyItem::applyToStyleRequested, this, styleCallback);
}

void PropertiesPanelAbstractModel::loadPropertyItem(PropertyItem* propertyItem, ConvertPropertyValueFunc convertElementPropertyValueFunc)
{
    loadPropertyItem(propertyItem, m_elementList, convertElementPropertyValueFunc);
}

void PropertiesPanelAbstractModel::loadPropertyItem(PropertyItem* propertyItem, const QList<EngravingItem*>& elements,
                                                    ConvertPropertyValueFunc convertElementPropertyValueFunc)
{
    if (!propertyItem) {
        return;
    }

    if (elements.isEmpty()) {
        propertyItem->setIsEnabled(false);
        return;
    }

    mu::engraving::Pid pid = propertyItem->propertyId();

    mu::engraving::Sid styleId = styleIdByPropertyId(pid);
    propertyItem->setStyleId(styleId);

    QVariant propertyValue;

    bool isUndefined = false;
    bool isModified = false;

    for (const mu::engraving::EngravingItem* element : elements) {
        IF_ASSERT_FAILED(element) {
            continue;
        }

        QVariant elementCurrentValue = valueFromElementUnits(pid, element->getProperty(pid), element);
        QVariant elementDefaultValue = valueFromElementUnits(pid, element->propertyDefault(pid), element);

        bool isPropertySupportedByElement = elementCurrentValue.isValid();

        if (!isPropertySupportedByElement) {
            continue;
        }

        if (convertElementPropertyValueFunc) {
            elementCurrentValue = convertElementPropertyValueFunc(elementCurrentValue);
            elementDefaultValue = convertElementPropertyValueFunc(elementDefaultValue);
        }

        if (!propertyValue.isValid()) {
            propertyValue = elementCurrentValue;
        }

        if (!isUndefined && propertyValue != elementCurrentValue) {
            isUndefined = true;
        }

        if (!isModified) {
            PropertyFlags f = element->propertyFlags(pid);
            if (f == PropertyFlags::UNSTYLED || elementCurrentValue != elementDefaultValue) {
                isModified = true;
            }
        }

        if (isUndefined && isModified) {
            break;
        }
    }

    //@note Some elements may support the property, some don't. If element doesn't support property it'll return invalid value.
    //      So we use that knowledge here
    propertyItem->setIsEnabled(propertyValue.isValid());

    if (isUndefined) {
        propertyValue = QVariant();
    }

    propertyItem->updateCurrentValue(propertyValue);
    propertyItem->setIsModified(isModified);
}

bool PropertiesPanelAbstractModel::isNotationExisting() const
{
    return context()->currentProject() != nullptr;
}

INotationStylePtr PropertiesPanelAbstractModel::style() const
{
    return currentNotation() ? currentNotation()->style() : nullptr;
}

INotationUndoStackPtr PropertiesPanelAbstractModel::undoStack() const
{
    return currentNotation() ? currentNotation()->undoStack() : nullptr;
}

void PropertiesPanelAbstractModel::beginCommand(const muse::TranslatableString& actionName)
{
    if (undoStack()) {
        undoStack()->prepareChanges(actionName);
    }

    //! NOTE prevents unnecessary updating of properties
    //! after changing their values in the propertiespanel
    m_shouldUpdateOnScoreChange = false;
}

void PropertiesPanelAbstractModel::endCommand()
{
    if (undoStack()) {
        undoStack()->commitChanges();
    }

    m_shouldUpdateOnScoreChange = true;
}

INotationPtr PropertiesPanelAbstractModel::currentNotation() const
{
    return context()->currentNotation();
}

bool PropertiesPanelAbstractModel::isMasterNotation() const
{
    INotationPtr notation = currentNotation();
    if (!notation) {
        return false;
    }

    return notation == context()->currentMasterNotation()->notation();
}

INotationSelectionPtr PropertiesPanelAbstractModel::selection() const
{
    INotationPtr notation = currentNotation();
    return notation ? notation->interaction()->selection() : nullptr;
}

void PropertiesPanelAbstractModel::updateNotation()
{
    if (!currentNotation()) {
        return;
    }

    currentNotation()->notationChanged().send(muse::RectF());
}
