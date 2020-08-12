#include "notesettingsproxymodel.h"

#include "stems/stemsettingsmodel.h"
#include "noteheads/noteheadsettingsmodel.h"
#include "beams/beamsettingsmodel.h"
#include "hooks/hooksettingsmodel.h"

NoteSettingsProxyModel::NoteSettingsProxyModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorProxyModel(parent)
{
    setSectionType(SECTION_NOTATION);
    setModelType(TYPE_NOTE);
    setTitle(tr("Note"));

    addModel(new StemSettingsModel(this, repository));
    addModel(new NoteheadSettingsModel(this, repository));
    addModel(new BeamSettingsModel(this, repository));
    addModel(new HookSettingsModel(this, repository));
}
