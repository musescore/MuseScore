#include "mu3launcher.h"

#include <QAction>

#include "log.h"
#include "shortcut.h"
#include "measureproperties.h"

RetVal<Val> MU3Launcher::open(const std::string& uri)
{
    return open(UriQuery(uri));
}

RetVal<Val> MU3Launcher::open(const UriQuery& uriQuery)
{
    RetVal<Val> result;
    result.ret = true;

    if (uriQuery.uri().path() == "notation/measureproperties") {
        showMeasureProperties(uriQuery.param("index").toInt());
    } else {
        result.ret = false;
    }

    return result;
}

ValCh<Uri> MU3Launcher::currentUri() const
{
    return ValCh<Uri>();
}

Ret MU3Launcher::openUrl(const std::string&)
{
    return false;
}

void MU3Launcher::showMeasureProperties(const int measureNumber)
{
    Ms::MeasureProperties vp(measureNumber);
    vp.exec();
}
