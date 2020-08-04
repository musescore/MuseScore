include (OptionDependentOnPackage)
include (PrintOptionStatus)

option (SOLOUD_DYNAMIC "Set to ON to build dynamic SoLoud" OFF)
print_option_status (SOLOUD_DYNAMIC "Build dynamic library")

option (SOLOUD_STATIC "Set to ON to build static SoLoud" ON)
print_option_status (SOLOUD_STATIC "Build static library")

# TODO:
option (SOLOUD_BUILD_DEMOS "Set to ON for building demos" OFF)
print_option_status (SOLOUD_BUILD_DEMOS "Build demos")

option (SOLOUD_BACKEND_NULL "Set to ON for building NULL backend" ON)
print_option_status (SOLOUD_BACKEND_NULL "NULL backend")

option (SOLOUD_BACKEND_SDL2 "Set to ON for building SDL2 backend" ON)
print_option_status (SOLOUD_BACKEND_SDL2 "SDL2 backend")

option (SOLOUD_BACKEND_COREAUDIO "Set to ON for building CoreAudio backend" OFF)
print_option_status (SOLOUD_BACKEND_COREAUDIO "CoreAudio backend")

option (SOLOUD_BACKEND_OPENSLES "Set to ON for building OpenSLES backend" OFF)
print_option_status (SOLOUD_BACKEND_OPENSLES "OpenSLES backend")

option (SOLOUD_BACKEND_XAUDIO2 "Set to ON for building XAudio2 backend" OFF)
print_option_status (SOLOUD_BACKEND_XAUDIO2 "XAudio2 backend")

option (SOLOUD_BACKEND_WINMM "Set to ON for building WINMM backend" OFF)
print_option_status (SOLOUD_BACKEND_WINMM "WINMM backend")

option (SOLOUD_BACKEND_WASAPI "Set to ON for building WASAPI backend" OFF)
print_option_status (SOLOUD_BACKEND_WASAPI "WASAPI backend")
