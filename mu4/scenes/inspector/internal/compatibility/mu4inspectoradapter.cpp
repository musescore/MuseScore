#include "mu4inspectoradapter.h"

#include "log.h"

using namespace mu::scene::inspector;
using namespace mu::domain::notation;

bool MU4InspectorAdapter::isNotationExisting() const
{
    return !context()->notations().empty();
}

void MU4InspectorAdapter::beginCommand()
{
    commander()->prepareChanges();
}

void MU4InspectorAdapter::endCommand()
{
    commander()->commitChanges();
}

void MU4InspectorAdapter::updateStyleValue(const Ms::Sid& styleId, const QVariant& newValue)
{
    style()->updateStyleValue(styleId, newValue);
}

QVariant MU4InspectorAdapter::styleValue(const Ms::Sid& styleId)
{
    return style()->styleValue(styleId);
}

void MU4InspectorAdapter::showSpecialCharactersDialog()
{
    NOT_IMPLEMENTED;
}

void MU4InspectorAdapter::showStaffTextPropertiesDialog()
{
    NOT_IMPLEMENTED;
}

void MU4InspectorAdapter::showPageSettingsDialog()
{
    NOT_IMPLEMENTED;
}

void MU4InspectorAdapter::showStyleSettingsDialog()
{
    NOT_IMPLEMENTED;
}

void MU4InspectorAdapter::showTimeSignaturePropertiesDialog()
{
    NOT_IMPLEMENTED;
}

void MU4InspectorAdapter::showArticulationPropertiesDialog()
{
    NOT_IMPLEMENTED;
}

void MU4InspectorAdapter::showGridConfigurationDialog()
{
    NOT_IMPLEMENTED;
}

void MU4InspectorAdapter::updatePageMarginsVisibility(const bool /*isVisible*/)
{
    NOT_IMPLEMENTED;
}

void MU4InspectorAdapter::updateFramesVisibility(const bool /*isVisible*/)
{
    NOT_IMPLEMENTED;
}

void MU4InspectorAdapter::updateHorizontalGridSnapping(const bool /*isSnapped*/)
{
    NOT_IMPLEMENTED;
}

void MU4InspectorAdapter::updateVerticalGridSnapping(const bool /*isSnapped*/)
{
    NOT_IMPLEMENTED;
}

void MU4InspectorAdapter::updateUnprintableElementsVisibility(const bool /*isVisible*/)
{
    NOT_IMPLEMENTED;
}

void MU4InspectorAdapter::updateInvisibleElementsDisplaying(const bool /*isVisible*/)
{
    NOT_IMPLEMENTED;
}

void MU4InspectorAdapter::updateNotation()
{
    IF_ASSERT_FAILED(context() && context()->currentNotation()) {
        return;
    }

    return context()->currentNotation()->notationChanged().notify();
}

INotationUndoStack* MU4InspectorAdapter::commander() const
{
    IF_ASSERT_FAILED(context() && context()->currentNotation()) {
        return nullptr;
    }

    return context()->currentNotation()->commander();
}

INotationStyle* MU4InspectorAdapter::style() const
{
    IF_ASSERT_FAILED(context() && context()->currentNotation()) {
        return nullptr;
    }

    return context()->currentNotation()->style();
}
