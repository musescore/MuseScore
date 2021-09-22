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

#include "log.h"
#include "types/texttypes.h"

using namespace mu::inspector;
using namespace mu::notation;

using InspectorModelType = AbstractInspectorModel::InspectorModelType;

static const QMap<ElementKey, InspectorModelType> NOTATION_ELEMENT_MODEL_TYPES = {
    { Ms::ElementType::NOTE, InspectorModelType::TYPE_NOTE },
    { Ms::ElementType::STEM, InspectorModelType::TYPE_STEM },
    { Ms::ElementType::NOTEDOT, InspectorModelType::TYPE_NOTE },
    { Ms::ElementType::NOTEHEAD, InspectorModelType::TYPE_NOTEHEAD },
    { Ms::ElementType::NOTELINE, InspectorModelType::TYPE_NOTE },
    { Ms::ElementType::SHADOW_NOTE, InspectorModelType::TYPE_NOTE },
    { Ms::ElementType::HOOK, InspectorModelType::TYPE_HOOK },
    { Ms::ElementType::BEAM, InspectorModelType::TYPE_BEAM },
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
    { Ms::ElementType::MEASURE_REPEAT, InspectorModelType::TYPE_MEASURE_REPEAT }
};

AbstractInspectorModel::AbstractInspectorModel(QObject* parent, IElementRepositoryService* repository, Ms::ElementType elementType)
    : QObject(parent), m_elementType(elementType)
{
    m_repository = repository;

    if (!m_repository) {
        return;
    }

    connect(m_repository->getQObject(), SIGNAL(elementsUpdated()), this, SLOT(updateProperties()));
    connect(this, &AbstractInspectorModel::requestReloadPropertyItems, this, &AbstractInspectorModel::updateProperties);
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

AbstractInspectorModel::InspectorSectionType AbstractInspectorModel::sectionType() const
{
    return m_sectionType;
}

InspectorModelType AbstractInspectorModel::modelType() const
{
    return m_modelType;
}

QList<AbstractInspectorModel::InspectorSectionType> AbstractInspectorModel::sectionTypesFromElementKey(const ElementKey& elementKey)
{
    QList<AbstractInspectorModel::InspectorSectionType> result;
    if (NOTATION_ELEMENT_MODEL_TYPES.keys().contains(elementKey)) {
        result << InspectorSectionType::SECTION_NOTATION;
    }

    if (TEXT_ELEMENT_TYPES.contains(elementKey.type)) {
        result << InspectorSectionType::SECTION_TEXT;
    }

    return result;
}

InspectorModelType AbstractInspectorModel::notationElementModelType(const ElementKey& elementKey)
{
    if (NOTATION_ELEMENT_MODEL_TYPES.contains(elementKey)) {
        return NOTATION_ELEMENT_MODEL_TYPES.value(elementKey);
    }

    return InspectorModelType::TYPE_UNDEFINED;
}

ElementKey AbstractInspectorModel::elementKey(const InspectorModelType& modelType)
{
    if (modelType == InspectorModelType::TYPE_UNDEFINED) {
        return Ms::ElementType::INVALID;
    }

    if (NOTATION_ELEMENT_MODEL_TYPES.values().contains(modelType)) {
        return NOTATION_ELEMENT_MODEL_TYPES.key(modelType);
    }

    return Ms::ElementType::TEXT;
}

bool AbstractInspectorModel::isEmpty() const
{
    return m_isEmpty;
}

QList<ElementKey> AbstractInspectorModel::supportedElementTypesBySectionType(
    const AbstractInspectorModel::InspectorSectionType sectionType)
{
    switch (sectionType) {
    case InspectorSectionType::SECTION_GENERAL:
        return { Ms::ElementType::MAXTYPE };
    case InspectorSectionType::SECTION_NOTATION: {
        return NOTATION_ELEMENT_MODEL_TYPES.keys();
    }
    case InspectorSectionType::SECTION_TEXT: {
        QList<ElementKey> keys;

        for (Ms::ElementType elementType : TEXT_ELEMENT_TYPES) {
            keys << ElementKey(elementType);
        }

        return keys;
    }
    default:
        return QList<ElementKey>();
    }
}

void AbstractInspectorModel::setTitle(QString title)
{
    if (m_title == title) {
        return;
    }

    m_title = title;
}

void AbstractInspectorModel::setIcon(mu::ui::IconCode::Code icon)
{
    m_icon = icon;
}

void AbstractInspectorModel::setSectionType(AbstractInspectorModel::InspectorSectionType sectionType)
{
    m_sectionType = sectionType;
}

void AbstractInspectorModel::setModelType(InspectorModelType modelType)
{
    m_modelType = modelType;
}

void AbstractInspectorModel::onPropertyValueChanged(const Ms::Pid pid, const QVariant& newValue)
{
    if (!hasAcceptableElements()) {
        return;
    }

    beginCommand();

    QVariant convertedValue;

    for (Ms::EngravingItem* element : m_elementList) {
        IF_ASSERT_FAILED(element) {
            continue;
        }

        Ms::PropertyFlags ps = element->propertyFlags(pid);

        if (ps == Ms::PropertyFlags::STYLED) {
            ps = Ms::PropertyFlags::UNSTYLED;
        }

        convertedValue = valueToElementUnits(pid, newValue, element);

        element->undoChangeProperty(pid, convertedValue, ps);
    }

    updateNotation();
    endCommand();

    emit elementsModified();
}

void AbstractInspectorModel::setIsEmpty(bool isEmpty)
{
    if (m_isEmpty == isEmpty) {
        return;
    }

    m_isEmpty = isEmpty;
    emit isEmptyChanged(m_isEmpty);
}

void AbstractInspectorModel::updateProperties()
{
    requestElements();

    setIsEmpty(!hasAcceptableElements());

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
    if (style() && style()->styleValue(sid) != newValue) {
        beginCommand();
        style()->setStyleValue(sid, newValue);
        endCommand();
    }
}

QVariant AbstractInspectorModel::styleValue(const Ms::Sid& sid) const
{
    return style() ? style()->styleValue(sid) : QVariant();
}

void AbstractInspectorModel::onResetToDefaults(const QList<Ms::Pid>& pidList)
{
    if (isEmpty()) {
        return;
    }

    beginCommand();

    for (Ms::EngravingItem* element : m_elementList) {
        IF_ASSERT_FAILED(element) {
            continue;
        }

        for (const Ms::Pid pid : pidList) {
            element->elementBase()->undoResetProperty(pid);
        }
    }

    endCommand();
    updateNotation();

    emit elementsModified();
    emit modelReseted();
}

QVariant AbstractInspectorModel::valueToElementUnits(const Ms::Pid& pid, const QVariant& value,
                                                     const Ms::EngravingItem* element) const
{
    switch (Ms::propertyType(pid)) {
    case Ms::P_TYPE::POINT_SP:
        return value.value<PointF>() * element->spatium();

    case Ms::P_TYPE::POINT_SP_MM: {
        if (element->sizeIsSpatiumDependent()) {
            return value.value<PointF>() * element->spatium();
        } else {
            return value.value<PointF>() * Ms::DPMM;
        }
    }

    case Ms::P_TYPE::SP_REAL:
        return value.toDouble() * element->spatium();

    case Ms::P_TYPE::SPATIUM:
        return value.value<Ms::Spatium>().val();

    case Ms::P_TYPE::TEMPO:
        return value.toDouble() / 60.0;

    case Ms::P_TYPE::ZERO_INT:
        return value.toInt() - 1;

    case Ms::P_TYPE::POINT_MM:
        return value.value<PointF>() * Ms::DPMM;

    case Ms::P_TYPE::SIZE_MM:
        return value.value<SizeF>() * Ms::DPMM;

    case Ms::P_TYPE::DIRECTION:
        return static_cast<int>(value.value<Ms::Direction>());

    case Ms::P_TYPE::INT_LIST: {
        QStringList strList;

        for (const int i : value.value<QList<int> >()) {
            strList << QString("%1").arg(i);
        }

        return strList.join(",");
    }

    default:
        if (value.type() == QVariant::Type::Color) {
            return QVariant::fromValue(mu::draw::Color(value.value<QColor>()));
        }
        return value;
    }
}

QVariant AbstractInspectorModel::valueFromElementUnits(const Ms::Pid& pid, const QVariant& value,
                                                       const Ms::EngravingItem* element) const
{
    switch (Ms::propertyType(pid)) {
    case Ms::P_TYPE::POINT_SP:
        return value.value<PointF>() / element->spatium();

    case Ms::P_TYPE::POINT_SP_MM: {
        if (element->sizeIsSpatiumDependent()) {
            return value.value<PointF>() / element->spatium();
        } else {
            return value.value<PointF>() / Ms::DPMM;
        }
    }

    case Ms::P_TYPE::SP_REAL:
        return value.toDouble() / element->spatium();

    case Ms::P_TYPE::SPATIUM:
        return value.value<Ms::Spatium>().val();

    case Ms::P_TYPE::TEMPO:
        return value.toDouble() * 60.0;

    case Ms::P_TYPE::ZERO_INT:
        return value.toInt() + 1;

    case Ms::P_TYPE::POINT_MM:
        return value.value<PointF>() / Ms::DPMM;

    case Ms::P_TYPE::SIZE_MM:
        return value.value<SizeF>() / Ms::DPMM;

    case Ms::P_TYPE::DIRECTION:
        return static_cast<int>(value.value<Ms::Direction>());

    case Ms::P_TYPE::INT_LIST: {
        QStringList strList;

        for (const int i : value.value<QList<int> >()) {
            strList << QString("%1").arg(i);
        }

        return strList.join(",");
    }
    case Ms::P_TYPE::COLOR:
        return value.value<mu::draw::Color>().toQColor();
    default:
        return value;
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

bool AbstractInspectorModel::hasAcceptableElements() const
{
    return !m_elementList.isEmpty();
}

INotationStylePtr AbstractInspectorModel::style() const
{
    if (!context() || !context()->currentNotation()) {
        return nullptr;
    }

    return context()->currentNotation()->style();
}

INotationUndoStackPtr AbstractInspectorModel::undoStack() const
{
    if (!context() || !context()->currentNotation()) {
        return nullptr;
    }

    return context()->currentNotation()->undoStack();
}

void AbstractInspectorModel::beginCommand()
{
    if (undoStack()) {
        undoStack()->prepareChanges();
    }
}

void AbstractInspectorModel::endCommand()
{
    if (undoStack()) {
        undoStack()->commitChanges();
    }
}

mu::async::Notification AbstractInspectorModel::currentNotationChanged() const
{
    IF_ASSERT_FAILED(context()) {
        return mu::async::Notification();
    }

    return context()->currentNotationChanged();
}

void AbstractInspectorModel::updateNotation()
{
    IF_ASSERT_FAILED(context()) {
        return;
    }

    if (!context()->currentNotation()) {
        return;
    }

    context()->currentNotation()->notationChanged().notify();
}
