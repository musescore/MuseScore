#ifndef KEYSIGNATURESETTINGSMODEL_H
#define KEYSIGNATURESETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

class KeySignatureSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * hasToShowCourtesy READ hasToShowCourtesy CONSTANT)
    Q_PROPERTY(PropertyItem * mode READ mode CONSTANT)
public:
    explicit KeySignatureSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* hasToShowCourtesy() const;
    PropertyItem* mode() const;

private:
    PropertyItem* m_hasToShowCourtesy = nullptr;
    PropertyItem* m_mode = nullptr;
};

#endif // KEYSIGNATURESETTINGSMODEL_H
