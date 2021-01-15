#include "bracketsettingsmodel.h"

#include "log.h"
#include "libmscore/bracket.h"

#include "translation.h"

using namespace mu::inspector;

BracketSettingsModel::BracketSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_BRACKET);
    setTitle(qtrc("inspector", "Bracket"));
    createProperties();
}

void BracketSettingsModel::createProperties()
{
    m_bracketColumnPosition = buildPropertyItem(Ms::Pid::BRACKET_COLUMN);
    m_bracketSpanStaves = buildPropertyItem(Ms::Pid::BRACKET_SPAN);
}

void BracketSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::BRACKET, [](const Ms::Element* element) -> bool {
        IF_ASSERT_FAILED(element) {
            return false;
        }

        const Ms::Bracket* bracket = Ms::toBracket(element);
        IF_ASSERT_FAILED(bracket) {
            return false;
        }

        return bracket->bracketType() != Ms::BracketType::BRACE;
    });
}

void BracketSettingsModel::loadProperties()
{
    loadPropertyItem(m_bracketColumnPosition);
    loadPropertyItem(m_bracketSpanStaves);
}

void BracketSettingsModel::resetProperties()
{
    m_bracketColumnPosition->resetToDefault();
    m_bracketSpanStaves->resetToDefault();
}

PropertyItem* BracketSettingsModel::bracketColumnPosition() const
{
    return m_bracketColumnPosition;
}

PropertyItem* BracketSettingsModel::bracketSpanStaves() const
{
    return m_bracketSpanStaves;
}
