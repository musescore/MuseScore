#include <gmock/gmock.h>

#include <QGuiApplication>

GTEST_API_ int main(int argc, char** argv)
{
    QGuiApplication app(argc, argv);

    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
