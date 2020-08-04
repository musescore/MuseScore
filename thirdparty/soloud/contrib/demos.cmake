set (HEADER_PATH ../include)
set (SOURCE_PATH ../demos)

function (soloud_add_demo name sources)
	set (TARGET_NAME SoLoud_${name})
	add_executable (${TARGET_NAME} ${sources})
	target_link_libraries (${TARGET_NAME} soloud)
	include (Install)
endfunction()

include_directories (${SOURCE_PATH}/common)
include_directories (${SOURCE_PATH}/common/imgui)


# soloud_add_demo(c_test ${SOURCE_PATH}/c_test/main.c)
soloud_add_demo(enumerate ${SOURCE_PATH}/enumerate/main.cpp)
# soloud_add_demo(env ${SOURCE_PATH}/env/main.cpp)
# soloud_add_demo(megademo
# 	${SOURCE_PATH}/megademo/3dtest.cpp
# 	${SOURCE_PATH}/megademo/main.cpp
# 	${SOURCE_PATH}/megademo/mixbusses.cpp
# 	${SOURCE_PATH}/megademo/monotone.cpp
# 	${SOURCE_PATH}/megademo/multimusic.cpp
# 	${SOURCE_PATH}/megademo/pewpew.cpp
# 	${SOURCE_PATH}/megademo/radiogaga.cpp
# 	${SOURCE_PATH}/megademo/space.cpp
# 	${SOURCE_PATH}/megademo/speechfilter.cpp
# 	${SOURCE_PATH}/megademo/tedsid.cpp
# 	${SOURCE_PATH}/megademo/virtualvoices.cpp
# )
soloud_add_demo(null ${SOURCE_PATH}/null/main.cpp)
# soloud_add_demo(piano
# 	${SOURCE_PATH}/piano/main.cpp
# 	${SOURCE_PATH}/piano/soloud_basicwave.cpp
# 	${SOURCE_PATH}/piano/soloud_padsynth.cpp
# )
soloud_add_demo(simplest ${SOURCE_PATH}/simplest/main.cpp)
soloud_add_demo(welcome ${SOURCE_PATH}/welcome/main.cpp)
