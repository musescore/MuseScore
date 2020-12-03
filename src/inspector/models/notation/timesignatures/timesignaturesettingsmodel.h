#ifndef TIMESIGNATURESETTINGSMODEL_H
#define TIMESIGNATURESETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

class TimeSignatureSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * horizontalScale READ horizontalScale CONSTANT)
    Q_PROPERTY(PropertyItem * verticalScale READ verticalScale CONSTANT)
    Q_PROPERTY(PropertyItem * shouldShowCourtesy READ shouldShowCourtesy CONSTANT)

public:
    explicit TimeSignatureSettingsModel(QObject* parent, IElementRepositoryService* repository);

    Q_INVOKABLE void showTimeSignatureProperties();

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* horizontalScale() const;
    PropertyItem* verticalScale() const;
    PropertyItem* shouldShowCourtesy() const;

private:

    PropertyItem* m_horizontalScale = nullptr;
    PropertyItem* m_verticalScale = nullptr;
    PropertyItem* m_shouldShowCourtesy = nullptr;
};

#endif // TIMESIGNATURESETTINGSMODEL_H
