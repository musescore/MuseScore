#include "footnotesettingsmodel.h"

#include "translation.h"

using namespace mu::inspector;

FootnoteSettingsModel::FootnoteSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : InspectorModelWithVoiceAndPositionOptions(parent, repository)
{
    setModelType(InspectorModelType::TYPE_FOOTNOTE);
    setTitle(muse::qtrc("inspector ", "Footnote"));
    setIcon(muse::ui::IconCode::Code::EXPRESSION);
    createProperties();
}

void FootnoteSettingsModel::createProperties()
{
    InspectorModelWithVoiceAndPositionOptions::createProperties();
}

void FootnoteSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::FOOTNOTE);
}

void FootnoteSettingsModel::loadProperties()
{
    InspectorModelWithVoiceAndPositionOptions::loadProperties();
}

void FootnoteSettingsModel::resetProperties()
{
    InspectorModelWithVoiceAndPositionOptions::resetProperties();
}
