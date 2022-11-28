#ifndef MU_ENGRAVING_ISYMBOLFONTSPROVIDER_H
#define MU_ENGRAVING_ISYMBOLFONTSPROVIDER_H

#include <string>

#include "global/modularity/imoduleexport.h"
#include "global/io/path.h"

#include "isymbolfont.h"

namespace mu::engraving {
class ISymbolFontsProvider : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(ISymbolFontsProvider);

public:
    virtual ~ISymbolFontsProvider() = default;

    virtual void addFont(const std::string& name, const std::string& family, const io::path_t& filePath) = 0;
    virtual ISymbolFontPtr fontByName(const std::string& name) const = 0;
    virtual std::vector<ISymbolFontPtr> fonts() const = 0;

    virtual void setFallbackFont(const std::string& name) = 0;
    virtual ISymbolFontPtr fallbackFont() const = 0;
    virtual bool isFallbackFont(const ISymbolFont* f) const = 0;
};
}

#endif // MU_ENGRAVING_ISYMBOLFONTSPROVIDER_H
