
set (KORS_ASYNC_SRC
    ${CMAKE_CURRENT_LIST_DIR}/conf.h
    ${CMAKE_CURRENT_LIST_DIR}/asyncable.h
    ${CMAKE_CURRENT_LIST_DIR}/processevents.h
    ${CMAKE_CURRENT_LIST_DIR}/channel.h
    ${CMAKE_CURRENT_LIST_DIR}/notification.h
    ${CMAKE_CURRENT_LIST_DIR}/async.h
    ${CMAKE_CURRENT_LIST_DIR}/promise.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/conf.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/ringqueue.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/rpcqueue.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/channelimpl.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/queuepool.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/queuepool.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/objectpool.h
)
