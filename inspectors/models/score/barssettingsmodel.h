#ifndef BARSSETTINGSMODEL_H
#define BARSSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"
#include "types/bartypes.h"

class BarsSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(int barCount READ barCount WRITE setBarCount NOTIFY barCountChanged)
    Q_PROPERTY(BarTypes::BarInsertionType barInsertionType READ barInsertionType WRITE setBarInsertionType NOTIFY barInsertionTypeChanged)

public:
    explicit BarsSettingsModel(QObject* parent, IElementRepositoryService* repository);

    Q_INVOKABLE void insertBars();
    Q_INVOKABLE void removeSelectedBars();

    void createProperties() override {}
    void requestElements() override;
    void loadProperties() override {}
    void resetProperties() override {}

    int barCount() const;
    BarTypes::BarInsertionType barInsertionType() const;

public slots:
    void setBarCount(int barCount);
    void setBarInsertionType(BarTypes::BarInsertionType barInsertionType);

signals:
    void barCountChanged(int barCount);
    void barInsertionTypeChanged(BarTypes::BarInsertionType barInsertionType);

private:
    int m_barCount = 0;
    BarTypes::BarInsertionType m_barInsertionType = BarTypes::BarInsertionType::TYPE_PREPEND_TO_SCORE;
};

#endif // BARSSETTINGSMODEL_H
