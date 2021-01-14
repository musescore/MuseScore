#ifndef MU_INSPECTOR_CHORDSYMBOLSETTINGSMODEL_H
#define MU_INSPECTOR_CHORDSYMBOLSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class ChordSymbolSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * isLiteral READ isLiteral CONSTANT)
    Q_PROPERTY(PropertyItem * voicingType READ voicingType CONSTANT)
    Q_PROPERTY(PropertyItem * durationType READ durationType CONSTANT)

public:
    explicit ChordSymbolSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* isLiteral() const;
    PropertyItem* voicingType() const;
    PropertyItem* durationType() const;

private:
    PropertyItem* m_isLiteral = nullptr;
    PropertyItem* m_voicingType = nullptr;
    PropertyItem* m_durationType = nullptr;
};
}

#endif // MU_INSPECTOR_CHORDSYMBOLSETTINGSMODEL_H
