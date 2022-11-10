#include <iostream>
#include <string>

#include <QString>
#include <QFileInfo>
#include <QApplication>
#include <QFontDatabase>
#include <QPainter>
#include <QImage>
#include <QPdfWriter>
#include <QPageLayout>
#include <QCommandLineParser>

#include "framework/global/globalmodule.h"
#include "framework/fonts/fontsmodule.h"
#include "framework/draw/drawmodule.h"

#include "engraving/engravingmodule.h"
#include "engraving/infrastructure/smufl.h"
#include "engraving/infrastructure/symbolfonts.h"

#include "importexport/guitarpro/guitarpromodule.h"

#include "symbolsinfontstat.h"
#include "drawsymbols.h"
#include "symbolsinscoresstat.h"

#include "log.h"

using namespace mu::framework;
using namespace mu::modularity;
using namespace mu::engraving;

int main(int argc, char* argv[])
{
    std::cout << "Hello World!" << std::endl;
    QApplication a(argc, argv);

    GlobalModule globalModule;
    std::vector<IModuleSetup*> modules;
    modules.push_back(new mu::fonts::FontsModule());
    modules.push_back(new mu::draw::DrawModule());
    modules.push_back(new mu::engraving::EngravingModule());
    modules.push_back(new mu::iex::guitarpro::GuitarProModule());

    // ====================================================
    // Setup modules: Resources, Exports, Imports, UiTypes
    // ====================================================
    globalModule.registerResources();
    globalModule.registerExports();
    globalModule.registerUiTypes();

    for (mu::modularity::IModuleSetup* m : modules) {
        m->registerResources();
    }

    for (mu::modularity::IModuleSetup* m : modules) {
        m->registerExports();
    }

    globalModule.resolveImports();
    for (mu::modularity::IModuleSetup* m : modules) {
        m->registerUiTypes();
        m->resolveImports();
    }

    IApplication::RunMode runMode = IApplication::RunMode::Editor;
    globalModule.onPreInit(runMode);
    for (mu::modularity::IModuleSetup* m : modules) {
        m->onPreInit(runMode);
    }

    globalModule.onInit(runMode);
    for (mu::modularity::IModuleSetup* m : modules) {
        m->onInit(runMode);
    }

    SymbolFonts::fontByName(u"Bravura"); // load
    SymbolFonts::fontByName(u"Leland");  // load

    QCommandLineParser parser;
    parser.addOption(QCommandLineOption("syms-in-fonts", "Symbols in fonts"));
    parser.addOption(QCommandLineOption("draw-syms", "Draw symbols"));
    parser.addOption(QCommandLineOption("syms-in-scores", "Symbols in fonts", "dir"));
    parser.process(QCoreApplication::arguments());

    if (parser.isSet("syms-in-fonts")) {
        SymbolsInFontStat::printStat();
    }

    if (parser.isSet("draw-syms")) {
        DrawSymbols::drawSymbols();
    }

    if (parser.isSet("syms-in-scores")) {
        SymbolsInScoresStat s;
        s.processDir(parser.value("syms-in-scores"));
    }

    return 0;
}
