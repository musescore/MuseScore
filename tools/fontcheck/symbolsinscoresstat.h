#ifndef SYMBOLSINSCORESSTAT_H
#define SYMBOLSINSCORESSTAT_H

#include <memory>

#include "global/io/path.h"

#include "modularity/ioc.h"
#include "project/inotationreadersregister.h"

namespace mu::engraving {
class MasterScore;
}

class SymbolsStatPaintProvider;
struct SymbolsStat;
class SymbolsInScoresStat
{
    INJECT(tools, mu::project::INotationReadersRegister, readers)
public:

    SymbolsInScoresStat();

    void processDir(const mu::io::path_t& scoreDir);
    void processFile(const mu::io::path_t& scorePath);

private:

    bool loadScore(mu::engraving::MasterScore* score, const mu::io::path_t& path);

    std::string statToString(const SymbolsStat& stat);

    std::shared_ptr<SymbolsStatPaintProvider> m_provider;
};

#endif // SYMBOLSINSCORESSTAT_H
