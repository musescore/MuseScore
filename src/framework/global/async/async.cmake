
include(${CMAKE_CURRENT_LIST_DIR}/../../../../thirdparty/deto_async/async/async.cmake)

set(ASYNC_SRC
    ${ASYNC_SRC}
    ${CMAKE_CURRENT_LIST_DIR}/asyncable.h
    ${CMAKE_CURRENT_LIST_DIR}/notification.h
    ${CMAKE_CURRENT_LIST_DIR}/channel.h
    ${CMAKE_CURRENT_LIST_DIR}/async.h
    ${CMAKE_CURRENT_LIST_DIR}/promise.h
    ${CMAKE_CURRENT_LIST_DIR}/processevents.h
    ${CMAKE_CURRENT_LIST_DIR}/notifylist.h
    )
