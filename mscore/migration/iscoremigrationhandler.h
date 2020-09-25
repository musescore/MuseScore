#ifndef ISCOREMIGRATIONHANDLER_H
#define ISCOREMIGRATIONHANDLER_H

#include "libmscore/score.h"

class IScoreMigrationHandler
{
public:
    IScoreMigrationHandler() = default;
    virtual ~IScoreMigrationHandler() = default;

    virtual bool handle(Ms::Score* score) = 0;
};

#endif // ISCOREMIGRATIONHANDLER_H
