#include "templatesrepository.h"

#include "log.h"

using namespace mu;
using namespace mu::domain::notation;
using namespace mu::framework;

RetVal<TemplateCategoryList> TemplatesRepository::categories() const
{
    TemplateCategoryList result;

    for (const QString& dirPath: configuration()->templatesDirPaths()) {
        if (isEmpty(dirPath)) {
            continue;
        }

        TemplateCategory category;

        category.title = correctedTitle(fsOperations()->dirName(dirPath));
        category.codeKey = dirPath;

        result << category;
    }

    return RetVal<TemplateCategoryList>::make_ok(result);
}

RetVal<MetaList> TemplatesRepository::templatesMeta(const QString& categoryCode) const
{
    MetaList result;
    QStringList templates = templatesPaths(categoryCode);

    for (const QString& path: templates) {
        RetVal<Meta> meta = msczReader()->readMeta(io::pathFromQString(path));

        if (!meta.ret) {
            LOGE() << meta.ret.toString();
            continue;
        }

        result << meta.val;
    }

    return RetVal<MetaList>::make_ok(result);
}

bool TemplatesRepository::isEmpty(const QString& dirPath) const
{
    return templatesPaths(dirPath).isEmpty();
}

QString TemplatesRepository::correctedTitle(const QString& title) const
{
    return title;
}

QStringList TemplatesRepository::templatesPaths(const QString& dirPath) const
{
    QStringList filters { "*.mscz", "*.mscx" };
    RetVal<QStringList> result = fsOperations()->scanFiles(dirPath, filters, IFsOperations::ScanMode::IncludeSubdirs);

    if (!result.ret) {
        LOGE() << result.ret.toString();
    }

    return result.val;
}
