#include "modularity/ioc.h"

#include "fontproviderstub.h"

#include "engraving/dom/score.h"
#include "engraving/compat/scoreaccess.h"

#include "importexport/guitarpro/guitarpromodule.h"

int main()
{
    const muse::modularity::ContextPtr ctx = std::make_shared<muse::modularity::Context>(1);
    muse::modularity::ioc(ctx)->registerExport<muse::draw::IFontProvider>("test", new muse::draw::FontProviderStub());

    mu::engraving::compat::ScoreAccess::createMasterScore(ctx);

    mu::iex::guitarpro::GuitarProModule gm;
}
