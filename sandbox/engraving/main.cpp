#include <QCoreApplication>
#include <QDebug>

#include "modularity/ioc.h"

#include "fontproviderstub.h"

#include "engraving/libmscore/score.h"

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);

    mu::modularity::ioc()->registerExport<mu::draw::IFontProvider>("test", new mu::draw::FontProviderStub());

    Ms::Score* s = new Ms::Score();
    qDebug() << "score: " << s;

    return a.exec();
}
