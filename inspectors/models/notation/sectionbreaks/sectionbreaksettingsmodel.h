#ifndef SECTIONBREAKSETTINGSMODEL_H
#define SECTIONBREAKSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

class SectionBreakSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem* startWithLongInstrNames READ startWithLongInstrNames CONSTANT)
    Q_PROPERTY(PropertyItem* resetBarNums READ resetBarNums CONSTANT)
    Q_PROPERTY(PropertyItem* pause READ pause CONSTANT)

public:
    explicit SectionBreakSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* startWithLongInstrNames() const;
    PropertyItem* resetBarNums() const;
    PropertyItem* pause() const;

private:
    PropertyItem* m_startWithLongInstrNames = nullptr;
    PropertyItem* m_resetBarNums = nullptr;
    PropertyItem* m_pause = nullptr;
};

#endif // SECTIONBREAKSETTINGSMODEL_H
