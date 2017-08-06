#include <iostream>

#include "mainwindow.h"
#include <QApplication>

main(int argc, char *argv[]){

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}

