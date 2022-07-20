#include "abstractelementpopupmodel.h"
#include "log.h"

using namespace mu::notation;

static const QMap<mu::engraving::ElementType, PopupModelType> ELEMENT_POPUP_TYPES = {
    { mu::engraving::ElementType::HARP_DIAGRAM, PopupModelType::TYPE_HARP_DIAGRAM },
};

AbstractElementPopupModel::AbstractElementPopupModel(QObject* parent, mu::engraving::ElementType elementType)
    : QObject(parent), m_elementType(elementType)
{
}

QString AbstractElementPopupModel::title() const
{
    LOGD() << "title: " << m_title;
    return m_title;
}

PopupModelType AbstractElementPopupModel::modelType() const
{
    return m_modelType;
}

PopupModelType AbstractElementPopupModel::modelTypeFromElement(const engraving::ElementType& elementType)
{
    return ELEMENT_POPUP_TYPES.value(elementType, PopupModelType::TYPE_UNDEFINED);
}

void AbstractElementPopupModel::setModelType(PopupModelType modelType)
{
    m_modelType = modelType;
}

void AbstractElementPopupModel::setTitle(QString title)
{
    if (m_title == title) {
        return;
    }

    m_title = title;
    emit titleChanged();
}

void AbstractElementPopupModel::setElementType(engraving::ElementType type)
{
    m_elementType = type;
}
