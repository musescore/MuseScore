#ifndef HORIZONTALFRAMESETTINGSMODEL_H
#define HORIZONTALFRAMESETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

class HorizontalFrameSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * frameWidth READ frameWidth CONSTANT)
    Q_PROPERTY(PropertyItem * leftGap READ leftGap CONSTANT)
    Q_PROPERTY(PropertyItem * rightGap READ rightGap CONSTANT)
    Q_PROPERTY(PropertyItem * shouldDisplayKeysAndBrackets READ shouldDisplayKeysAndBrackets CONSTANT)

public:
    explicit HorizontalFrameSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* frameWidth() const;
    PropertyItem* leftGap() const;
    PropertyItem* rightGap() const;
    PropertyItem* shouldDisplayKeysAndBrackets() const;

private:
    PropertyItem* m_frameWidth = nullptr;
    PropertyItem* m_leftGap = nullptr;
    PropertyItem* m_rightGap = nullptr;
    PropertyItem* m_shouldDisplayKeysAndBrackets = nullptr;
};

#endif // HORIZONTALFRAMESETTINGSMODEL_H
