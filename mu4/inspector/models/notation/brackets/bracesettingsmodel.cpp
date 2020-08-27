#include "bracesettingsmodel.h"

#include "log.h"
#include "libmscore/bracket.h"

BraceSettingsModel::BraceSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(TYPE_BRACE);
    setTitle(tr("Brace"));
    createProperties();
}

void BraceSettingsModel::createProperties()
{
    m_bracketColumnPosition = buildPropertyItem(Ms::Pid::BRACKET_COLUMN);
    m_bracketSpanStaves = buildPropertyItem(Ms::Pid::BRACKET_SPAN);
}

void BraceSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::BRACKET, [](const Ms::Element* element) -> bool {
        IF_ASSERT_FAILED(element) {
            return false;
        }

        const Ms::Bracket* bracket = Ms::toBracket(element);
        IF_ASSERT_FAILED(bracket) {
            return false;
        }

        return bracket->bracketType() == Ms::BracketType::BRACE;
    });
}

void BraceSettingsModel::loadProperties()
{
    loadPropertyItem(m_bracketColumnPosition);
    loadPropertyItem(m_bracketSpanStaves);
}

void BraceSettingsModel::resetProperties()
{
    m_bracketColumnPosition->resetToDefault();
    m_bracketSpanStaves->resetToDefault();
}

PropertyItem* BraceSettingsModel::bracketColumnPosition() const
{
    return m_bracketColumnPosition;
}

PropertyItem* BraceSettingsModel::bracketSpanStaves() const
{
    return m_bracketSpanStaves;
}
