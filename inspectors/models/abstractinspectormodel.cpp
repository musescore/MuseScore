#include "abstractinspectormodel.h"
#include "global/log.h"

AbstractInspectorModel::AbstractInspectorModel(QObject* parent, IElementRepositoryService* repository)
    : QObject(parent)
{
    m_repository = repository;

    if (!m_repository)
        return;

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

AbstractInspectorModel::InspectorSectionType AbstractInspectorModel::modelTypeFromElementType(const Ms::ElementType elementType)
{
    switch (elementType) {
    case Ms::ElementType::NOTE:
    case Ms::ElementType::STEM:
    case Ms::ElementType::NOTEDOT:
    case Ms::ElementType::NOTEHEAD:
    case Ms::ElementType::NOTELINE:
    case Ms::ElementType::SHADOW_NOTE:
    case Ms::ElementType::HOOK:
    case Ms::ElementType::BEAM:
    case Ms::ElementType::GLISSANDO:
    case Ms::ElementType::GLISSANDO_SEGMENT:
    case Ms::ElementType::TEMPO_TEXT:
    case Ms::ElementType::FERMATA:
    case Ms::ElementType::LAYOUT_BREAK:
    case Ms::ElementType::BAR_LINE:
    case Ms::ElementType::MARKER:
    case Ms::ElementType::JUMP:
    case Ms::ElementType::KEYSIG:
    case Ms::ElementType::ACCIDENTAL:
    case Ms::ElementType::FRET_DIAGRAM:
    case Ms::ElementType::SPACER:
        return SECTION_NOTATION;

    default:
        return SECTION_UNDEFINED;
    }
}

bool AbstractInspectorModel::isEmpty() const
{
    return m_isEmpty;
}

void AbstractInspectorModel::setTitle(QString title)
{
    if (m_title == title)
        return;

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
    if (!hasAcceptableElements())
        return;

    Ms::Score* score  = parentScore();
    IF_ASSERT_FAILED(score) {
        return;
    }
    
    score->startCmd();

    QVariant convertedValue;

    for (Ms::Element* element : m_elementList) {
        IF_ASSERT_FAILED(element) {
            continue;
        }
        
        Ms::PropertyFlags ps = element->propertyFlags(pid);

        if (ps == Ms::PropertyFlags::STYLED)
            ps = Ms::PropertyFlags::UNSTYLED;

        convertedValue = valueToElementUnits(pid, newValue, element);

        element->undoChangeProperty(pid, convertedValue, ps);
        element->triggerLayout();
    }

    score->endCmd(true /*isCmdFromInspector*/);

    emit elementsModified();
}

void AbstractInspectorModel::setIsEmpty(bool isEmpty)
{
    if (m_isEmpty == isEmpty)
        return;

    m_isEmpty = isEmpty;
    emit isEmptyChanged(m_isEmpty);
}

void AbstractInspectorModel::updateProperties()
{
    requestElements();

    setIsEmpty(!hasAcceptableElements());

    if (!isEmpty()) {
        loadProperties();
    } else {
        resetProperties();
    }
}

void AbstractInspectorModel::onResetToDefaults(const QList<Ms::Pid>& pidList)
{
    if (isEmpty())
        return;

    Ms::Score* score  = parentScore();
    IF_ASSERT_FAILED(score) {
        return;
    }

    score->startCmd();

    for (Ms::Element* element : m_elementList) {
        IF_ASSERT_FAILED(element) {
            continue;
        }
        
        for (const Ms::Pid pid : pidList)
            element->elementBase()->undoResetProperty(pid);
    }

    score->endCmd();

    emit elementsModified();

    auto firstElement = m_elementList.at(0);
    IF_ASSERT_FAILED(firstElement) {
        return;
    }
    firstElement->triggerLayout();

    emit modelReseted();
}

QVariant AbstractInspectorModel::valueToElementUnits(const Ms::Pid& pid, const QVariant& value, const Ms::Element* element) const
{
    switch (Ms::propertyType(pid)) {
    case Ms::P_TYPE::POINT_SP:
        return value.toPointF() * element->spatium();

    case Ms::P_TYPE::POINT_SP_MM: {

        if (element->sizeIsSpatiumDependent())
            return value.toPointF() * element->spatium();
        else
            return value.toPointF() * Ms::DPMM;
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

        for (const int i : value.value<QList<int> >())
            strList << QString("%1").arg(i);

        return strList.join(",");
    }

    default:
        return value;
    }
}

QVariant AbstractInspectorModel::valueFromElementUnits(const Ms::Pid& pid, const QVariant& value, const Ms::Element* element) const
{
    switch (Ms::propertyType(pid)) {

    case Ms::P_TYPE::POINT_SP:
        return value.toPointF() / element->spatium();

    case Ms::P_TYPE::POINT_SP_MM: {

        if (element->sizeIsSpatiumDependent())
            return value.toPointF() / element->spatium();
        else
            return value.toPointF() / Ms::DPMM;
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

        for (const int i : value.value<QList<int> >())
            strList << QString("%1").arg(i);

        return strList.join(",");
    }

    default:
        return value;
    }
}

PropertyItem* AbstractInspectorModel::buildPropertyItem(const Ms::Pid& pid, std::function<void(const int propertyId, const QVariant& newValue)> onPropertyChangedCallBack)
{
    PropertyItem* newPropertyItem = new PropertyItem(static_cast<int>(pid), this);

    auto callback = onPropertyChangedCallBack;

    if (!callback) {
        callback = [this] (const int propertyId, const QVariant& newValue) {
            onPropertyValueChanged(static_cast<Ms::Pid>(propertyId), newValue);
        };
    }

    connect(newPropertyItem, &PropertyItem::propertyModified, this, callback);

    return newPropertyItem;
}

void AbstractInspectorModel::loadPropertyItem(PropertyItem* propertyItem, std::function<QVariant(const QVariant&)> convertElementPropertyValueFunc)
{
    if (!propertyItem || m_elementList.isEmpty())
        return;

    Ms::Pid pid = static_cast<Ms::Pid>(propertyItem->propertyId());

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

        if (isUndefined)
            break;
    }

    //@note Some elements may support the property, some don't. If element doesn't support property it'll return invalid value.
    //      So we use that knowledge here
    propertyItem->setIsEnabled(propertyValue.isValid());

    if (isUndefined) {
        propertyValue = QVariant();
    }

    propertyItem->fillValues(propertyValue, defaultPropertyValue);
}

bool AbstractInspectorModel::hasAcceptableElements() const
{
    return !m_elementList.isEmpty();
}

Ms::Score* AbstractInspectorModel::parentScore() const
{
    if (m_elementList.isEmpty()) {
        return nullptr;
    }

    auto firstElement = m_elementList.at(0);
    IF_ASSERT_FAILED(firstElement) {
        return nullptr;
    }
    
    return firstElement->score();
}
