#include "interactivetestmodel.h"

#include "log.h"

InteractiveTestModel::InteractiveTestModel() {}

void InteractiveTestModel::openDialog()
{
    LOGDA() << "before open dialog";
    interactive()->info(std::string("Title"), std::string("Info"));
    //interactive()->open("muse://interactive/sample?sync=false");
    LOGDA() << "after open dialog";
}
