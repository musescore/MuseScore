#include "modularity/ioc.h"

#include "fontproviderstub.h"

#include "engraving/dom/score.h"
#include "engraving/compat/scoreaccess.h"

#include "importexport/guitarpro/guitarpromodule.h"

int main(int argc, char* argv[])
{
    mu::modularity::ioc()->registerExport<mu::draw::IFontProvider>("test", new mu::draw::FontProviderStub());

    mu::engraving::compat::ScoreAccess::createMasterScore();

    mu::iex::guitarpro::GuitarProModule gm;
}
