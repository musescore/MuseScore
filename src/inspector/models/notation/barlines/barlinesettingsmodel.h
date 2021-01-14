#ifndef MU_INSPECTOR_BARLINESETTINGSMODEL_H
#define MU_INSPECTOR_BARLINESETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class BarlineSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * type READ type CONSTANT)
    Q_PROPERTY(PropertyItem * isSpanToNextStaff READ isSpanToNextStaff CONSTANT)
    Q_PROPERTY(PropertyItem * spanFrom READ spanFrom CONSTANT)
    Q_PROPERTY(PropertyItem * spanTo READ spanTo CONSTANT)
    Q_PROPERTY(PropertyItem * hasToShowTips READ hasToShowTips CONSTANT)

public:
    explicit BarlineSettingsModel(QObject* parent, IElementRepositoryService* repository);

    Q_INVOKABLE void applyToAllStaffs();
    Q_INVOKABLE void applySpanPreset(const int presetType);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* type() const;
    PropertyItem* isSpanToNextStaff() const;
    PropertyItem* spanFrom() const;
    PropertyItem* spanTo() const;
    PropertyItem* hasToShowTips() const;

private:
    PropertyItem* m_type = nullptr;
    PropertyItem* m_isSpanToNextStaff = nullptr;
    PropertyItem* m_spanFrom = nullptr;
    PropertyItem* m_spanTo = nullptr;
    PropertyItem* m_hasToShowTips = nullptr;
};
}

#endif // MU_INSPECTOR_BARLINESETTINGSMODEL_H
