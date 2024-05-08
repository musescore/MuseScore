#include "modularity/ioc.h"

#include "fontproviderstub.h"

#include "engraving/dom/score.h"
#include "engraving/compat/scoreaccess.h"

#include "importexport/guitarpro/guitarpromodule.h"

int main()
{
    muse::modularity::globalIoc()->registerExport<muse::draw::IFontProvider>("test", new muse::draw::FontProviderStub());

    mu::engraving::compat::ScoreAccess::createMasterScore(nullptr);

    mu::iex::guitarpro::GuitarProModule gm;
}
