#include "modularity/ioc.h"

#include "fontproviderstub.h"

#include "engraving/libmscore/score.h"
#include "engraving/compat/scoreaccess.h"

int main(int argc, char* argv[])
{
    mu::modularity::ioc()->registerExport<mu::draw::IFontProvider>("test", new mu::draw::FontProviderStub());

    mu::engraving::MasterScore* s = mu::engraving::compat::ScoreAccess::createMasterScore();
}
