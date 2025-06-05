#ifndef FOOTNOTESETTINGSMODEL_H
#define FOOTNOTESETTINGSMODEL_H

#include "models/inspectormodelwithvoiceandpositionoptions.h"

namespace mu::inspector {
class FootnoteSettingsModel : public InspectorModelWithVoiceAndPositionOptions
{
    Q_OBJECT

public:
    explicit FootnoteSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;
};
}

#endif // FOOTNOTESETTINGSMODEL_H
