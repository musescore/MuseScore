#pragma once

#include <string>
#include <vector>

class PreviewProviderCxx
{
public:
    // Must be called on main thread.
    static void initIfNeeded();

    static std::vector<uint8_t> getPdfPreviewData(const std::string& filePath);
};
