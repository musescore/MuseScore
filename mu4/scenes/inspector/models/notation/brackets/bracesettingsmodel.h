#ifndef BRACESETTINGSMODEL_H
#define BRACESETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

class BraceSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * bracketColumnPosition READ bracketColumnPosition CONSTANT)
    Q_PROPERTY(PropertyItem * bracketSpanStaves READ bracketSpanStaves CONSTANT)

public:

    explicit BraceSettingsModel(QObject* parent, IElementRepositoryService* repository);

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

#endif // BRACESETTINGSMODEL_H
