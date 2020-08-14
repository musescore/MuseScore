#ifndef SCOREAPPEARANCESETTINGSMODEL_H
#define SCOREAPPEARANCESETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

#include "internal/pagetypelistmodel.h"

class ScoreAppearanceSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PageTypeListModel * pageTypeListModel READ pageTypeListModel CONSTANT)
    Q_PROPERTY(int orientationType READ orientationType WRITE setOrientationType NOTIFY orientationTypeChanged)
    Q_PROPERTY(qreal staffSpacing READ staffSpacing WRITE setStaffSpacing NOTIFY staffSpacingChanged)
    Q_PROPERTY(qreal staffDistance READ staffDistance WRITE setStaffDistance NOTIFY staffDistanceChanged)

public:
    explicit ScoreAppearanceSettingsModel(QObject* parent, IElementRepositoryService* repository);

    Q_INVOKABLE void showPageSettings();
    Q_INVOKABLE void showStyleSettings();

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    bool hasAcceptableElements() const override;

    PageTypeListModel* pageTypeListModel() const;

    int orientationType() const;
    qreal staffSpacing() const;
    qreal staffDistance() const;

public slots:
    void setPageTypeListModel(PageTypeListModel* pageTypeListModel);
    void setOrientationType(int orientationType);
    void setStaffSpacing(qreal staffSpacing);
    void setStaffDistance(qreal staffDistance);

signals:
    void orientationTypeChanged(int orientationType);
    void staffSpacingChanged(qreal staffSpacing);
    void staffDistanceChanged(qreal staffDistance);

private:
    void updatePageSize();

    int m_orientationType = 0;
    qreal m_staffSpacing = 0.0;
    qreal m_staffDistance = 0.0;

    PageTypeListModel* m_pageTypeListModel = nullptr;
};

#endif // SCOREAPPEARANCESETTINGSMODEL_H
