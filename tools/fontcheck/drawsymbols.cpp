#include "drawsymbols.h"

#include <QFont>
#include <QPdfWriter>
#include <QPainter>

#include "engraving/types/symnames.h"
#include "engraving/infrastructure/smufl.h"

#include "types.h"

using namespace mu::engraving;

void DrawSymbols::drawSymbols()
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
}
