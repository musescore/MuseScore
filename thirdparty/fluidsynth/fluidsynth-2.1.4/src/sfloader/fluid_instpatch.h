
#ifndef _FLUID_INSTPATCH_H
#define _FLUID_INSTPATCH_H

#include "fluid_sfont.h"
#include "fluid_settings.h"

void fluid_instpatch_init(void);
void fluid_instpatch_deinit(void);
fluid_sfloader_t *new_fluid_instpatch_loader(fluid_settings_t *settings);

#endif // _FLUID_INSTPATCH_H
