#include "notesettingsproxymodel.h"

#include "stems/stemsettingsmodel.h"
#include "noteheads/noteheadsettingsmodel.h"
#include "beams/beamsettingsmodel.h"
#include "hooks/hooksettingsmodel.h"

using namespace mu::inspector;

NoteSettingsProxyModel::NoteSettingsProxyModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorProxyModel(parent)
{
    setSectionType(InspectorSectionType::SECTION_NOTATION);
    setModelType(InspectorModelType::TYPE_NOTE);
    setTitle(qtrc("inspector", "Note"));

    addModel(new StemSettingsModel(this, repository));
    addModel(new NoteheadSettingsModel(this, repository));
    addModel(new BeamSettingsModel(this, repository));
    addModel(new HookSettingsModel(this, repository));
}
