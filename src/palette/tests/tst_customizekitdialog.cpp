#include <gtest/gtest.h>
#include "io/path.h"

using namespace muse::io;

class DrumsetFileSaveTests : public ::testing::Test
{
};

TEST_F(DrumsetFileSaveTests, ensureDrumsetFileExtension)
{
    path_t withSuffix("MyKit.drm");
    path_t withoutSuffix("MyKit");

    if (!withSuffix.hasSuffix("drm")) {
        withSuffix = withSuffix.appendingSuffix("drm");
    }

    if (!withoutSuffix.hasSuffix("drm")) {
        withoutSuffix = withoutSuffix.appendingSuffix("drm");
    }

    EXPECT_EQ(withSuffix.toStdString(), "MyKit.drm");
    EXPECT_EQ(withoutSuffix.toStdString(), "MyKit.drm");
}
