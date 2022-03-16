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

#include "libmscore/musescoreCore.h"

#include "types/texttypes.h"

#include "log.h"

using namespace mu::inspector;
using namespace mu::notation;
using namespace mu::engraving;

static const QMap<Ms::ElementType, InspectorModelType> NOTATION_ELEMENT_MODEL_TYPES = {
    { Ms::ElementType::NOTE, InspectorModelType::TYPE_NOTE },
    { Ms::ElementType::STEM, InspectorModelType::TYPE_NOTE },
    { Ms::ElementType::NOTEDOT, InspectorModelType::TYPE_NOTE },
    { Ms::ElementType::NOTEHEAD, InspectorModelType::TYPE_NOTE },
    { Ms::ElementType::NOTELINE, InspectorModelType::TYPE_NOTE },
    { Ms::ElementType::SHADOW_NOTE, InspectorModelType::TYPE_NOTE },
    { Ms::ElementType::HOOK, InspectorModelType::TYPE_NOTE },
    { Ms::ElementType::BEAM, InspectorModelType::TYPE_NOTE },
    { Ms::ElementType::GLISSANDO, InspectorModelType::TYPE_GLISSANDO },
    { Ms::ElementType::GLISSANDO_SEGMENT, InspectorModelType::TYPE_GLISSANDO },
    { Ms::ElementType::VIBRATO, InspectorModelType::TYPE_VIBRATO },
    { Ms::ElementType::VIBRATO_SEGMENT, InspectorModelType::TYPE_VIBRATO },
    { Ms::ElementType::TEMPO_TEXT, InspectorModelType::TYPE_TEMPO },
    { Ms::ElementType::FERMATA, InspectorModelType::TYPE_FERMATA },
    { Ms::ElementType::LAYOUT_BREAK, InspectorModelType::TYPE_SECTIONBREAK },
    { Ms::ElementType::BAR_LINE, InspectorModelType::TYPE_BARLINE },
    { Ms::ElementType::MARKER, InspectorModelType::TYPE_MARKER },
    { Ms::ElementType::JUMP, InspectorModelType::TYPE_JUMP },
    { Ms::ElementType::KEYSIG, InspectorModelType::TYPE_KEYSIGNATURE },
    { Ms::ElementType::ACCIDENTAL, InspectorModelType::TYPE_ACCIDENTAL },
    { Ms::ElementType::FRET_DIAGRAM, InspectorModelType::TYPE_FRET_DIAGRAM },
    { Ms::ElementType::PEDAL, InspectorModelType::TYPE_PEDAL },
    { Ms::ElementType::PEDAL_SEGMENT, InspectorModelType::TYPE_PEDAL },
    { Ms::ElementType::SPACER, InspectorModelType::TYPE_SPACER },
    { Ms::ElementType::CLEF, InspectorModelType::TYPE_CLEF },
    { Ms::ElementType::HAIRPIN, InspectorModelType::TYPE_HAIRPIN },
    { Ms::ElementType::HAIRPIN_SEGMENT, InspectorModelType::TYPE_HAIRPIN },
    { Ms::ElementType::OTTAVA, InspectorModelType::TYPE_OTTAVA },
    { Ms::ElementType::OTTAVA_SEGMENT, InspectorModelType::TYPE_OTTAVA },
    { Ms::ElementType::VOLTA, InspectorModelType::TYPE_VOLTA },
    { Ms::ElementType::VOLTA_SEGMENT, InspectorModelType::TYPE_VOLTA },
    { Ms::ElementType::PALM_MUTE, InspectorModelType::TYPE_PALM_MUTE },
    { Ms::ElementType::PALM_MUTE_SEGMENT, InspectorModelType::TYPE_PALM_MUTE },
    { Ms::ElementType::LET_RING, InspectorModelType::TYPE_LET_RING },
    { Ms::ElementType::LET_RING_SEGMENT, InspectorModelType::TYPE_LET_RING },
    { Ms::ElementType::STAFFTYPE_CHANGE, InspectorModelType::TYPE_STAFF_TYPE_CHANGES },
    { Ms::ElementType::TBOX, InspectorModelType::TYPE_TEXT_FRAME },// text frame
    { Ms::ElementType::VBOX, InspectorModelType::TYPE_VERTICAL_FRAME },// vertical frame
    { Ms::ElementType::HBOX, InspectorModelType::TYPE_HORIZONTAL_FRAME },// horizontal frame
    { Ms::ElementType::ARTICULATION, InspectorModelType::TYPE_ARTICULATION },
    { Ms::ElementType::IMAGE, InspectorModelType::TYPE_IMAGE },
    { Ms::ElementType::HARMONY, InspectorModelType::TYPE_CHORD_SYMBOL },
    { Ms::ElementType::AMBITUS, InspectorModelType::TYPE_AMBITUS },
    { Ms::ElementType::BRACKET, InspectorModelType::TYPE_BRACKET },
    { Ms::ElementType::TIMESIG, InspectorModelType::TYPE_TIME_SIGNATURE },
    { Ms::ElementType::MMREST, InspectorModelType::TYPE_MMREST },
    { Ms::ElementType::BEND, InspectorModelType::TYPE_BEND },
    { Ms::ElementType::TREMOLOBAR, InspectorModelType::TYPE_TREMOLOBAR },
    { Ms::ElementType::TREMOLO, InspectorModelType::TYPE_TREMOLO },
    { Ms::ElementType::MEASURE_REPEAT, InspectorModelType::TYPE_MEASURE_REPEAT },
    { Ms::ElementType::TUPLET, InspectorModelType::TYPE_TUPLET },
    { Ms::ElementType::TEXTLINE, InspectorModelType::TYPE_TEXT_LINE },
    { Ms::ElementType::TEXTLINE_SEGMENT, InspectorModelType::TYPE_TEXT_LINE },
    { Ms::ElementType::INSTRUMENT_NAME, InspectorModelType::TYPE_INSTRUMENT_NAME }
};

static QMap<Ms::HairpinType, InspectorModelType> HAIRPIN_ELEMENT_MODEL_TYPES = {
    { Ms::HairpinType::CRESC_HAIRPIN, InspectorModelType::TYPE_HAIRPIN },
    { Ms::HairpinType::DECRESC_HAIRPIN, InspectorModelType::TYPE_HAIRPIN },
    { Ms::HairpinType::CRESC_LINE, InspectorModelType::TYPE_CRESCENDO },
    { Ms::HairpinType::DECRESC_LINE, InspectorModelType::TYPE_DIMINUENDO },
};

static QMap<Ms::LayoutBreakType, InspectorModelType> LAYOUT_BREAK_ELEMENT_MODEL_TYPES = {
    { Ms::LayoutBreakType::SECTION, InspectorModelType::TYPE_SECTIONBREAK }
};

AbstractInspectorModel::AbstractInspectorModel(QObject* parent, IElementRepositoryService* repository, Ms::ElementType elementType)
    : QObject(parent), m_elementType(elementType), m_updatePropertiesAllowed(true)
{
    m_repository = repository;

    if (!m_repository) {
        return;
    }

    setupCurrentNotationChangedConnection();

    connect(m_repository->getQObject(), SIGNAL(elementsUpdated(const QList<Ms::EngravingItem*>&)), this, SLOT(updateProperties()));
    connect(this, &AbstractInspectorModel::requestReloadPropertyItems, this, &AbstractInspectorModel::updateProperties);
}

void AbstractInspectorModel::setupCurrentNotationChangedConnection()
{
    auto listenNotationChanged = [this]() {
        INotationPtr notation = currentNotation();
        if (!notation) {
            return;
        }

        notation->notationChanged().onNotify(this, [this]() {
            if (m_updatePropertiesAllowed && !isEmpty()) {
                updatePropertiesOnNotationChanged();
            }

            m_updatePropertiesAllowed = true;
        });
    };

    listenNotationChanged();

    currentNotationChanged().onNotify(this, [listenNotationChanged]() {
        listenNotationChanged();
    });
}

void AbstractInspectorModel::updatePropertiesOnNotationChanged()
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

InspectorModelType AbstractInspectorModel::modelTypeByElementKey(const ElementKey& elementKey)
{
    if (elementKey.type == Ms::ElementType::HAIRPIN || elementKey.type == Ms::ElementType::HAIRPIN_SEGMENT) {
        return HAIRPIN_ELEMENT_MODEL_TYPES.value(static_cast<Ms::HairpinType>(elementKey.subtype), InspectorModelType::TYPE_UNDEFINED);
    }

    if (elementKey.type == Ms::ElementType::LAYOUT_BREAK) {
        return LAYOUT_BREAK_ELEMENT_MODEL_TYPES.value(static_cast<Ms::LayoutBreakType>(elementKey.subtype),
                                                      InspectorModelType::TYPE_UNDEFINED);
    }

    if (elementKey.type == Ms::ElementType::ARTICULATION) {
        if (Ms::Articulation::isOrnament(elementKey.subtype)) {
            return InspectorModelType::TYPE_ORNAMENT;
        }
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

InspectorSectionTypeSet AbstractInspectorModel::sectionTypesByElementKeys(const ElementKeySet& elementKeySet, bool isRange)
{
    InspectorSectionTypeSet types;

    for (const ElementKey& key : elementKeySet) {
        if (NOTATION_ELEMENT_MODEL_TYPES.keys().contains(key.type)
            && (modelTypeByElementKey(key) != InspectorModelType::TYPE_UNDEFINED)) {
            types << InspectorSectionType::SECTION_NOTATION;
        }

        if (TEXT_ELEMENT_TYPES.contains(key.type)) {
            types << InspectorSectionType::SECTION_TEXT;
        }

        if (key.type != Ms::ElementType::INSTRUMENT_NAME) {
            types << InspectorSectionType::SECTION_GENERAL;
        }
    }

    if (isRange) {
        types << InspectorSectionType::SECTION_MEASURES;
    }

    return types;
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

void AbstractInspectorModel::onPropertyValueChanged(const Ms::Pid pid, const QVariant& newValue)
{
    if (isEmpty()) {
        return;
    }

    beginCommand();

    for (Ms::EngravingItem* element : m_elementList) {
        IF_ASSERT_FAILED(element) {
            continue;
        }

        Ms::PropertyFlags ps = element->propertyFlags(pid);

        if (ps == Ms::PropertyFlags::STYLED) {
            ps = Ms::PropertyFlags::UNSTYLED;
        }

        PropertyValue propValue = valueToElementUnits(pid, newValue, element);
        element->undoChangeProperty(pid, propValue, ps);
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
    if (m_elementType != Ms::ElementType::INVALID) {
        m_elementList = m_repository->findElementsByType(m_elementType);
    }
}

Ms::Sid AbstractInspectorModel::styleIdByPropertyId(const Ms::Pid pid) const
{
    Ms::Sid result = Ms::Sid::NOSTYLE;

    for (const Ms::EngravingItem* element : m_elementList) {
        result = element->getPropertyStyle(pid);

        if (result != Ms::Sid::NOSTYLE) {
            break;
        }
    }

    return result;
}

void AbstractInspectorModel::updateStyleValue(const Ms::Sid& sid, const QVariant& newValue)
{
    PropertyValue newVal = PropertyValue::fromQVariant(newValue, Ms::MStyle::valueType(sid));
    if (style() && style()->styleValue(sid) != newVal) {
        beginCommand();
        style()->setStyleValue(sid, newVal);
        endCommand();
    }
}

QVariant AbstractInspectorModel::styleValue(const Ms::Sid& sid) const
{
    return style() ? style()->styleValue(sid).toQVariant() : QVariant();
}

PropertyValue AbstractInspectorModel::valueToElementUnits(const Ms::Pid& pid, const QVariant& value, const Ms::EngravingItem* element) const
{
    if (Ms::Pid::VERSE == pid) {
        return value.toInt() - 1;
    }

    auto toPoint = [](const QVariant& v) {
        return PointF::fromQPointF(v.value<QPointF>());
    };

    P_TYPE type = Ms::propertyType(pid);
    switch (type) {
    case P_TYPE::POINT: {
        if (element->sizeIsSpatiumDependent()) {
            return toPoint(value) * element->spatium();
        } else {
            return toPoint(value) * Ms::DPMM;
        }
    }

    case P_TYPE::MILLIMETRE:
        return Spatium(value.toReal()).toMM(element->spatium());

    case P_TYPE::SPATIUM:
        return Spatium(value.toReal());

    case P_TYPE::TEMPO:
        return BeatsPerSecond::fromBPM(BeatsPerMinute(value.toReal()));

    case P_TYPE::INT_LIST:
        return value.value<QList<int> >();

    case P_TYPE::COLOR:
        return Color::fromQColor(value.value<QColor>());

    default:
        return PropertyValue::fromQVariant(value, type);
    }
}

QVariant AbstractInspectorModel::valueFromElementUnits(const Ms::Pid& pid, const PropertyValue& value,
                                                       const Ms::EngravingItem* element) const
{
    if (Ms::Pid::VERSE == pid) {
        return value.toInt() + 1;
    }

    switch (value.type()) {
    case P_TYPE::POINT: {
        if (element->sizeIsSpatiumDependent()) {
            return value.value<PointF>().toQPointF() / element->spatium();
        } else {
            return value.value<PointF>().toQPointF() / Ms::DPMM;
        }
    }

    case P_TYPE::MILLIMETRE:
        return Spatium::fromMM(value.toReal(), element->spatium()).val();

    case P_TYPE::SPATIUM:
        return value.value<Spatium>().val();

    case P_TYPE::TEMPO:
        return value.value<BeatsPerSecond>().toBPM().val;

    case P_TYPE::DIRECTION_V:
        return static_cast<int>(value.value<Ms::DirectionV>());

    case P_TYPE::INT_LIST: {
        QStringList strList;

        for (const int i : value.value<QList<int> >()) {
            strList << QString("%1").arg(i);
        }

        return strList.join(",");
    }
    case P_TYPE::COLOR:
        return value.value<mu::draw::Color>().toQColor();
    default:
        return value.toQVariant();
    }
}

PropertyItem* AbstractInspectorModel::buildPropertyItem(const Ms::Pid& propertyId, std::function<void(const Ms::Pid propertyId,
                                                                                                      const QVariant& newValue)> onPropertyChangedCallBack)
{
    PropertyItem* newPropertyItem = new PropertyItem(propertyId, this);

    auto callback = onPropertyChangedCallBack;

    if (!callback) {
        callback = [this](const Ms::Pid propertyId, const QVariant& newValue) {
            onPropertyValueChanged(propertyId, newValue);
        };
    }

    connect(newPropertyItem, &PropertyItem::propertyModified, this, callback);
    connect(newPropertyItem, &PropertyItem::applyToStyleRequested, this, [this](const Ms::Sid sid, const QVariant& newStyleValue) {
        updateStyleValue(sid, newStyleValue);

        emit requestReloadPropertyItems();
    });

    return newPropertyItem;
}

void AbstractInspectorModel::loadPropertyItem(PropertyItem* propertyItem,
                                              std::function<QVariant(const QVariant&)> convertElementPropertyValueFunc)
{
    if (!propertyItem || m_elementList.isEmpty()) {
        return;
    }

    Ms::Pid pid = propertyItem->propertyId();

    Ms::Sid styleId = styleIdByPropertyId(pid);
    propertyItem->setStyleId(styleId);

    QVariant propertyValue;
    QVariant defaultPropertyValue;

    bool isUndefined = false;

    for (const Ms::EngravingItem* element : m_elementList) {
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
