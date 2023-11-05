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
#include "abstractinspectormodel.h"
#include "engraving/dom/dynamic.h"

#include "dataformatter.h"

#include "types/texttypes.h"

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
    { mu::engraving::ElementType::TIE, InspectorModelType::TYPE_TIE },
    { mu::engraving::ElementType::TIE_SEGMENT, InspectorModelType::TYPE_TIE },
    { mu::engraving::ElementType::TEMPO_TEXT, InspectorModelType::TYPE_TEMPO },
    { mu::engraving::ElementType::FERMATA, InspectorModelType::TYPE_FERMATA },
    { mu::engraving::ElementType::LAYOUT_BREAK, InspectorModelType::TYPE_SECTIONBREAK },
    { mu::engraving::ElementType::BAR_LINE, InspectorModelType::TYPE_BARLINE },
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
    { mu::engraving::ElementType::ARTICULATION, InspectorModelType::TYPE_ARTICULATION },
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
    { mu::engraving::ElementType::TREMOLO, InspectorModelType::TYPE_TREMOLO },
    { mu::engraving::ElementType::MEASURE_REPEAT, InspectorModelType::TYPE_MEASURE_REPEAT },
    { mu::engraving::ElementType::TUPLET, InspectorModelType::TYPE_TUPLET },
    { mu::engraving::ElementType::TEXTLINE, InspectorModelType::TYPE_TEXT_LINE },
    { mu::engraving::ElementType::TEXTLINE_SEGMENT, InspectorModelType::TYPE_TEXT_LINE },
    { mu::engraving::ElementType::GRADUAL_TEMPO_CHANGE, InspectorModelType::TYPE_GRADUAL_TEMPO_CHANGE },
    { mu::engraving::ElementType::GRADUAL_TEMPO_CHANGE_SEGMENT, InspectorModelType::TYPE_GRADUAL_TEMPO_CHANGE },
    { mu::engraving::ElementType::INSTRUMENT_NAME, InspectorModelType::TYPE_INSTRUMENT_NAME },
    { mu::engraving::ElementType::LYRICS, InspectorModelType::TYPE_LYRICS },
    { mu::engraving::ElementType::REST, InspectorModelType::TYPE_REST },
    { mu::engraving::ElementType::DYNAMIC, InspectorModelType::TYPE_DYNAMIC },
    { mu::engraving::ElementType::EXPRESSION, InspectorModelType::TYPE_EXPRESSION },
    { mu::engraving::ElementType::STRING_TUNINGS, InspectorModelType::TYPE_STRING_TUNINGS }
};

static QMap<mu::engraving::HairpinType, InspectorModelType> HAIRPIN_ELEMENT_MODEL_TYPES = {
    { mu::engraving::HairpinType::CRESC_HAIRPIN, InspectorModelType::TYPE_HAIRPIN },
    { mu::engraving::HairpinType::DECRESC_HAIRPIN, InspectorModelType::TYPE_HAIRPIN },
    { mu::engraving::HairpinType::CRESC_LINE, InspectorModelType::TYPE_CRESCENDO },
    { mu::engraving::HairpinType::DECRESC_LINE, InspectorModelType::TYPE_DIMINUENDO },
};

static QMap<mu::engraving::LayoutBreakType, InspectorModelType> LAYOUT_BREAK_ELEMENT_MODEL_TYPES = {
    { mu::engraving::LayoutBreakType::SECTION, InspectorModelType::TYPE_SECTIONBREAK }
};

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

    notation->undoStack()->changesChannel().onReceive(this, [this](const ChangesRange& range) {
        if (range.changedPropertyIdSet.empty() && range.changedStyleIdSet.empty()) {
            return;
        }

        if (m_updatePropertiesAllowed && !isEmpty()) {
            PropertyIdSet expandedPropertyIdSet = propertyIdSetFromStyleIdSet(range.changedStyleIdSet);
            expandedPropertyIdSet.insert(range.changedPropertyIdSet.cbegin(), range.changedPropertyIdSet.cend());
            onNotationChanged(expandedPropertyIdSet, range.changedStyleIdSet);
        }

        m_updatePropertiesAllowed = true;
    });
}

void AbstractInspectorModel::onNotationChanged(const PropertyIdSet&, const StyleIdSet&)
{
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
    for (EngravingItem* item : selectedElementList) {
        if (!item->isTextBase()) {
            continue;
        }
        if (!item->isDynamic()) {
            return false;
        }
        Dynamic* dynamic = toDynamic(item);
        if (dynamic->hasCustomText()) {
            return false;
        }
    }
    return true;
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
        if (TEXT_ELEMENT_TYPES.contains(key.type) && !isPureDynamics(selectedElementList)) {
            types << InspectorSectionType::SECTION_TEXT;
        }

        if (key.type != mu::engraving::ElementType::INSTRUMENT_NAME) {
            types << InspectorSectionType::SECTION_GENERAL;
        }
    }

    if (isRange) {
        types << InspectorSectionType::SECTION_MEASURES;
    }

    if (showPartsSection(selectedElementList)) {
        types << InspectorSectionType::SECTION_PARTS;
    }

    return types;
}

bool AbstractInspectorModel::showPartsSection(const QList<EngravingItem*>& selectedElementList)
{
    for (EngravingItem* element : selectedElementList) {
        if (!element->score()->isMaster() || element->canBeExcludedFromOtherParts()) {
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

void AbstractInspectorModel::setIcon(mu::ui::IconCode::Code icon)
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

void AbstractInspectorModel::setElementType(mu::engraving::ElementType type)
{
    m_elementType = type;
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
            if (mu::contains(styleIdSet, property.sid)) {
                result.insert(property.pid);
            }
        }
    }

    return result;
}

QVariant AbstractInspectorModel::styleValue(const mu::engraving::Sid& sid) const
{
    return style() ? style()->styleValue(sid).toQVariant() : QVariant();
}

PropertyValue AbstractInspectorModel::valueToElementUnits(const mu::engraving::Pid& pid, const QVariant& value,
                                                          const mu::engraving::EngravingItem* element)
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
        if (element) {
            if (pid == Pid::OFFSET ? element->offsetIsSpatiumDependent() : element->sizeIsSpatiumDependent()) {
                return toPoint(value) * element->spatium();
            } else {
                return toPoint(value) * mu::engraving::DPMM;
            }
        }
        // If !element, then it's for style; no need to multiply with spatium in that case,
        // so go to the bottom of this method
        break;
    }

    case P_TYPE::MILLIMETRE:
        if (!element) {
            // then it's for style
            return value.toReal();
        }

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
        break;
    }

    return PropertyValue::fromQVariant(value, type);
}

QVariant AbstractInspectorModel::valueFromElementUnits(const mu::engraving::Pid& pid, const PropertyValue& value,
                                                       const mu::engraving::EngravingItem* element)
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
        return value.value<mu::draw::Color>().toQColor();
    default:
        return value.toQVariant();
    }
}

AbstractInspectorModel::ElementDependent_InternalToUi_Converter
AbstractInspectorModel::default_internalToUi_converter(engraving::Pid pid)
{
    return [pid](const engraving::PropertyValue& propertyValue, const engraving::EngravingItem* element) {
        return valueFromElementUnits(pid, propertyValue, element);
    };
}

AbstractInspectorModel::ElementDependent_InternalToUi_Converter
AbstractInspectorModel::make_elementDependent_internalToUi_converter(ElementIndependent_InternalToUi_Converter converter)
{
    IF_ASSERT_FAILED(converter) {
        return nullptr;
    }

    return [converter](const engraving::PropertyValue& propertyValue, const engraving::EngravingItem*) {
        return converter(propertyValue);
    };
}

AbstractInspectorModel::ElementDependent_InternalToUi_Converter
AbstractInspectorModel::roundedDouble_internalToUi_converter(engraving::Pid pid)
{
    return [pid](const engraving::PropertyValue& propertyValue, const engraving::EngravingItem* element) {
        return DataFormatter::roundDouble(valueFromElementUnits(pid, propertyValue, element).toDouble());
    };
}

AbstractInspectorModel::ElementDependent_UiToInternal_Converter
AbstractInspectorModel::default_element_uiToInternal_converter(engraving::Pid pid)
{
    return [pid](const QVariant& value, const engraving::EngravingItem* element) {
        return valueToElementUnits(pid, value, element);
    };
}

AbstractInspectorModel::ElementIndependent_UiToInternal_Converter
AbstractInspectorModel::default_style_uiToInternal_converter(engraving::Pid pid)
{
    return [pid](const QVariant& value) {
        return valueToElementUnits(pid, value, nullptr);
    };
}

AbstractInspectorModel::ElementDependent_UiToInternal_Converter
AbstractInspectorModel::make_elementDependent_uiToInternal_converter(ElementIndependent_UiToInternal_Converter converter)
{
    IF_ASSERT_FAILED(converter) {
        return nullptr;
    }

    return [converter](const QVariant& value, const engraving::EngravingItem*) {
        return converter(value);
    };
}

AbstractInspectorModel::SetProperty_Callback AbstractInspectorModel::default_setProperty_callback(engraving::Pid pid)
{
    return make_setProperty_callback(default_element_uiToInternal_converter(pid));
}

AbstractInspectorModel::SetStyleValue_Callback AbstractInspectorModel::default_setStyleValue_callback(engraving::Pid pid)
{
    return make_setStyleValue_callback(default_style_uiToInternal_converter(pid));
}

AbstractInspectorModel::SetProperty_Callback AbstractInspectorModel::make_setProperty_callback(
    ElementIndependent_UiToInternal_Converter converter)
{
    IF_ASSERT_FAILED(converter) {
        return nullptr;
    }

    return [this, converter](engraving::Pid pid, const QVariant& newValue) {
        engraving::PropertyValue propertyValue = converter(newValue);
        setProperty(pid, propertyValue);
    };
}

AbstractInspectorModel::SetProperty_Callback AbstractInspectorModel::make_setProperty_callback(
    ElementDependent_UiToInternal_Converter converter)
{
    IF_ASSERT_FAILED(converter) {
        return nullptr;
    }

    return [this, converter](engraving::Pid pid, const QVariant& newValue) {
        setProperty(pid, newValue, converter);
    };
}

AbstractInspectorModel::SetStyleValue_Callback AbstractInspectorModel::make_setStyleValue_callback(
    ElementIndependent_UiToInternal_Converter converter)
{
    IF_ASSERT_FAILED(converter) {
        return nullptr;
    }

    return [this, converter](engraving::Sid sid, const QVariant& newValue) {
        setStyleValue(sid, newValue, converter);

        emit requestReloadPropertyItems();
    };
}

void AbstractInspectorModel::setProperty(engraving::Pid pid, const QVariant& newValue, ElementDependent_UiToInternal_Converter converter)
{
    setProperty(m_elementList, pid, newValue, converter);
}

void AbstractInspectorModel::setProperty(const ElementList& items, engraving::Pid pid, const QVariant& newValue,
                                         ElementDependent_UiToInternal_Converter converter)
{
    if (isEmpty()) {
        return;
    }

    beginCommand();

    for (mu::engraving::EngravingItem* item : items) {
        IF_ASSERT_FAILED(item) {
            continue;
        }

        mu::engraving::PropertyFlags ps = item->propertyFlags(pid);

        if (ps == mu::engraving::PropertyFlags::STYLED) {
            ps = mu::engraving::PropertyFlags::UNSTYLED;
        }

        PropertyValue propValue = converter(newValue, item);
        item->undoChangeProperty(pid, propValue, ps);
    }

    updateNotation();
    endCommand();
}

void AbstractInspectorModel::setProperty(engraving::Pid pid, const engraving::PropertyValue& newValue)
{
    setProperty(m_elementList, pid, newValue);
}

void AbstractInspectorModel::setProperty(const ElementList& items, engraving::Pid pid, const engraving::PropertyValue& newValue)
{
    if (isEmpty()) {
        return;
    }

    beginCommand();

    for (mu::engraving::EngravingItem* item : items) {
        IF_ASSERT_FAILED(item) {
            continue;
        }

        mu::engraving::PropertyFlags ps = item->propertyFlags(pid);

        if (ps == mu::engraving::PropertyFlags::STYLED) {
            ps = mu::engraving::PropertyFlags::UNSTYLED;
        }

        item->undoChangeProperty(pid, newValue, ps);
    }

    updateNotation();
    endCommand();
}

void AbstractInspectorModel::setStyleValue(engraving::Sid sid, const QVariant& newValue,
                                           ElementIndependent_UiToInternal_Converter converter)
{
    PropertyValue newVal = converter(newValue);

    if (style() && style()->styleValue(sid) != newVal) {
        beginCommand();
        style()->setStyleValue(sid, newVal);
        endCommand();
    }
}

bool AbstractInspectorModel::setStyleValue(engraving::Sid sid, const PropertyValue& newValue)
{
    if (style() && style()->styleValue(sid) != newValue) {
        beginCommand();
        style()->setStyleValue(sid, newValue);
        endCommand();
        return true;
    }

    return false;
}

PropertyItem* AbstractInspectorModel::buildPropertyItem(const mu::engraving::Pid& pid)
{
    return buildPropertyItem(pid, default_element_uiToInternal_converter(pid), default_style_uiToInternal_converter(pid));
}

PropertyItem* AbstractInspectorModel::buildPropertyItem(const mu::engraving::Pid& pid, ElementIndependent_UiToInternal_Converter converter)
{
    return buildPropertyItem(pid, make_setProperty_callback(converter), make_setStyleValue_callback(converter));
}

PropertyItem* AbstractInspectorModel::buildPropertyItem(const mu::engraving::Pid& pid,
                                                        ElementDependent_UiToInternal_Converter elementConverter,
                                                        ElementIndependent_UiToInternal_Converter styleConverter)
{
    return buildPropertyItem(pid, make_setProperty_callback(elementConverter), make_setStyleValue_callback(styleConverter));
}

PropertyItem* AbstractInspectorModel::buildPropertyItem(const mu::engraving::Pid& propertyId,
                                                        SetProperty_Callback propertySetter,
                                                        SetStyleValue_Callback styleSetter)
{
    PropertyItem* propertyItem = new PropertyItem(propertyId, this);

    if (!propertySetter) {
        propertySetter = default_setProperty_callback(propertyId);
    }

    if (!styleSetter) {
        styleSetter = default_setStyleValue_callback(propertyId);
    }

    connect(propertyItem, &PropertyItem::propertyModified, this, propertySetter);
    connect(propertyItem, &PropertyItem::applyToStyleRequested, this, styleSetter);

    return propertyItem;
}

void AbstractInspectorModel::loadPropertyItem(PropertyItem* propertyItem, const ElementList& items)
{
    loadPropertyItem(propertyItem, ElementDependent_InternalToUi_Converter(), items);
}

void AbstractInspectorModel::loadPropertyItem(PropertyItem* propertyItem, ElementIndependent_InternalToUi_Converter converter)
{
    loadPropertyItem(propertyItem, make_elementDependent_internalToUi_converter(converter), m_elementList);
}

void AbstractInspectorModel::loadPropertyItem(PropertyItem* propertyItem, ElementIndependent_InternalToUi_Converter converter,
                                              const ElementList& items)
{
    loadPropertyItem(propertyItem, make_elementDependent_internalToUi_converter(converter), items);
}

void AbstractInspectorModel::loadPropertyItem(PropertyItem* propertyItem, ElementDependent_InternalToUi_Converter converter)
{
    loadPropertyItem(propertyItem, converter, m_elementList);
}

void AbstractInspectorModel::loadPropertyItem(PropertyItem* propertyItem, ElementDependent_InternalToUi_Converter converter,
                                              const ElementList& elements)
{
    if (!propertyItem || elements.isEmpty()) {
        return;
    }

    mu::engraving::Pid pid = propertyItem->propertyId();

    if (!converter) {
        converter = default_internalToUi_converter(pid);
    }

    mu::engraving::Sid styleId = styleIdByPropertyId(pid);
    propertyItem->setStyleId(styleId);

    QVariant propertyValue;
    QVariant defaultPropertyValue;

    bool isUndefined = false;

    for (const mu::engraving::EngravingItem* element : elements) {
        IF_ASSERT_FAILED(element) {
            continue;
        }

        QVariant elementCurrentValue = converter(element->getProperty(pid), element);
        QVariant elementDefaultValue = converter(element->propertyDefault(pid), element);

        bool isPropertySupportedByElement = elementCurrentValue.isValid();

        if (!isPropertySupportedByElement) {
            continue;
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

void AbstractInspectorModel::beginCommand()
{
    if (undoStack()) {
        undoStack()->prepareChanges();
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

mu::async::Notification AbstractInspectorModel::currentNotationChanged() const
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
