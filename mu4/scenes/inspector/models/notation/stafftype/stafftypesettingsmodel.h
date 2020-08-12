#ifndef STAFFTYPESETTINGSMODEL_H
#define STAFFTYPESETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

class StaffTypeSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * isSmall READ isSmall CONSTANT)
    Q_PROPERTY(PropertyItem * verticalOffset READ verticalOffset CONSTANT)
    Q_PROPERTY(PropertyItem * scale READ scale CONSTANT)

    Q_PROPERTY(PropertyItem * lineCount READ lineCount CONSTANT)
    Q_PROPERTY(PropertyItem * lineDistance READ lineDistance CONSTANT)
    Q_PROPERTY(PropertyItem * stepOffset READ stepOffset CONSTANT)

    Q_PROPERTY(PropertyItem * noteheadSchemeType READ noteheadSchemeType CONSTANT)
    Q_PROPERTY(PropertyItem * isStemless READ isStemless CONSTANT)
    Q_PROPERTY(PropertyItem * shouldShowBarlines READ shouldShowBarlines CONSTANT)
    Q_PROPERTY(PropertyItem * shouldShowLedgerLines READ shouldShowLedgerLines CONSTANT)
    Q_PROPERTY(PropertyItem * shouldGenerateClefs READ shouldGenerateClefs CONSTANT)
    Q_PROPERTY(PropertyItem * shouldGenerateTimeSignatures READ shouldGenerateTimeSignatures CONSTANT)
    Q_PROPERTY(PropertyItem * shouldGenerateKeySignatures READ shouldGenerateKeySignatures CONSTANT)
public:
    explicit StaffTypeSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* isSmall() const;
    PropertyItem* verticalOffset() const;
    PropertyItem* scale() const;

    PropertyItem* lineCount() const;
    PropertyItem* lineDistance() const;
    PropertyItem* stepOffset() const;

    PropertyItem* noteheadSchemeType() const;
    PropertyItem* isStemless() const;
    PropertyItem* shouldShowBarlines() const;
    PropertyItem* shouldShowLedgerLines() const;
    PropertyItem* shouldGenerateClefs() const;
    PropertyItem* shouldGenerateTimeSignatures() const;
    PropertyItem* shouldGenerateKeySignatures() const;

private:
    PropertyItem* m_isSmall = nullptr;
    PropertyItem* m_verticalOffset = nullptr;
    PropertyItem* m_scale = nullptr;

    PropertyItem* m_lineCount = nullptr;
    PropertyItem* m_lineDistance = nullptr;
    PropertyItem* m_stepOffset = nullptr;

    PropertyItem* m_noteheadSchemeType = nullptr;
    PropertyItem* m_isStemless = nullptr;
    PropertyItem* m_shouldShowBarlines = nullptr;
    PropertyItem* m_shouldShowLedgerLines = nullptr;
    PropertyItem* m_shouldGenerateClefs = nullptr;
    PropertyItem* m_shouldGenerateTimeSignatures = nullptr;
    PropertyItem* m_shouldGenerateKeySignatures = nullptr;
};

#endif // STAFFTYPESETTINGSMODEL_H
