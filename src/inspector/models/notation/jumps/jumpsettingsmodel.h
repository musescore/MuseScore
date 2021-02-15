#ifndef MU_INSPECTOR_JUMPSETTINGSMODEL_H
#define MU_INSPECTOR_JUMPSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class JumpSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * jumpTo READ jumpTo CONSTANT)
    Q_PROPERTY(PropertyItem * playUntil READ playUntil CONSTANT)
    Q_PROPERTY(PropertyItem * continueAt READ continueAt CONSTANT)
    Q_PROPERTY(PropertyItem * hasToPlayRepeats READ hasToPlayRepeats CONSTANT)
public:
    explicit JumpSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* jumpTo() const;
    PropertyItem* playUntil() const;
    PropertyItem* continueAt() const;
    PropertyItem* hasToPlayRepeats() const;

private:
    PropertyItem* m_jumpTo = nullptr;
    PropertyItem* m_playUntil = nullptr;
    PropertyItem* m_continueAt = nullptr;
    PropertyItem* m_hasToPlayRepeats = nullptr;
};
}

#endif // MU_INSPECTOR_JUMPSETTINGSMODEL_H
