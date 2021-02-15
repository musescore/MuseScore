#ifndef MU_INSPECTOR_SECTIONBREAKSETTINGSMODEL_H
#define MU_INSPECTOR_SECTIONBREAKSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class SectionBreakSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * shouldStartWithLongInstrNames READ shouldStartWithLongInstrNames CONSTANT)
    Q_PROPERTY(PropertyItem * shouldResetBarNums READ shouldResetBarNums CONSTANT)
    Q_PROPERTY(PropertyItem * pauseDuration READ pauseDuration CONSTANT)

public:
    explicit SectionBreakSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* shouldStartWithLongInstrNames() const;
    PropertyItem* shouldResetBarNums() const;
    PropertyItem* pauseDuration() const;

private:
    PropertyItem* m_shouldStartWithLongInstrNames = nullptr;
    PropertyItem* m_shouldResetBarNums = nullptr;
    PropertyItem* m_pauseDuration = nullptr;
};
}

#endif // MU_INSPECTOR_SECTIONBREAKSETTINGSMODEL_H
