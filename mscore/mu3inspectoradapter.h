#ifndef MU3INSPECTORADAPTER_H
#define MU3INSPECTORADAPTER_H

#include "modularity/ioc.h"

#include "iinspectoradapter.h"

class MU3InspectorAdapter : public mu::inspector::IInspectorAdapter
{
public:
    MU3InspectorAdapter();

    bool isNotationExisting() const override;
    bool isTextEditingStarted() const override;
    mu::async::Notification isTextEditingChanged() const override;

    // notation commands
    void beginCommand() override;
    void endCommand() override;

    // notation styling
    void updateStyleValue(const Ms::Sid& styleId, const QVariant& newValue) override;
    QVariant styleValue(const Ms::Sid& styleId) override;

    // dialogs
    void showSpecialCharactersDialog() override;
    void showStaffTextPropertiesDialog() override;
    void showPageSettingsDialog() override;
    void showStyleSettingsDialog() override;
    void showTimeSignaturePropertiesDialog() override;
    void showArticulationPropertiesDialog() override;
    void showGridConfigurationDialog() override;

    // actions
    void updatePageMarginsVisibility(const bool isVisible) override;
    void updateFramesVisibility(const bool isVisible) override;
    void updateHorizontalGridSnapping(const bool isSnapped) override;
    void updateVerticalGridSnapping(const bool isSnapped) override;
    void updateUnprintableElementsVisibility(const bool isVisible) override;
    void updateInvisibleElementsDisplaying(const bool isVisible) override;

    // notation layout
    void updateNotation() override;
private:
    Ms::Score* score() const;

    mu::async::Notification m_isTextEditingNotification;
};

#endif // MU3INSPECTORADAPTER_H
