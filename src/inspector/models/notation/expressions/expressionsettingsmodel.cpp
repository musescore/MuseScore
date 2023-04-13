#include "expressionsettingsmodel.h"

#include "translation.h"

using namespace mu::inspector;

ExpressionSettingsModel::ExpressionSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_EXPRESSION);
    setTitle(qtrc("inspector ", "Expression"));
    createProperties();
}

void ExpressionSettingsModel::createProperties()
{
    m_snapExpression = buildPropertyItem(mu::engraving::Pid::SNAP_TO_DYNAMICS);
}

void ExpressionSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::EXPRESSION);
}

void ExpressionSettingsModel::loadProperties()
{
    loadPropertyItem(m_snapExpression);
}

void ExpressionSettingsModel::resetProperties()
{
    m_snapExpression->resetToDefault();
}

PropertyItem* ExpressionSettingsModel::snapExpression() const
{
    return m_snapExpression;
}
