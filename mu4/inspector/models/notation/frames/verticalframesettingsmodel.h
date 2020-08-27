#ifndef VERTICALFRAMESETTINGSMODEL_H
#define VERTICALFRAMESETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

class VerticalFrameSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * frameHeight READ frameHeight CONSTANT)
    Q_PROPERTY(PropertyItem * gapAbove READ gapAbove CONSTANT)
    Q_PROPERTY(PropertyItem * gapBelow READ gapBelow CONSTANT)
    Q_PROPERTY(PropertyItem * frameLeftMargin READ frameLeftMargin CONSTANT)
    Q_PROPERTY(PropertyItem * frameRightMargin READ frameRightMargin CONSTANT)
    Q_PROPERTY(PropertyItem * frameTopMargin READ frameTopMargin CONSTANT)
    Q_PROPERTY(PropertyItem * frameBottomMargin READ frameBottomMargin CONSTANT)

public:
    explicit VerticalFrameSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* frameHeight() const;
    PropertyItem* gapAbove() const;
    PropertyItem* gapBelow() const;
    PropertyItem* frameLeftMargin() const;
    PropertyItem* frameRightMargin() const;
    PropertyItem* frameTopMargin() const;
    PropertyItem* frameBottomMargin() const;

private:
    PropertyItem* m_frameHeight = nullptr;
    PropertyItem* m_gapAbove = nullptr;
    PropertyItem* m_gapBelow = nullptr;
    PropertyItem* m_frameLeftMargin = nullptr;
    PropertyItem* m_frameRightMargin = nullptr;
    PropertyItem* m_frameTopMargin = nullptr;
    PropertyItem* m_frameBottomMargin = nullptr;
};

#endif // VERTICALFRAMESETTINGSMODEL_H
