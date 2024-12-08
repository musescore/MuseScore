#ifndef MU_ENGRAVING_IENGRAVINGFONTSPROVIDER_H
#define MU_ENGRAVING_IENGRAVINGFONTSPROVIDER_H

#include <string>

#include "global/modularity/imoduleinterface.h"
#include "global/io/path.h"

#include "iengravingfont.h"

namespace mu::engraving {
class IEngravingFontsProvider : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IEngravingFontsProvider)

public:
    virtual ~IEngravingFontsProvider() = default;

    virtual void addInternalFont(const std::string& name, const std::string& family, const muse::io::path_t& filePath) = 0;
    virtual void addExternalFont(const std::string& name, const std::string& family, const muse::io::path_t& filePath,
                                 const muse::io::path_t& metadataPath, bool isPrivate) = 0;
    virtual IEngravingFontPtr fontByName(const std::string& name) const = 0;
    virtual std::vector<IEngravingFontPtr> fonts() const = 0;

    virtual void setFallbackFont(const std::string& name) = 0;
    virtual IEngravingFontPtr fallbackFont() const = 0;
    virtual bool isFallbackFont(const IEngravingFont* f) const = 0;

    virtual void clearUserFonts() = 0;

    virtual void loadAllFonts() = 0;
};
}

#endif // MU_ENGRAVING_IENGRAVINGFONTSPROVIDER_H
