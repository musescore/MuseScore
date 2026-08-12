#pragma once

#include "global/iapplication.h"

#include "cmdoptions.h"

namespace mu::app {
class AppFactory
{
public:
    AppFactory() = default;

    std::shared_ptr<muse::IApplication> newApp(const std::shared_ptr<MuseScoreCmdOptions>& options) const;

private:
    std::shared_ptr<muse::IApplication> newGuiApp(const std::shared_ptr<MuseScoreCmdOptions>& options) const;
    std::shared_ptr<muse::IApplication> newConsoleApp(const std::shared_ptr<MuseScoreCmdOptions>& options) const;
};
}
