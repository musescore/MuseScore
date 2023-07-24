#include <fluidsynth.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
  /* We use any function from the API to ensure the linker does not skip the library */
  fluid_settings_t *settings = new_fluid_settings();
  delete_fluid_settings(settings);
  return EXIT_SUCCESS;
}
