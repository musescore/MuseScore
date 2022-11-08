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

enum FontCode {
    Bravura,
    BravuraText,
    Leland,
    LelandText
};

static std::vector<FontCode> fonts = { Bravura, BravuraText, Leland, LelandText };

void printStat()
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

void drawImage()
{
    struct Font {
        FontCode code;
        QFont font;

        Font(FontCode c, const QString& f)
            : code(c), font(f, 20) {}
    };

    std::vector<Font> fonts;
    fonts.push_back(Font(Bravura, "Bravura"));
    fonts.push_back(Font(BravuraText, "BravuraText"));
    fonts.push_back(Font(Leland, "Leland"));
    fonts.push_back(Font(LelandText, "LelandText"));

    // QImage img(CONTENT_WIDTH + X_MARGIN * 2, 100 * ROW_HEIGHT, QImage::Format_Grayscale8);
    QPdfWriter pdf("Symbols.pdf");
    pdf.setResolution(300);
    pdf.setTitle("Symbols");
    QPageSize pageSize(QPageSize::A4);
    pdf.setPageLayout(QPageLayout(pageSize,
                                  QPageLayout::Orientation::Portrait,
                                  QMarginsF()));

    const int PAGE_WIDTH = pageSize.sizePixels(300).width();
    const int PAGE_HEIGHT = pageSize.sizePixels(300).height();
    const int MARGIN = 100;
    const int CONTENT_WIDTH = PAGE_WIDTH - MARGIN * 2;
    const int COL_WIDTH = CONTENT_WIDTH / 6;
    const int FIRST_COL_WIDTH = COL_WIDTH * 2;
    const int ROW_COUNT = 20;
    const int ROW_HEIGHT = PAGE_HEIGHT / (ROW_COUNT + 1);

    int CUR_X_POS = MARGIN;
    int CUR_Y_POS = MARGIN;

    auto centerY = [=](int posY) {
        return posY - (ROW_HEIGHT / 2);
    };

    QPainter p;
    p.begin(&pdf);

    QFont headerFont("Ubuntu", 8, 600);
    QFont symNameFont("Ubuntu", 8, 400);

    for (size_t s = size_t(SymId::noSym); s < size_t(SymId::lastSym); ++s) {
        if (s % ROW_COUNT == 0) {
            if (s != 0) {
                pdf.newPage();
            }
            CUR_X_POS = MARGIN;
            CUR_Y_POS = MARGIN;

            // Draw header
            p.setPen(QColor("#000000"));
            p.setFont(headerFont);
            p.drawText(QPointF(CUR_X_POS, CUR_Y_POS - 10), "Sym");
            CUR_X_POS += FIRST_COL_WIDTH;
            for (const Font& f : fonts) {
                p.drawText(QPointF(CUR_X_POS, CUR_Y_POS - 10), f.font.family());
                CUR_X_POS += COL_WIDTH;
            }

            QPen pen(QColor("#000000"));
            pen.setWidth(2);
            p.setPen(pen);
            p.drawLine(MARGIN, CUR_Y_POS, MARGIN + CONTENT_WIDTH, CUR_Y_POS);
        }

        SymId symId = SymId(s);

        char32_t smuflCode = Smufl::smuflCode(symId);

        QString symStr = QString::fromUcs4(&smuflCode, 1);

        CUR_Y_POS += ROW_HEIGHT;
        CUR_X_POS = MARGIN;

        // row line
        {
            QPen pen(QColor("#000000"));
            pen.setWidth(2);
            p.setPen(pen);
            p.drawLine(CUR_X_POS, CUR_Y_POS, CUR_X_POS + CONTENT_WIDTH, CUR_Y_POS);
        }

        // base line
        {
            p.setPen(QColor("#999999"));
            p.drawLine(CUR_X_POS, centerY(CUR_Y_POS), CUR_X_POS + CONTENT_WIDTH, centerY(CUR_Y_POS));
        }

        QPen pen(QColor("#000000"));
        pen.setWidth(1);
        p.setPen(pen);

        p.setFont(symNameFont);
        p.drawText(QPointF(CUR_X_POS, centerY(CUR_Y_POS)), QString::number(s) + ". " + SymNames::nameForSymId(symId).ascii());
        CUR_X_POS += FIRST_COL_WIDTH;

        for (const Font& f : fonts) {
            p.setFont(f.font);
            p.drawText(QPointF(CUR_X_POS, centerY(CUR_Y_POS)), symStr);
            CUR_X_POS += COL_WIDTH;
        }
    }

    p.end();

    //img.save("fonts.png");
}

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
    SymbolFonts::fontByName(u"Bravura"); // load
    SymbolFonts::fontByName(u"Leland");  // load

    QFontDatabase::addApplicationFont(":/fonts/bravura/BravuraText.otf");
    QFontDatabase::addApplicationFont(":/fonts/leland/LelandText.otf");

    // printStat();
    drawImage();

    return 0;
}
