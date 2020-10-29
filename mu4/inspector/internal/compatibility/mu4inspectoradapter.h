#ifndef MU4INSPECTORADAPTER_H
#define MU4INSPECTORADAPTER_H

#include "iinspectoradapter.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "notation/inotation.h"

namespace mu {
namespace inspector {
class MU4InspectorAdapter : public IInspectorAdapter
{
    INJECT(inspector, mu::context::IGlobalContext, context)
public:
    MU4InspectorAdapter() = default;

    bool isNotationExisting() const override;
    bool isTextEditingStarted() const override;
    async::Notification isTextEditingChanged() const override;

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
    mu::notation::INotationUndoStackPtr undoStack() const;
    mu::notation::INotationStylePtr style() const;
};
}
}

#endif // MU4INSPECTORADAPTER_H
