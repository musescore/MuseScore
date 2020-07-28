#ifndef MU3LAUNCHER_H
#define MU3LAUNCHER_H

#include "framework/global/ilauncher.h"

using namespace mu;

class MU3Launcher : public framework::ILauncher
{
public:
    MU3Launcher() = default;

    RetVal<mu::Val> open(const std::string& uri) override;
    RetVal<mu::Val> open(const mu::UriQuery& uriQuery) override;
    ValCh<mu::Uri> currentUri() const override;
    mu::Ret openUrl(const std::string& url) override;

private:
    void showMeasureProperties(const int measureNumber);
};

#endif // MU3LAUNCHER_H
