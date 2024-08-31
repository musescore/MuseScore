#pragma once

#include "extensions/extensionstypes.h"

namespace mu::project {
static inline const muse::extensions::ExecPointName EXEC_ONPOST_PROJECT_CREATED
    = "onpost_project_created";

static inline const muse::extensions::ExecPointName EXEC_ONPOST_PROJECT_OPENED
    = "onpost_project_opened";

static inline const muse::extensions::ExecPointName EXEC_ONPRE_PROJECT_SAVE
    = "onpre_project_saved";

static inline const muse::extensions::ExecPointName EXEC_ONPOST_PROJECT_SAVED
    = "onpost_project_saved";
}
