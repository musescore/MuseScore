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
#include "abstractinspectormodel.h"
#include "dom/barline.h"
#include "engraving/dom/dynamic.h"
#include "engraving/dom/property.h"

#include "shortcuts/shortcutstypes.h"

#include "types/texttypes.h"

#include "engraving/dom/tempotext.h"

#include "log.h"

using namespace mu::inspector;
using namespace mu::notation;
using namespace mu::engraving;

static const QMap<mu::engraving::ElementType, InspectorModelType> NOTATION_ELEMENT_MODEL_TYPES = {
    { mu::engraving::ElementType::NOTE, InspectorModelType::TYPE_NOTE },
    { mu::engraving::ElementType::STEM, InspectorModelType::TYPE_NOTE },
    { mu::engraving::ElementType::NOTEDOT, InspectorModelType::TYPE_NOTE },
    { mu::engraving::ElementType::NOTEHEAD, InspectorModelType::TYPE_NOTE },
    { mu::engraving::ElementType::NOTELINE, InspectorModelType::TYPE_NOTE },
    { mu::engraving::ElementType::SHADOW_NOTE, InspectorModelType::TYPE_NOTE },
    { mu::engraving::ElementType::HOOK, InspectorModelType::TYPE_NOTE },
    { mu::engraving::ElementType::BEAM, InspectorModelType::TYPE_NOTE },
    { mu::engraving::ElementType::GLISSANDO, InspectorModelType::TYPE_GLISSANDO },
    { mu::engraving::ElementType::GLISSANDO_SEGMENT, InspectorModelType::TYPE_GLISSANDO },
    { mu::engraving::ElementType::VIBRATO, InspectorModelType::TYPE_VIBRATO },
    { mu::engraving::ElementType::VIBRATO_SEGMENT, InspectorModelType::TYPE_VIBRATO },
    { mu::engraving::ElementType::SLUR, InspectorModelType::TYPE_SLUR },
    { mu::engraving::ElementType::SLUR_SEGMENT, InspectorModelType::TYPE_SLUR },
    { mu::engraving::ElementType::HAMMER_ON_PULL_OFF, InspectorModelType::TYPE_HAMMER_ON_PULL_OFF },
    { mu::engraving::ElementType::HAMMER_ON_PULL_OFF_SEGMENT, InspectorModelType::TYPE_HAMMER_ON_PULL_OFF },
    { mu::engraving::ElementType::TIE, InspectorModelType::TYPE_TIE },
    { mu::engraving::ElementType::TIE_SEGMENT, InspectorModelType::TYPE_TIE },
    { mu::engraving::ElementType::LAISSEZ_VIB, InspectorModelType::TYPE_LAISSEZ_VIB },
    { mu::engraving::ElementType::LAISSEZ_VIB_SEGMENT, InspectorModelType::TYPE_LAISSEZ_VIB },
    { mu::engraving::ElementType::PARTIAL_TIE, InspectorModelType::TYPE_PARTIAL_TIE },
    { mu::engraving::ElementType::PARTIAL_TIE_SEGMENT, InspectorModelType::TYPE_PARTIAL_TIE },
    { mu::engraving::ElementType::TEMPO_TEXT, InspectorModelType::TYPE_TEMPO },
    { mu::engraving::ElementType::FERMATA, InspectorModelType::TYPE_FERMATA },
    { mu::engraving::ElementType::LAYOUT_BREAK, InspectorModelType::TYPE_SECTIONBREAK },
    { mu::engraving::ElementType::BAR_LINE, InspectorModelType::TYPE_BARLINE },
    { mu::engraving::ElementType::PLAY_COUNT_TEXT, InspectorModelType::TYPE_PLAY_COUNT_TEXT },
    { mu::engraving::ElementType::MARKER, InspectorModelType::TYPE_MARKER },
    { mu::engraving::ElementType::JUMP, InspectorModelType::TYPE_JUMP },
    { mu::engraving::ElementType::KEYSIG, InspectorModelType::TYPE_KEYSIGNATURE },
    { mu::engraving::ElementType::ACCIDENTAL, InspectorModelType::TYPE_ACCIDENTAL },
    { mu::engraving::ElementType::FRET_DIAGRAM, InspectorModelType::TYPE_FRET_DIAGRAM },
    { mu::engraving::ElementType::PEDAL, InspectorModelType::TYPE_PEDAL },
    { mu::engraving::ElementType::PEDAL_SEGMENT, InspectorModelType::TYPE_PEDAL },
    { mu::engraving::ElementType::SPACER, InspectorModelType::TYPE_SPACER },
    { mu::engraving::ElementType::CLEF, InspectorModelType::TYPE_CLEF },
    { mu::engraving::ElementType::HAIRPIN, InspectorModelType::TYPE_HAIRPIN },
    { mu::engraving::ElementType::HAIRPIN_SEGMENT, InspectorModelType::TYPE_HAIRPIN },
    { mu::engraving::ElementType::OTTAVA, InspectorModelType::TYPE_OTTAVA },
    { mu::engraving::ElementType::OTTAVA_SEGMENT, InspectorModelType::TYPE_OTTAVA },
    { mu::engraving::ElementType::VOLTA, InspectorModelType::TYPE_VOLTA },
    { mu::engraving::ElementType::VOLTA_SEGMENT, InspectorModelType::TYPE_VOLTA },
    { mu::engraving::ElementType::PALM_MUTE, InspectorModelType::TYPE_PALM_MUTE },
    { mu::engraving::ElementType::PALM_MUTE_SEGMENT, InspectorModelType::TYPE_PALM_MUTE },
    { mu::engraving::ElementType::LET_RING, InspectorModelType::TYPE_LET_RING },
    { mu::engraving::ElementType::LET_RING_SEGMENT, InspectorModelType::TYPE_LET_RING },
    { mu::engraving::ElementType::STAFFTYPE_CHANGE, InspectorModelType::TYPE_STAFF_TYPE_CHANGES },
    { mu::engraving::ElementType::TBOX, InspectorModelType::TYPE_TEXT_FRAME },// text frame
    { mu::engraving::ElementType::VBOX, InspectorModelType::TYPE_VERTICAL_FRAME },// vertical frame
    { mu::engraving::ElementType::HBOX, InspectorModelType::TYPE_HORIZONTAL_FRAME },// horizontal frame
    { mu::engraving::ElementType::FBOX, InspectorModelType::TYPE_FRET_FRAME },// fret diagram legend
    { mu::engraving::ElementType::ARTICULATION, InspectorModelType::TYPE_ARTICULATION },
    { mu::engraving::ElementType::TAPPING,      InspectorModelType::TYPE_TAPPING },
    { mu::engraving::ElementType::TAPPING_HALF_SLUR_SEGMENT, InspectorModelType::TYPE_TAPPING },
    { mu::engraving::ElementType::ORNAMENT, InspectorModelType::TYPE_ORNAMENT },
    { mu::engraving::ElementType::TRILL, InspectorModelType::TYPE_ORNAMENT },
    { mu::engraving::ElementType::TRILL_SEGMENT, InspectorModelType::TYPE_ORNAMENT },
    { mu::engraving::ElementType::IMAGE, InspectorModelType::TYPE_IMAGE },
    { mu::engraving::ElementType::HARMONY, InspectorModelType::TYPE_CHORD_SYMBOL },
    { mu::engraving::ElementType::AMBITUS, InspectorModelType::TYPE_AMBITUS },
    { mu::engraving::ElementType::BRACKET, InspectorModelType::TYPE_BRACKET },
    { mu::engraving::ElementType::TIMESIG, InspectorModelType::TYPE_TIME_SIGNATURE },
    { mu::engraving::ElementType::MMREST, InspectorModelType::TYPE_MMREST },
    { mu::engraving::ElementType::BEND, InspectorModelType::TYPE_BEND },
    { mu::engraving::ElementType::GUITAR_BEND, InspectorModelType::TYPE_BEND },
    { mu::engraving::ElementType::GUITAR_BEND_SEGMENT, InspectorModelType::TYPE_BEND },
    { mu::engraving::ElementType::GUITAR_BEND_HOLD, InspectorModelType::TYPE_BEND },
    { mu::engraving::ElementType::GUITAR_BEND_HOLD_SEGMENT, InspectorModelType::TYPE_BEND },
    { mu::engraving::ElementType::TREMOLOBAR, InspectorModelType::TYPE_TREMOLOBAR },
    { mu::engraving::ElementType::TREMOLO_SINGLECHORD, InspectorModelType::TYPE_TREMOLO },
    { mu::engraving::ElementType::TREMOLO_TWOCHORD, InspectorModelType::TYPE_TREMOLO },
    { mu::engraving::ElementType::MEASURE_REPEAT, InspectorModelType::TYPE_MEASURE_REPEAT },
    { mu::engraving::ElementType::TUPLET, InspectorModelType::TYPE_TUPLET },
    { mu::engraving::ElementType::TEXTLINE, InspectorModelType::TYPE_TEXT_LINE },
    { mu::engraving::ElementType::TEXTLINE_SEGMENT, InspectorModelType::TYPE_TEXT_LINE },
    { mu::engraving::ElementType::NOTELINE, InspectorModelType::TYPE_NOTELINE },
    { mu::engraving::ElementType::NOTELINE_SEGMENT, InspectorModelType::TYPE_NOTELINE },
    { mu::engraving::ElementType::GRADUAL_TEMPO_CHANGE, InspectorModelType::TYPE_GRADUAL_TEMPO_CHANGE },
    { mu::engraving::ElementType::GRADUAL_TEMPO_CHANGE_SEGMENT, InspectorModelType::TYPE_GRADUAL_TEMPO_CHANGE },
    { mu::engraving::ElementType::INSTRUMENT_NAME, InspectorModelType::TYPE_INSTRUMENT_NAME },
    { mu::engraving::ElementType::LYRICS, InspectorModelType::TYPE_LYRICS },
    { mu::engraving::ElementType::PARTIAL_LYRICSLINE, InspectorModelType::TYPE_LYRICS },
    { mu::engraving::ElementType::PARTIAL_LYRICSLINE_SEGMENT, InspectorModelType::TYPE_LYRICS },
    { mu::engraving::ElementType::REST, InspectorModelType::TYPE_REST },
    { mu::engraving::ElementType::DYNAMIC, InspectorModelType::TYPE_DYNAMIC },
    { mu::engraving::ElementType::EXPRESSION, InspectorModelType::TYPE_EXPRESSION },
    { mu::engraving::ElementType::STRING_TUNINGS, InspectorModelType::TYPE_STRING_TUNINGS },
    { mu::engraving::ElementType::SYMBOL, InspectorModelType::TYPE_SYMBOL },
};

static const QMap<mu::engraving::HairpinType, InspectorModelType> HAIRPIN_ELEMENT_MODEL_TYPES = {
    { mu::engraving::HairpinType::CRESC_HAIRPIN, InspectorModelType::TYPE_HAIRPIN },
    { mu::engraving::HairpinType::DIM_HAIRPIN, InspectorModelType::TYPE_HAIRPIN },
    { mu::engraving::HairpinType::CRESC_LINE, InspectorModelType::TYPE_CRESCENDO },
    { mu::engraving::HairpinType::DIM_LINE, InspectorModelType::TYPE_DIMINUENDO },
};

static const QMap<mu::engraving::LayoutBreakType, InspectorModelType> LAYOUT_BREAK_ELEMENT_MODEL_TYPES = {
    { mu::engraving::LayoutBreakType::SECTION, InspectorModelType::TYPE_SECTIONBREAK }
};

static const QMap<mu::engraving::TempoTextType, InspectorModelType> TEMPO_TEXT_ELEMENT_MODEL_TYPES = {
    { mu::engraving::TempoTextType::NORMAL, InspectorModelType::TYPE_TEMPO },
    { mu::engraving::TempoTextType::A_TEMPO, InspectorModelType::TYPE_A_TEMPO },
    { mu::engraving::TempoTextType::TEMPO_PRIMO, InspectorModelType::TYPE_TEMPO_PRIMO },
};

QString AbstractInspectorModel::shortcutsForActionCode(std::string code) const
{
    const muse::ui::UiAction& action = uiActionsRegister()->action(code);
    return muse::shortcuts::sequencesToNativeText(action.shortcuts);
}

AbstractInspectorModel::AbstractInspectorModel(QObject* parent, IElementRepositoryService* repository,
                                               mu::engraving::ElementType elementType)
    : QObject(parent), m_elementType(elementType), m_updatePropertiesAllowed(true)
{
    m_repository = repository;

    if (!m_repository) {
        return;
    }

    connect(m_repository->getQObject(), SIGNAL(elementsUpdated(const QList<mu::engraving::EngravingItem*>&)), this,
            SLOT(updateProperties()));
    connect(this, &AbstractInspectorModel::requestReloadPropertyItems, this, &AbstractInspectorModel::updateProperties);
}

void AbstractInspectorModel::init()
{
    onCurrentNotationChanged();
}

void AbstractInspectorModel::onCurrentNotationChanged()
{
    INotationPtr notation = currentNotation();
    if (!notation) {
        return;
    }

    notation->viewModeChanged().onNotify(this, [this]() {
        onNotationChanged({}, {});
    });

    notation->undoStack()->changesChannel().onReceive(this, [this](const ScoreChanges& changes) {
        if (changes.isTextEditing) {
            return;
        }

        if (changes.changedPropertyIdSet.empty() && changes.changedStyleIdSet.empty()) {
            return;
        }

        if (m_updatePropertiesAllowed && !isEmpty()) {
            PropertyIdSet expandedPropertyIdSet = propertyIdSetFromStyleIdSet(changes.changedStyleIdSet);
            expandedPropertyIdSet.insert(changes.changedPropertyIdSet.cbegin(), changes.changedPropertyIdSet.cend());
            onNotationChanged(expandedPropertyIdSet, changes.changedStyleIdSet);
        }

        m_updatePropertiesAllowed = true;
    });
}

bool AbstractInspectorModel::isSystemObjectBelowBottomStaff() const
{
    return m_isSystemObjectBelowBottomStaff;
}

void AbstractInspectorModel::updateIsSystemObjectBelowBottomStaff()
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

void AbstractInspectorModel::onNotationChanged(const PropertyIdSet&, const StyleIdSet&)
{
}

void AbstractInspectorModel::requestResetToDefaults()
{
    resetProperties();
}

QString AbstractInspectorModel::title() const
{
    return m_title;
}

int AbstractInspectorModel::icon() const
{
    return static_cast<int>(m_icon);
}

InspectorSectionType AbstractInspectorModel::sectionType() const
{
    return m_sectionType;
}

InspectorModelType AbstractInspectorModel::modelType() const
{
    return m_modelType;
}

ElementKey AbstractInspectorModel::makeKey(const EngravingItem* item)
{
    switch (item->type()) {
    case ElementType::TEMPO_TEXT: {
        const auto tempoText = static_cast<const TempoText*>(item);
        return ElementKey{ ElementType::TEMPO_TEXT, static_cast<int>(tempoText->tempoTextType()) };
    }
    default:
        return ElementKey{ item->type(), item->subtype() };
    }
}

InspectorModelType AbstractInspectorModel::modelTypeByElementKey(const ElementKey& elementKey)
{
    if (elementKey.type == mu::engraving::ElementType::HAIRPIN || elementKey.type == mu::engraving::ElementType::HAIRPIN_SEGMENT) {
        return HAIRPIN_ELEMENT_MODEL_TYPES.value(static_cast<mu::engraving::HairpinType>(elementKey.subtype),
                                                 InspectorModelType::TYPE_UNDEFINED);
    }

    if (elementKey.type == mu::engraving::ElementType::LAYOUT_BREAK) {
        return LAYOUT_BREAK_ELEMENT_MODEL_TYPES.value(static_cast<mu::engraving::LayoutBreakType>(elementKey.subtype),
                                                      InspectorModelType::TYPE_UNDEFINED);
    }

    if (elementKey.type == mu::engraving::ElementType::TEMPO_TEXT) {
        return TEMPO_TEXT_ELEMENT_MODEL_TYPES.value(static_cast<mu::engraving::TempoTextType>(elementKey.subtype),
                                                    InspectorModelType::TYPE_UNDEFINED);
    }

    return NOTATION_ELEMENT_MODEL_TYPES.value(elementKey.type, InspectorModelType::TYPE_UNDEFINED);
}

InspectorModelTypeSet AbstractInspectorModel::modelTypesByElementKeys(const ElementKeySet& elementKeySet)
{
    InspectorModelTypeSet types;

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

        if (toBarLine(item)->playCountText()) {
            return true;
        }
    }

    return false;
}

InspectorSectionTypeSet AbstractInspectorModel::sectionTypesByElementKeys(const ElementKeySet& elementKeySet, bool isRange,
                                                                          const QList<mu::engraving::EngravingItem*>& selectedElementList)
{
    InspectorSectionTypeSet types;

    for (const ElementKey& key : elementKeySet) {
        if (NOTATION_ELEMENT_MODEL_TYPES.contains(key.type)
            && (modelTypeByElementKey(key) != InspectorModelType::TYPE_UNDEFINED)) {
            types << InspectorSectionType::SECTION_NOTATION;
        }

        // Don't show the "Text" inspector panel for "pure" dynamics (i.e. without custom text)
        if ((TEXT_ELEMENT_TYPES.contains(key.type) && !isPureDynamics(selectedElementList)) || barlineWithPlayText(selectedElementList)) {
            types << InspectorSectionType::SECTION_TEXT;
        }

        if (key.type != mu::engraving::ElementType::INSTRUMENT_NAME) {
            types << InspectorSectionType::SECTION_GENERAL;
        }
    }

    if (isRange) {
        types << InspectorSectionType::SECTION_MEASURES;
        types << InspectorSectionType::SECTION_EMPTY_STAVES;
    }

    if (showPartsSection(selectedElementList)) {
        types << InspectorSectionType::SECTION_PARTS;
    }

    return types;
}

bool AbstractInspectorModel::showPartsSection(const QList<EngravingItem*>& selectedElementList)
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

bool AbstractInspectorModel::isEmpty() const
{
    return m_elementList.isEmpty();
}

void AbstractInspectorModel::setTitle(QString title)
{
    if (m_title == title) {
        return;
    }

    m_title = title;
    emit titleChanged();
}

void AbstractInspectorModel::setIcon(muse::ui::IconCode::Code icon)
{
    m_icon = icon;
}

void AbstractInspectorModel::setSectionType(InspectorSectionType sectionType)
{
    m_sectionType = sectionType;
}

void AbstractInspectorModel::setModelType(InspectorModelType modelType)
{
    m_modelType = modelType;
}

void AbstractInspectorModel::onPropertyValueChanged(const mu::engraving::Pid pid, const QVariant& newValue)
{
    setPropertyValue(m_elementList, pid, newValue);
}

void AbstractInspectorModel::setPropertyValue(const QList<engraving::EngravingItem*>& items, const mu::engraving::Pid pid,
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

void AbstractInspectorModel::updateProperties()
{
    requestElements();

    emit isEmptyChanged();

    if (!isEmpty()) {
        loadProperties();
    }
}

void AbstractInspectorModel::requestElements()
{
    if (m_elementType != mu::engraving::ElementType::INVALID) {
        m_elementList = m_repository->findElementsByType(m_elementType);
    }
}

mu::engraving::Sid AbstractInspectorModel::styleIdByPropertyId(const mu::engraving::Pid pid) const
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

mu::engraving::PropertyIdSet AbstractInspectorModel::propertyIdSetFromStyleIdSet(const StyleIdSet& styleIdSet) const
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

bool AbstractInspectorModel::updateStyleValue(const mu::engraving::Sid& sid, const QVariant& newValue)
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

QVariant AbstractInspectorModel::styleValue(const mu::engraving::Sid& sid) const
{
    return style() ? style()->styleValue(sid).toQVariant() : QVariant();
}

PropertyValue AbstractInspectorModel::valueToElementUnits(const mu::engraving::Pid& pid, const QVariant& value,
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

    case P_TYPE::MILLIMETRE:
        return Spatium(value.toReal()).toMM(element->spatium());

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

QVariant AbstractInspectorModel::valueFromElementUnits(const mu::engraving::Pid& pid, const PropertyValue& value,
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

    case P_TYPE::MILLIMETRE:
        return Spatium::fromMM(value.toReal(), element->spatium()).val();

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

void AbstractInspectorModel::setElementType(mu::engraving::ElementType type)
{
    m_elementType = type;
}

PropertyItem* AbstractInspectorModel::buildPropertyItem(const mu::engraving::Pid& propertyId,
                                                        std::function<void(const mu::engraving::Pid propertyId,
                                                                           const QVariant& newValue)> onPropertyChangedCallBack,
                                                        std::function<void(const mu::engraving::Sid styleId,
                                                                           const QVariant& newValue)> onStyleChangedCallBack)
{
    PropertyItem* newPropertyItem = new PropertyItem(propertyId, this);

    initPropertyItem(newPropertyItem, onPropertyChangedCallBack, onStyleChangedCallBack);

    return newPropertyItem;
}

PointFPropertyItem* AbstractInspectorModel::buildPointFPropertyItem(const mu::engraving::Pid& propertyId,
                                                                    std::function<void(const mu::engraving::Pid propertyId,
                                                                                       const QVariant& newValue)> onPropertyChangedCallBack)
{
    PointFPropertyItem* newPropertyItem = new PointFPropertyItem(propertyId, this);

    initPropertyItem(newPropertyItem, onPropertyChangedCallBack);

    return newPropertyItem;
}

void AbstractInspectorModel::initPropertyItem(PropertyItem* propertyItem,
                                              std::function<void(const mu::engraving::Pid propertyId,
                                                                 const QVariant& newValue)> onPropertyChangedCallBack,
                                              std::function<void(const mu::engraving::Sid styleId,
                                                                 const QVariant& newValue)> onStyleChangedCallBack)
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

    connect(propertyItem, &PropertyItem::propertyModified, this, propertyCallback);
    connect(propertyItem, &PropertyItem::applyToStyleRequested, this, styleCallback);
}

void AbstractInspectorModel::loadPropertyItem(PropertyItem* propertyItem, ConvertPropertyValueFunc convertElementPropertyValueFunc)
{
    loadPropertyItem(propertyItem, m_elementList, convertElementPropertyValueFunc);
}

void AbstractInspectorModel::loadPropertyItem(PropertyItem* propertyItem, const QList<EngravingItem*>& elements,
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
    QVariant defaultPropertyValue;

    bool isUndefined = false;

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

        if (!(propertyValue.isValid() && defaultPropertyValue.isValid())) {
            propertyValue = elementCurrentValue;
            defaultPropertyValue = elementDefaultValue;
        }

        isUndefined = propertyValue != elementCurrentValue;

        if (isUndefined) {
            break;
        }
    }

    //@note Some elements may support the property, some don't. If element doesn't support property it'll return invalid value.
    //      So we use that knowledge here
    propertyItem->setIsEnabled(propertyValue.isValid());

    if (isUndefined) {
        propertyValue = QVariant();
    }

    propertyItem->fillValues(propertyValue, defaultPropertyValue);
}

bool AbstractInspectorModel::isNotationExisting() const
{
    return context()->currentProject() != nullptr;
}

INotationStylePtr AbstractInspectorModel::style() const
{
    return currentNotation() ? currentNotation()->style() : nullptr;
}

INotationUndoStackPtr AbstractInspectorModel::undoStack() const
{
    return currentNotation() ? currentNotation()->undoStack() : nullptr;
}

void AbstractInspectorModel::beginCommand(const muse::TranslatableString& actionName)
{
    if (undoStack()) {
        undoStack()->prepareChanges(actionName);
    }

    //! NOTE prevents unnecessary updating of properties
    //! after changing their values in the inspector
    m_updatePropertiesAllowed = false;
}

void AbstractInspectorModel::endCommand()
{
    if (undoStack()) {
        undoStack()->commitChanges();
    }
}

INotationPtr AbstractInspectorModel::currentNotation() const
{
    return context()->currentNotation();
}

muse::async::Notification AbstractInspectorModel::currentNotationChanged() const
{
    return context()->currentNotationChanged();
}

bool AbstractInspectorModel::isMasterNotation() const
{
    INotationPtr notation = currentNotation();
    if (!notation) {
        return false;
    }

    return notation == context()->currentMasterNotation()->notation();
}

INotationSelectionPtr AbstractInspectorModel::selection() const
{
    INotationPtr notation = currentNotation();
    return notation ? notation->interaction()->selection() : nullptr;
}

void AbstractInspectorModel::updateNotation()
{
    if (!currentNotation()) {
        return;
    }

    currentNotation()->notationChanged().notify();
}
