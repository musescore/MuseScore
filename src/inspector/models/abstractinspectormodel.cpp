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

using namespace mu::inspector;
using namespace mu::notation;

static const QMap<Ms::ElementType, AbstractInspectorModel::InspectorModelType> NOTATION_ELEMENT_MODEL_TYPES = {
    { Ms::ElementType::NOTE, AbstractInspectorModel::InspectorModelType::TYPE_NOTE },
    { Ms::ElementType::STEM, AbstractInspectorModel::InspectorModelType::TYPE_STEM },
    { Ms::ElementType::NOTEDOT, AbstractInspectorModel::InspectorModelType::TYPE_NOTE },
    { Ms::ElementType::NOTEHEAD, AbstractInspectorModel::InspectorModelType::TYPE_NOTEHEAD },
    { Ms::ElementType::NOTELINE, AbstractInspectorModel::InspectorModelType::TYPE_NOTE },
    { Ms::ElementType::SHADOW_NOTE, AbstractInspectorModel::InspectorModelType::TYPE_NOTE },
    { Ms::ElementType::HOOK, AbstractInspectorModel::InspectorModelType::TYPE_HOOK },
    { Ms::ElementType::BEAM, AbstractInspectorModel::InspectorModelType::TYPE_BEAM },
    { Ms::ElementType::GLISSANDO, AbstractInspectorModel::InspectorModelType::TYPE_GLISSANDO },
    { Ms::ElementType::GLISSANDO_SEGMENT, AbstractInspectorModel::InspectorModelType::TYPE_GLISSANDO },
    { Ms::ElementType::TEMPO_TEXT, AbstractInspectorModel::InspectorModelType::TYPE_TEMPO },
    { Ms::ElementType::FERMATA, AbstractInspectorModel::InspectorModelType::TYPE_FERMATA },
    { Ms::ElementType::LAYOUT_BREAK, AbstractInspectorModel::InspectorModelType::TYPE_SECTIONBREAK },
    { Ms::ElementType::BAR_LINE, AbstractInspectorModel::InspectorModelType::TYPE_BARLINE },
    { Ms::ElementType::MARKER, AbstractInspectorModel::InspectorModelType::TYPE_MARKER },
    { Ms::ElementType::JUMP, AbstractInspectorModel::InspectorModelType::TYPE_JUMP },
    { Ms::ElementType::KEYSIG, AbstractInspectorModel::InspectorModelType::TYPE_KEYSIGNATURE },
    { Ms::ElementType::ACCIDENTAL, AbstractInspectorModel::InspectorModelType::TYPE_ACCIDENTAL },
    { Ms::ElementType::FRET_DIAGRAM, AbstractInspectorModel::InspectorModelType::TYPE_FRET_DIAGRAM },
    { Ms::ElementType::PEDAL, AbstractInspectorModel::InspectorModelType::TYPE_PEDAL },
    { Ms::ElementType::PEDAL_SEGMENT, AbstractInspectorModel::InspectorModelType::TYPE_PEDAL },
    { Ms::ElementType::SPACER, AbstractInspectorModel::InspectorModelType::TYPE_SPACER },
    { Ms::ElementType::CLEF, AbstractInspectorModel::InspectorModelType::TYPE_CLEF },
    { Ms::ElementType::HAIRPIN, AbstractInspectorModel::InspectorModelType::TYPE_HAIRPIN },
    { Ms::ElementType::HAIRPIN_SEGMENT, AbstractInspectorModel::InspectorModelType::TYPE_UNDEFINED },
    { Ms::ElementType::STAFFTYPE_CHANGE, AbstractInspectorModel::InspectorModelType::TYPE_STAFF_TYPE_CHANGES },
    { Ms::ElementType::TBOX, AbstractInspectorModel::InspectorModelType::TYPE_TEXT_FRAME },// text frame
    { Ms::ElementType::VBOX, AbstractInspectorModel::InspectorModelType::TYPE_VERTICAL_FRAME },// vertical frame
    { Ms::ElementType::HBOX, AbstractInspectorModel::InspectorModelType::TYPE_HORIZONTAL_FRAME },// horizontal frame
    { Ms::ElementType::ARTICULATION, AbstractInspectorModel::InspectorModelType::TYPE_ARTICULATION },
    { Ms::ElementType::IMAGE, AbstractInspectorModel::InspectorModelType::TYPE_IMAGE },
    { Ms::ElementType::HARMONY, AbstractInspectorModel::InspectorModelType::TYPE_CHORD_SYMBOL },
    { Ms::ElementType::AMBITUS, AbstractInspectorModel::InspectorModelType::TYPE_AMBITUS },
    { Ms::ElementType::BRACKET, AbstractInspectorModel::InspectorModelType::TYPE_BRACKET },
    { Ms::ElementType::TIMESIG, AbstractInspectorModel::InspectorModelType::TYPE_TIME_SIGNATURE },
    { Ms::ElementType::MMREST, AbstractInspectorModel::InspectorModelType::TYPE_MMREST },
    { Ms::ElementType::BEND, AbstractInspectorModel::InspectorModelType::TYPE_BEND },
    { Ms::ElementType::TREMOLOBAR, AbstractInspectorModel::InspectorModelType::TYPE_TREMOLOBAR },
    { Ms::ElementType::TREMOLO, AbstractInspectorModel::InspectorModelType::TYPE_TREMOLOBAR },
    { Ms::ElementType::MEASURE_REPEAT, AbstractInspectorModel::InspectorModelType::TYPE_MEASURE_REPEAT }
};

static const QList<Ms::ElementType> TEXT_ELEMENT_TYPES = {
    Ms::ElementType::TEXT,
    Ms::ElementType::TEXTLINE,
    Ms::ElementType::TEXTLINE_BASE,
    Ms::ElementType::TEXTLINE_SEGMENT,
    Ms::ElementType::STAFF_TEXT,
    Ms::ElementType::SYSTEM_TEXT
};

AbstractInspectorModel::AbstractInspectorModel(QObject* parent, IElementRepositoryService* repository)
    : QObject(parent)
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

AbstractInspectorModel::InspectorModelType AbstractInspectorModel::modelType() const
{
    return m_modelType;
}

AbstractInspectorModel::InspectorSectionType AbstractInspectorModel::sectionTypeFromElementType(const Ms::ElementType elementType)
{
    if (NOTATION_ELEMENT_MODEL_TYPES.keys().contains(elementType)) {
        return InspectorSectionType::SECTION_NOTATION;
    } else if (TEXT_ELEMENT_TYPES.contains(elementType)) {
        return InspectorSectionType::SECTION_TEXT;
    }

    return InspectorSectionType::SECTION_UNDEFINED;
}

AbstractInspectorModel::InspectorModelType AbstractInspectorModel::notationElementModelType(const Ms::ElementType elementType)
{
    if (NOTATION_ELEMENT_MODEL_TYPES.contains(elementType)) {
        return NOTATION_ELEMENT_MODEL_TYPES.value(elementType);
    }

    return InspectorModelType::TYPE_UNDEFINED;
}

Ms::ElementType AbstractInspectorModel::elementType(const AbstractInspectorModel::InspectorModelType modelType)
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

QList<Ms::ElementType> AbstractInspectorModel::supportedElementTypesBySectionType(
    const AbstractInspectorModel::InspectorSectionType sectionType)
{
    switch (sectionType) {
    case InspectorSectionType::SECTION_GENERAL:
        return { Ms::ElementType::MAXTYPE };
    case InspectorSectionType::SECTION_NOTATION: {
        return NOTATION_ELEMENT_MODEL_TYPES.keys();
    }
    case InspectorSectionType::SECTION_TEXT: {
        return TEXT_ELEMENT_TYPES;
    }
    default:
        return QList<Ms::ElementType>();
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

void AbstractInspectorModel::setModelType(AbstractInspectorModel::InspectorModelType modelType)
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

    for (Ms::Element* element : m_elementList) {
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

Ms::Sid AbstractInspectorModel::styleIdByPropertyId(const Ms::Pid pid) const
{
    Ms::Sid result = Ms::Sid::NOSTYLE;

    for (const Ms::Element* element : m_elementList) {
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

    for (Ms::Element* element : m_elementList) {
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
                                                     const Ms::Element* element) const
{
    switch (Ms::propertyType(pid)) {
    case Ms::P_TYPE::POINT_SP:
        return value.toPointF() * element->spatium();

    case Ms::P_TYPE::POINT_SP_MM: {
        if (element->sizeIsSpatiumDependent()) {
            return value.toPointF() * element->spatium();
        } else {
            return value.toPointF() * Ms::DPMM;
        }
    }

    case Ms::P_TYPE::SP_REAL:
        return value.toDouble() * element->spatium();

    case Ms::P_TYPE::TEMPO:
        return value.toDouble() / 60.0;

    case Ms::P_TYPE::ZERO_INT:
        return value.toInt() - 1;

    case Ms::P_TYPE::POINT_MM:
        return value.toPointF() * Ms::DPMM;

    case Ms::P_TYPE::SIZE_MM:
        return value.toSizeF() * Ms::DPMM;

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
        return value;
    }
}

QVariant AbstractInspectorModel::valueFromElementUnits(const Ms::Pid& pid, const QVariant& value,
                                                       const Ms::Element* element) const
{
    switch (Ms::propertyType(pid)) {
    case Ms::P_TYPE::POINT_SP:
        return value.toPointF() / element->spatium();

    case Ms::P_TYPE::POINT_SP_MM: {
        if (element->sizeIsSpatiumDependent()) {
            return value.toPointF() / element->spatium();
        } else {
            return value.toPointF() / Ms::DPMM;
        }
    }

    case Ms::P_TYPE::SP_REAL:
        return value.toDouble() / element->spatium();

    case Ms::P_TYPE::TEMPO:
        return value.toDouble() * 60.0;

    case Ms::P_TYPE::ZERO_INT:
        return value.toInt() + 1;

    case Ms::P_TYPE::POINT_MM:
        return value.toPointF() / Ms::DPMM;

    case Ms::P_TYPE::SIZE_MM:
        return value.toSizeF() / Ms::DPMM;

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
        return value;
    }
}

PropertyItem* AbstractInspectorModel::buildPropertyItem(const Ms::Pid& propertyId, std::function<void(const int propertyId,
                                                                                                      const QVariant& newValue)> onPropertyChangedCallBack)
{
    PropertyItem* newPropertyItem = new PropertyItem(static_cast<int>(propertyId), this);

    auto callback = onPropertyChangedCallBack;

    if (!callback) {
        callback = [this](const int propertyId, const QVariant& newValue) {
            onPropertyValueChanged(static_cast<Ms::Pid>(propertyId), newValue);
        };
    }

    connect(newPropertyItem, &PropertyItem::propertyModified, this, callback);
    connect(newPropertyItem, &PropertyItem::applyToStyleRequested, this, [this](const int sid, const QVariant& newStyleValue) {
        updateStyleValue(static_cast<Ms::Sid>(sid), newStyleValue);

        emit requestReloadPropertyItems();
    });

    return newPropertyItem;
}

void AbstractInspectorModel::loadPropertyItem(PropertyItem* propertyItem, std::function<QVariant(
                                                                                            const QVariant&)> convertElementPropertyValueFunc)
{
    if (!propertyItem || m_elementList.isEmpty()) {
        return;
    }

    Ms::Pid pid = static_cast<Ms::Pid>(propertyItem->propertyId());

    Ms::Sid styleId = styleIdByPropertyId(pid);
    propertyItem->setStyleId(static_cast<int>(styleId));
    propertyItem->setIsStyled(styleId != Ms::Sid::NOSTYLE);

    QVariant propertyValue;
    QVariant defaultPropertyValue;

    bool isUndefined = false;

    for (const Ms::Element* element : m_elementList) {
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
    return !context()->notationProjects().empty();
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
