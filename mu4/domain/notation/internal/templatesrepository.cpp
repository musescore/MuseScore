#include "templatesrepository.h"

#include "log.h"

using namespace mu;
using namespace mu::domain::notation;
using namespace mu::framework;

RetVal<MetaList> TemplatesRepository::templatesMeta() const
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
    auto scanForTemplates = [this](const QString& dirPath) -> RetVal<QStringList> {
        QStringList filters { "*.mscz", "*.mscx" };
        return fsOperations()->scanFiles(dirPath, filters, IFsOperations::ScanMode::IncludeSubdirs);
    };

    QStringList result;

    for (const QString& dirPath: configuration()->templatesDirPaths()) {
        RetVal<QStringList> templates = scanForTemplates(dirPath);

        if (!templates.ret) {
            LOGE() << templates.ret.toString();
            continue;
        }

        result << templates.val;
    }

    return result;
}
