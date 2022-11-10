#include "symbolsinfontstat.h"

#include <map>
#include <vector>
#include <QString>
#include <QFileInfo>
#include <QTextStream>

#include "engraving/infrastructure/smufl.h"
#include "engraving/infrastructure/symbolfonts.h"
#include "engraving/types/symnames.h"

#include "types.h"
#include "fontface.h"

using namespace mu::engraving;

#define FORMAT(str, width) QString(str).leftJustified(width, ' ', true).append("  ").toLatin1().constData()
#define TITLE(str) FORMAT(QString(str), 24)

void SymbolsInFontStat::printStat()
{
    SymbolFont* bravura = SymbolFonts::fontByName(u"Bravura");
    SymbolFont* leland = SymbolFonts::fontByName(u"Leland");

    std::map<FontCode, QString> paths;
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

    std::map<FontCode, FontStat> stats;

    for (FontCode f : fonts) {
        FontStat fs;
        fs.path = paths[f];
        fs.face = new FontFace();
        fs.face->load(fs.path);
        fs.glyphs = fs.face->allGlyphs(true);

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

        for (FontCode f : fonts) {
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

    for (FontCode f : fonts) {
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
}
