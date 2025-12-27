include(FetchContent)

# Show FetchContent progress
set(FETCHCONTENT_QUIET OFF)

FetchContent_Declare(Framework
	GIT_REPOSITORY https://gitlab.com/advanced-effects/framework.git # this will be replaced by your own detached Muse framework
	GIT_TAG 25.12.03
	GIT_SHALLOW TRUE # fast clone
	GIT_PROGRESS TRUE)
FetchContent_MakeAvailable(Framework)

set(MUSE_FRAMEWORK_PATH ${CMAKE_BINARY_DIR}/_deps/framework-src)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
		      ${MUSE_FRAMEWORK_PATH}/buildscripts
		      ${MUSE_FRAMEWORK_PATH}/buildscripts/cmake)

include(DeclareModuleSetup)
