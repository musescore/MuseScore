#ifndef BRACKETSETTINGSMODEL_H
#define BRACKETSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

class BracketSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * bracketColumnPosition READ bracketColumnPosition CONSTANT)
    Q_PROPERTY(PropertyItem * bracketSpanStaves READ bracketSpanStaves CONSTANT)

public:

    explicit BracketSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* bracketColumnPosition() const;
    PropertyItem* bracketSpanStaves() const;

private:
    PropertyItem* m_bracketColumnPosition = nullptr;
    PropertyItem* m_bracketSpanStaves = nullptr;
};

#endif // BRACKETSETTINGSMODEL_H
