/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "printprovider.h"

#include <QPrinter>
#include <QPrintDialog>

#include "log.h"

using namespace mu;
using namespace mu::print;
using namespace muse;
using namespace muse::draw;
using namespace mu::notation;

Ret PrintProvider::printNotation(INotationPtr notation)
{
    IF_ASSERT_FAILED(notation) {
        return make_ret(Ret::Code::InternalError);
    }

    auto painting = notation->painting();

    SizeF pageSizeInch = painting->pageSizeInch();
    QPrinter printerDev(QPrinter::HighResolution);
    QPageSize ps(QPageSize::id(pageSizeInch.toQSizeF(), QPageSize::Inch));
    printerDev.setPageSize(ps);
    printerDev.setPageOrientation(pageSizeInch.width() > pageSizeInch.height() ? QPageLayout::Landscape : QPageLayout::Portrait);

    //printerDev.setCreator("MuseScore Studio Version: " VERSION);
    printerDev.setFullPage(true);
    if (!printerDev.setPageMargins(QMarginsF())) {
        LOGD() << "unable to clear printer margins";
    }

    printerDev.setColorMode(QPrinter::Color);
    printerDev.setDocName(notation->projectWorkTitleAndPartName());
    printerDev.setOutputFormat(QPrinter::NativeFormat);
    printerDev.setFromTo(1, painting->pageCount());

    QPrintDialog pd(&printerDev, 0);
    if (!pd.exec()) {
        return muse::make_ret(Ret::Code::Cancel);
    }

    Painter painter(&printerDev, "print");

    INotationPainting::Options opt;
    opt.fromPage = printerDev.fromPage() - 1;
    opt.toPage = printerDev.toPage() - 1;
    // See https://doc.qt.io/qt-5/qprinter.html#supportsMultipleCopies
    opt.copyCount = printerDev.supportsMultipleCopies() ? 1 : printerDev.copyCount();
    opt.deviceDpi = printerDev.logicalDpiX();
    opt.onNewPage = [&printerDev]() { printerDev.newPage(); };

    painting->paintPrint(&painter, opt);

    painter.endDraw();

    return muse::make_ok();
}
