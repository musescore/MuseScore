/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

Ret PrintProvider::setupAndPrint(const Options& opt, const PrintFuncs& funcs)
{
    QPrinter printerDev(QPrinter::HighResolution);
    //QPageSize ps(QPageSize::id(opt.pageSize, QPageSize::Inch));
    QPageSize ps(QPageSize::A2);
    printerDev.setPageSize(ps);
    printerDev.setPageOrientation(opt.pageSize.width() > opt.pageSize.height() ? QPageLayout::Landscape : QPageLayout::Portrait);

    //printerDev.setCreator("MuseScore Version: " VERSION);
    printerDev.setFullPage(true);
    if (!printerDev.setPageMargins(QMarginsF())) {
        LOGD() << "unable to clear printer margins";
    }

    printerDev.setColorMode(QPrinter::Color);
    printerDev.setDocName(opt.title);
    printerDev.setOutputFormat(QPrinter::NativeFormat);
    printerDev.setFromTo(1, opt.pages);

    QPrintDialog pd(&printerDev, 0);
    if (!pd.exec()) {
        return mu::make_ret(Ret::Code::Cancel);
    }

    QPainter p(&printerDev);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);
    p.scale(1, 1);

    if (funcs.onStartPrint) {
        funcs.onStartPrint(&p, printerDev.logicalDpiX());
    }

    if (funcs.onPagePrint) {
        int fromPage = printerDev.fromPage() - 1;
        int toPage   = printerDev.toPage() - 1;
        if (fromPage < 0) {
            fromPage = 0;
        }
        if ((toPage < 0) || (toPage >= opt.pages)) {
            toPage = opt.pages - 1;
        }

        for (int copy = 0; copy < printerDev.copyCount(); ++copy) {
            bool firstPage = true;
            for (int n = fromPage; n <= toPage; ++n) {
                if (!firstPage) {
                    printerDev.newPage();
                }
                firstPage = false;

                funcs.onPagePrint(&p, n);

                if ((copy + 1) < printerDev.copyCount()) {
                    printerDev.newPage();
                }
            }
        }
    }

    if (funcs.onEndPrint) {
        funcs.onEndPrint(&p);
    }

    p.end();

    return mu::make_ret(Ret::Code::Ok);
}
