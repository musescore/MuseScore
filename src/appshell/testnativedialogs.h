#ifndef TESTNATIVEDIALOGS_H
#define TESTNATIVEDIALOGS_H

#include <QObject>
#include <QFileDialog>

class TestNativeDialogs : public QFileDialog
{
    Q_OBJECT
public:
    TestNativeDialogs();

    void test();
};

#endif // TESTNATIVEDIALOGS_H
