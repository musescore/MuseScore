#include "interactivetestmodel.h"

#include "log.h"

InteractiveTestModel::InteractiveTestModel() {}

void InteractiveTestModel::openDialog()
{
    LOGDA() << "=";
    interactive()->info(std::string("Title"), std::string("Info"));
}
