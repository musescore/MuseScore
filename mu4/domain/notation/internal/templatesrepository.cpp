#include "templatesrepository.h"

#include "log.h"

using namespace mu;
using namespace mu::domain::notation;
using namespace mu::framework;

RetVal<MetaList> TemplatesRepository::templates() const
{
    MetaList result;

    for (const QString& path: templatesPaths()) {
        RetVal<Meta> meta = msczReader()->readMeta(io::pathFromQString(path));

        if (!meta.ret) {
            LOGE() << meta.ret.toString();
            continue;
        }

        result << meta.val;
    }

    return RetVal<MetaList>::make_ok(result);
}

QStringList TemplatesRepository::templatesPaths() const
{
    auto scanForTemplates = [this](const QString& dirPath) -> QStringList {
        QStringList filters { "*.mscz", "*.mscx" };
        return fsOperations()->scanForFiles(dirPath, filters, IFsOperations::ScanMode::IncludeSubdirs);
    };

    QStringList result;

    result << scanForTemplates(configuration()->templatesPath());
    result << scanForTemplates(configuration()->userTemplatesPath());

    for (const QString& extensionTemplatesPath: configuration()->extensionsTemplatesPaths()) {
        result << scanForTemplates(extensionTemplatesPath);
    }

    return result;
}
