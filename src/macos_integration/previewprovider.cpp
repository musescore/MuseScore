#include "previewprovider.h"

#include <memory>
#include <string>
#include <vector>

#include <QGuiApplication>

#include "draw/drawmodule.h"
#include "engraving/engravingmodule.h"
#include "global/globalmodule.h"
#include "global/iapplication.h"
#include "global/io/buffer.h"
#include "importexport/imagesexport/imagesexportmodule.h"
#include "importexport/imagesexport/internal/pdfwriter.h"
#include "modularity/imodulesetup.h"
#include "modularity/ioc.h"
#include "notation/notationmodule.h"
#include "project/inotationproject.h"
#include "project/iprojectcreator.h"
#include "project/projectmodule.h"
#include "project/types/projecttypes.h"
#include "types/bytearray.h"

class App
{
public:
    App()
    {
        m_modules.emplace_back(std::make_unique<muse::draw::DrawModule>());
        m_modules.emplace_back(std::make_unique<mu::engraving::EngravingModule>());
        m_modules.emplace_back(std::make_unique<mu::notation::NotationModule>());
        m_modules.emplace_back(std::make_unique<mu::project::ProjectModule>());
        m_modules.emplace_back(std::make_unique<mu::iex::imagesexport::ImagesExportModule>());

        int argc = 0;
        std::vector<char*> argv;

        m_qapp = std::make_unique<QGuiApplication>(argc, argv.data());

        m_globalModule.registerResources();
        m_globalModule.registerExports();
        for (const auto& m : m_modules) {
            m->registerResources();
        }

        for (const auto& m : m_modules) {
            m->registerExports();
        }

        m_globalModule.resolveImports();
        for (const auto& m : m_modules) {
            m->resolveImports();
        }

        constexpr auto runMode = muse::IApplication::RunMode::ConsoleApp;
        m_globalModule.onPreInit(runMode);
        for (const auto& m : m_modules) {
            m->onPreInit(runMode);
        }

        m_globalModule.onInit(runMode);
        for (const auto& m : m_modules) {
            m->onInit(runMode);
        }

        m_globalModule.onAllInited(runMode);
        for (const auto& m : m_modules) {
            m->onAllInited(runMode);
        }
    }

    ~App()
    {
        for (const auto& m : m_modules) {
            m->onDeinit();
        }
        m_globalModule.onDeinit();

        for (const auto& m : m_modules) {
            m->onDestroy();
        }
        m_globalModule.onDestroy();
    }

private:
    std::unique_ptr<QGuiApplication> m_qapp; // Needed for fonts loading
    muse::GlobalModule m_globalModule;
    std::vector<std::unique_ptr<muse::modularity::IModuleSetup> > m_modules;
};

void PreviewProviderCxx::initIfNeeded()
{
    static App app;
}

std::vector<uint8_t> PreviewProviderCxx::getPdfPreviewData(const std::string& filePath)
{
    muse::GlobalInject<mu::project::IProjectCreator> projectCreator;

    mu::project::INotationProjectPtr project = projectCreator()->newProject(nullptr);

    mu::project::OpenParams openParams;
    openParams.forceMode = true;
    project->load(filePath, openParams);

    muse::io::Buffer buffer;
    buffer.open(muse::io::IODevice::WriteOnly);

    mu::iex::imagesexport::PdfWriter pdfWriter { nullptr };
    pdfWriter.write(project->masterNotation()->notation(), buffer);

    muse::ByteArray byteArray = buffer.data();
    std::vector<uint8_t> data = byteArray.vdata();

    return data;
}
