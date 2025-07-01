#include "expressionsettingsmodel.h"

#include "translation.h"

using namespace mu::inspector;

ExpressionSettingsModel::ExpressionSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : InspectorModelWithVoiceAndPositionOptions(parent, repository)
{
    setModelType(InspectorModelType::TYPE_EXPRESSION);
    setTitle(muse::qtrc("inspector ", "Expression"));
    setIcon(muse::ui::IconCode::Code::EXPRESSION);
    createProperties();
}

void ExpressionSettingsModel::createProperties()
{
    InspectorModelWithVoiceAndPositionOptions::createProperties();

    m_snapExpression = buildPropertyItem(mu::engraving::Pid::SNAP_TO_DYNAMICS);
}

void ExpressionSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::EXPRESSION);
}

void ExpressionSettingsModel::loadProperties()
{
    InspectorModelWithVoiceAndPositionOptions::loadProperties();

    loadPropertyItem(m_snapExpression);
}

void ExpressionSettingsModel::resetProperties()
{
    InspectorModelWithVoiceAndPositionOptions::resetProperties();

    m_snapExpression->resetToDefault();
}

PropertyItem* ExpressionSettingsModel::snapExpression() const
{
    return m_snapExpression;
}
