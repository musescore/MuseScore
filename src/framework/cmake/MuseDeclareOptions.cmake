
macro(declare_muse_module_opt name def)
    option(MUSE_MODULE_${name} "Build ${name} module" ${def})
    option(MUSE_MODULE_${name}_TESTS "Build ${name} tests" ${def})
    option(MUSE_MODULE_${name}_API "Build ${name} api" ${def})
endmacro()

# Modules framework (alphabetical order please)
declare_muse_module_opt(ACCESSIBILITY ON)
option(MUSE_MODULE_ACCESSIBILITY_TRACE "Enable accessibility logging" OFF)

declare_muse_module_opt(ACTIONS ON)

declare_muse_module_opt(AUDIO ON)
option(MUSE_MODULE_AUDIO_JACK "Enable jack support" OFF)

if (OS_IS_WIN)
    option(MUSE_MODULE_AUDIO_ASIO "Enable asio support" ON)
endif()

if (OS_IS_LIN OR OS_IS_FBSD)
    option(MUSE_MODULE_AUDIO_PIPEWIRE "Use PipeWire audio driver" OFF) # Turns ON on CI
endif()

option(MUSE_MODULE_AUDIO_EXPORT "Enable audio export" ON)

# 1 - worker
# 2 - driver callback
# 3 - worker - RPC, driver callback - process
set(MUSE_MODULE_AUDIO_WORKMODE 1 CACHE INT "Audio subsystem work mode")

declare_muse_module_opt(AUDIOPLUGINS ON)

declare_muse_module_opt(AUTOBOT ON)
declare_muse_module_opt(CLOUD ON)
option(MUSE_MODULE_CLOUD_MUSESCORECOM "Enable MuseScore.com account" ON)

declare_muse_module_opt(DIAGNOSTICS ON)
option(MUSE_MODULE_DIAGNOSTICS_CRASHPAD_CLIENT "Enable crashpad client" OFF) # enable on CI
option(MUSE_MODULE_DIAGNOSTICS_CRASHPAD_HANDLER_PATH "Path to custom crashpad_handler executable (optional)" "")
set(MUSE_MODULE_DIAGNOSTICS_CRASHREPORT_URL "" CACHE STRING "URL where to send crash reports")

declare_muse_module_opt(DOCKWINDOW ON)

declare_muse_module_opt(DRAW ON)
option(MUSE_MODULE_DRAW_TRACE "Trace draw objects" OFF)
option(MUSE_MODULE_DRAW_USE_QTFONTMETRICS "Use Qt font metrics (for some metrics)" ON)

declare_muse_module_opt(EXTENSIONS ON)

declare_muse_module_opt(GLOBAL ON)
option(MUSE_MODULE_GLOBAL_LOGGER_DEBUGLEVEL "Enable logging debug level" ON)
option(MUSE_MODULE_GLOBAL_MULTI_IOC "Enable multi ioc (multi windows)" OFF)

declare_muse_module_opt(LANGUAGES ON)
declare_muse_module_opt(LEARN ON)
declare_muse_module_opt(MIDI ON)
declare_muse_module_opt(MPE ON)
declare_muse_module_opt(MULTIINSTANCES ON)

declare_muse_module_opt(MUSESAMPLER ON)
option(MUSE_MODULE_MUSESAMPLER_LOAD_IN_DEBUG "Load MuseSampler module in debug builds" OFF)

declare_muse_module_opt(NETWORK ON)
option(MUSE_MODULE_NETWORK_WEBSOCKET "Enable websocket support" OFF)

declare_muse_module_opt(SHORTCUTS ON)

declare_muse_module_opt(TOURS ON)

declare_muse_module_opt(UI ON)
option(MUSE_MODULE_UI_DISABLE_MODALITY "Disable dialogs modality for testing purpose" OFF)
option(MUSE_MODULE_UI_SYSTEMDRAG_SUPPORTED "System drag supported" ON)
option(MUSE_MODULE_UI_SYNCINTERACTIVE_SUPPORTED "Sync interactive supported" ON)
option(MUSE_MODULE_UI_NAVIGATION_EXCLUDEPROJECT "Exclude the main project view from navigation" ON)

declare_muse_module_opt(UPDATE ON)

set(VST3_SDK_VERSION "3.7")
declare_muse_module_opt(VST OFF)
set(MUSE_MODULE_VST_VST3_SDK_PATH "" CACHE PATH "Path to VST3_SDK. SDK version >= ${VST3_SDK_VERSION} required")

declare_muse_module_opt(WORKSPACE ON)

# === Enviropment ===
option(MUSE_THREADS_SUPPORT "Build with threads support" ON)
option(MUSE_CONFIGURATION_IS_WEB "Configuration is web" OFF)
option(MUSE_COMPILE_BUILD_64 "Build 64 bit version" ON)
option(MUSE_COMPILE_ASAN "Enable Address Sanitizer" OFF)
option(MUSE_COMPILE_USE_PCH "Use precompiled headers." ON)
option(MUSE_COMPILE_STRING_DEBUG_HACK "Enable string debug hack (only clang)" ON)

# === Tests ===
option(MUSE_ENABLE_UNIT_TESTS "Build framework unit tests" ON)
option(MUSE_ENABLE_UNIT_TESTS_CODE_COVERAGE "Enable code coverage for unit tests" OFF)

# === Tools ===
option(MUSE_ENABLE_CUSTOM_ALLOCATOR "Enable custom allocator" OFF)
