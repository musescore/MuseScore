#include "notationsettingsproxymodel.h"

#include "notes/notesettingsproxymodel.h"
#include "fermatas/fermatasettingsmodel.h"
#include "tempos/temposettingsmodel.h"
#include "glissandos/glissandosettingsmodel.h"
#include "barlines/barlinesettingsmodel.h"
#include "staffs/staffsettingsmodel.h"
#include "sectionbreaks/sectionbreaksettingsmodel.h"
#include "markers/markersettingsmodel.h"
#include "jumps/jumpsettingsmodel.h"
#include "keysignature/keysignaturesettingsmodel.h"
#include "accidentals/accidentalsettingsmodel.h"
#include "fretdiagrams/fretdiagramsettingsmodel.h"
#include "pedals/pedalssettingsmodel.h"
#include "spacers/spacersettingsmodel.h"
#include "clefs/clefsettingsmodel.h"
#include "hairpins/hairpinsettingsmodel.h"
#include "crescendos/crescendosettingsmodel.h"
#include "stafftype/stafftypesettingsmodel.h"
#include "frames/textframesettingsmodel.h"
#include "frames/verticalframesettingsmodel.h"
#include "frames/horizontalframesettingsmodel.h"

NotationSettingsProxyModel::NotationSettingsProxyModel(QObject* parent, IElementRepositoryService* repository) :
    AbstractInspectorProxyModel(parent)
{
    setSectionType(SECTION_NOTATION);
    setTitle(QStringLiteral("Notation"));

    addModel(new NoteSettingsProxyModel(this, repository));
    addModel(new FermataSettingsModel(this, repository));
    addModel(new TempoSettingsModel(this, repository));
    addModel(new GlissandoSettingsModel(this, repository));
    addModel(new BarlineSettingsModel(this, repository));
    addModel(new StaffSettingsModel(this, repository));
    addModel(new SectionBreakSettingsModel(this, repository));
    addModel(new MarkerSettingsModel(this, repository));
    addModel(new JumpSettingsModel(this, repository));
    addModel(new KeySignatureSettingsModel(this, repository));
    addModel(new AccidentalSettingsModel(this, repository));
    addModel(new FretDiagramSettingsModel(this, repository));
    addModel(new PedalsSettingsModel(this, repository));
    addModel(new SpacerSettingsModel(this, repository));
    addModel(new ClefSettingsModel(this, repository));
    addModel(new HairpinSettingsModel(this, repository));
    addModel(new CrescendoSettingsModel(this, repository));
    addModel(new StaffTypeSettingsModel(this, repository));
    addModel(new TextFrameSettingsModel(this, repository));
    addModel(new VerticalFrameSettingsModel(this, repository));
    addModel(new HorizontalFrameSettingsModel(this, repository));
}
