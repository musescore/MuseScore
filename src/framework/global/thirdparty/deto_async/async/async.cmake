
set (ASYNC_SRC
    ${CMAKE_CURRENT_LIST_DIR}/asyncable.h
    ${CMAKE_CURRENT_LIST_DIR}/channel.h
    ${CMAKE_CURRENT_LIST_DIR}/notification.h
    ${CMAKE_CURRENT_LIST_DIR}/async.h
    ${CMAKE_CURRENT_LIST_DIR}/promise.h
    ${CMAKE_CURRENT_LIST_DIR}/notifylist.h
    ${CMAKE_CURRENT_LIST_DIR}/changednotify.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/abstractinvoker.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/abstractinvoker.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/queuedinvoker.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/queuedinvoker.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/asyncimpl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/asyncimpl.h
)
