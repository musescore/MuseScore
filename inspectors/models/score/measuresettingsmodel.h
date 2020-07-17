#ifndef BARSSETTINGSMODEL_H
#define BARSSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"
#include "types/measuretypes.h"

class MeasureSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(int measureCount READ measureCount WRITE setMeasureCount NOTIFY measureCountChanged)
    Q_PROPERTY(MeasureTypes::MeasureInsertionType measureInsertionType READ measureInsertionType WRITE setMeasureInsertionType NOTIFY measureInsertionTypeChanged)

public:
    explicit MeasureSettingsModel(QObject* parent, IElementRepositoryService* repository);

    Q_INVOKABLE void insertMeasures();
    Q_INVOKABLE void removeSelectedMeasures();

    void createProperties() override {}
    void requestElements() override;
    void loadProperties() override {}
    void resetProperties() override {}

    int measureCount() const;
    MeasureTypes::MeasureInsertionType measureInsertionType() const;

public slots:
    void setMeasureCount(int measureCount);
    void setMeasureInsertionType(MeasureTypes::MeasureInsertionType measureInsertionType);

signals:
    void measureCountChanged(int measureCount);
    void measureInsertionTypeChanged(MeasureTypes::MeasureInsertionType measureInsertionType);

private:
    int m_measureCount = 0;
    MeasureTypes::MeasureInsertionType m_barInsertionType = MeasureTypes::MeasureInsertionType::TYPE_PREPEND_TO_SCORE;
};

#endif // BARSSETTINGSMODEL_H
