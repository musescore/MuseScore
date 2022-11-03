#include <iostream>
#include <string>

#include <QString>
#include <QFileInfo>
#include <QApplication>

#include "framework/global/globalmodule.h"
#include "framework/fonts/fontsmodule.h"
#include "framework/draw/drawmodule.h"

#include "engraving/infrastructure/smufl.h"
#include "engraving/infrastructure/symbolfonts.h"
#include "engraving/types/symnames.h"

#include "fontface.h"

#include "log.h"

using namespace mu::framework;
using namespace mu::modularity;
using namespace mu::engraving;

#define FORMAT(str, width) QString(str).leftJustified(width, ' ', true).append("  ").toLatin1().constData()
#define TITLE(str) FORMAT(QString(str), 24)

int main(int argc, char* argv[])
{
    std::cout << "Hello World!" << std::endl;
    QApplication a(argc, argv);

    GlobalModule globalModule;
    std::vector<IModuleSetup*> modules;
    modules.push_back(new mu::fonts::FontsModule());
    modules.push_back(new mu::draw::DrawModule());

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

    // Symbols
    Smufl::init();

    SymbolFonts::addFont(u"Bravura",    u"Bravura",     ":/fonts/bravura/Bravura.otf");
    SymbolFonts::addFont(u"Leland",     u"Leland",      ":/fonts/leland/Leland.otf");

    SymbolFont* bravura = SymbolFonts::fontByName(u"Bravura");
    SymbolFont* leland = SymbolFonts::fontByName(u"Leland");

    enum FontName {
        Bravura,
        BravuraText,
        Leland,
        LelandText
    };

    std::vector<FontName> fonts = { Bravura, BravuraText, Leland, LelandText };

    std::map<FontName, QString> paths;
    paths[Bravura] = ":/fonts/bravura/Bravura.otf";
    paths[BravuraText] = ":/fonts/bravura/BravuraText.otf";
    paths[Leland] = ":/fonts/leland/Leland.otf";
    paths[LelandText] = ":/fonts/leland/LelandText.otf";

    struct FontStat {
        QString path;
        FontFace* face = nullptr;
        std::vector<FontFace::Glyph> glyphs;
        std::set<char32_t> inFont;
        std::set<char32_t> inSymbol;

        QString name() const { return QFileInfo(path).baseName(); }
    };

    std::map<FontName, FontStat> stats;

    for (FontName f : fonts) {
        FontStat fs;
        fs.path = paths[f];
        fs.face = new FontFace();
        fs.face->load(fs.path);
        fs.glyphs = fs.face->glyphs(true);

        stats[f] = fs;
    }

    auto hex = [](char32_t ch) {
        return QString::number(ch, 16);
    };

    auto isDiffCode = [](char32_t leland, char32_t bravura, char32_t smufl) {
        if (!leland) {
            return false;
        }

        if (!bravura) {
            return false;
        }

        if (!smufl) {
            return true;
        }

        return leland != bravura || leland != smufl || bravura != smufl;
    };

    QString str;
    QTextStream stream(&str);
    stream << "\n";

    for (size_t s = size_t(SymId::noSym); s < size_t(SymId::lastSym); ++s) {
        SymId symId = SymId(s);

        if (symId == SymId::noSym || symId == SymId::lastSym) {
            continue;
        }

        char32_t lelandCode = leland->symCode(symId, false);
        if (lelandCode) {
            stats[Leland].inSymbol.insert(lelandCode);
        }

        char32_t bravuraCode = bravura->symCode(symId, false);
        if (bravuraCode) {
            stats[Bravura].inSymbol.insert(bravuraCode);
        }

        char32_t smuflCode = Smufl::smuflCode(symId);

        if (isDiffCode(lelandCode, bravuraCode, smuflCode)) {
            stream << TITLE(SymNames::nameForSymId(symId).ascii())
                   << " leland: " << hex(lelandCode)
                   << " bravura: " << hex(bravuraCode)
                   << " smufl: " << hex(smuflCode)
                   << "\n";
        }

        for (FontName f : fonts) {
            FontStat& fs = stats[f];
            char32_t code = bravuraCode;
            if (f == Leland || f == LelandText) {
                code = lelandCode;
            }

            if (fs.face->inFont(code)) {
                fs.inFont.insert(code);
            }
        }
    }

    stream << "\n\n";

    size_t syms = size_t(SymId::lastSym) - 1;

    stream << "total syms: " << syms << "\n";
    stream << "\n";

    for (FontName f : fonts) {
        FontStat& fs = stats[f];
        stream << fs.name() << ": \n";
        stream << "in font:          " << fs.inFont.size() << "\n";
        stream << "no in font:       " << (syms - fs.inFont.size()) << "\n";
        if (f == Bravura || f == Leland) {
            stream << "in font (sym):    " << fs.inSymbol.size() << "\n";
            stream << "no in font (sym): " << (syms - fs.inSymbol.size()) << "\n";
        }
        stream << "not used:         " << (fs.glyphs.size() - fs.inFont.size()) << "\n";
        stream << "\n";
    }

    std::cout << str.toStdString();

    return 0;
}
