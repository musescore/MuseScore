#include <QCoreApplication>
#include <QDebug>

#include "modularity/ioc.h"

#include "fontproviderstub.h"

#include "engraving/dom/score.h"
#include "engraving/compat/scoreaccess.h"

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);

    mu::modularity::ioc()->registerExport<mu::draw::IFontProvider>("test", new mu::draw::FontProviderStub());

    Ms::MasterScore* s = mu::engraving::compat::ScoreAccess::createMasterScore();
    qDebug() << "score: " << s;


    return a.exec();
}
