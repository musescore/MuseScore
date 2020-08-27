#include "abstractinspectormodel.h"

#include "libmscore/musescoreCore.h"
#include "log.h"

static const QList<Ms::ElementType> NOTATION_ELEMENT_TYPES = {
    Ms::ElementType::NOTE,
    Ms::ElementType::STEM,
    Ms::ElementType::NOTEDOT,
    Ms::ElementType::NOTEHEAD,
    Ms::ElementType::NOTELINE,
    Ms::ElementType::SHADOW_NOTE,
    Ms::ElementType::HOOK,
    Ms::ElementType::BEAM,
    Ms::ElementType::GLISSANDO,
    Ms::ElementType::GLISSANDO_SEGMENT,
    Ms::ElementType::TEMPO_TEXT,
    Ms::ElementType::FERMATA,
    Ms::ElementType::LAYOUT_BREAK,
    Ms::ElementType::BAR_LINE,
    Ms::ElementType::MARKER,
    Ms::ElementType::JUMP,
    Ms::ElementType::KEYSIG,
    Ms::ElementType::ACCIDENTAL,
    Ms::ElementType::FRET_DIAGRAM,
    Ms::ElementType::PEDAL,
    Ms::ElementType::PEDAL_SEGMENT,
    Ms::ElementType::SPACER,
    Ms::ElementType::CLEF,
    Ms::ElementType::HAIRPIN,
    Ms::ElementType::HAIRPIN_SEGMENT,
    Ms::ElementType::STAFFTYPE_CHANGE,
    Ms::ElementType::TBOX, // text frame
    Ms::ElementType::VBOX, // vertical frame
    Ms::ElementType::HBOX, // horizontal frame
    Ms::ElementType::ARTICULATION,
    Ms::ElementType::IMAGE,
    Ms::ElementType::HARMONY,
    Ms::ElementType::AMBITUS,
    Ms::ElementType::BRACKET,
    Ms::ElementType::TIMESIG,
    Ms::ElementType::MMREST,
    Ms::ElementType::BEND,
    Ms::ElementType::TREMOLOBAR,
    Ms::ElementType::TREMOLO
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

AbstractInspectorModel::InspectorSectionType AbstractInspectorModel::sectionType() const
{
    return m_sectionType;
}

AbstractInspectorModel::InspectorModelType AbstractInspectorModel::modelType() const
{
    return m_modelType;
}

AbstractInspectorModel::InspectorSectionType AbstractInspectorModel::sectionTypeFromElementType(
    const Ms::ElementType elementType)
{
    if (NOTATION_ELEMENT_TYPES.contains(elementType)) {
        return SECTION_NOTATION;
    } else if (TEXT_ELEMENT_TYPES.contains(elementType)) {
        return SECTION_TEXT;
    }

    return SECTION_UNDEFINED;
}

bool AbstractInspectorModel::isEmpty() const
{
    return m_isEmpty;
}

QList<Ms::ElementType> AbstractInspectorModel::supportedElementTypesBySectionType(
    const AbstractInspectorModel::InspectorSectionType sectionType)
{
    switch (sectionType) {
    case SECTION_GENERAL:
        return { Ms::ElementType::MAXTYPE };
    case SECTION_NOTATION: {
        return NOTATION_ELEMENT_TYPES;
    }
    case SECTION_TEXT: {
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

    adapter()->beginCommand();

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

    adapter()->updateNotation();
    adapter()->endCommand();

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
    adapter()->beginCommand();
    adapter()->updateStyleValue(sid, newValue);
    adapter()->endCommand();
}

QVariant AbstractInspectorModel::styleValue(const Ms::Sid& sid) const
{
    return adapter()->styleValue(sid);
}

void AbstractInspectorModel::onResetToDefaults(const QList<Ms::Pid>& pidList)
{
    if (isEmpty()) {
        return;
    }

    adapter()->beginCommand();

    for (Ms::Element* element : m_elementList) {
        IF_ASSERT_FAILED(element) {
            continue;
        }

        for (const Ms::Pid pid : pidList) {
            element->elementBase()->undoResetProperty(pid);
        }
    }

    adapter()->endCommand();

    adapter()->updateNotation();

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
    return adapter()->isNotationExisting();
}

bool AbstractInspectorModel::hasAcceptableElements() const
{
    return !m_elementList.isEmpty();
}
