aux_source_directory(. SRC_LIST)

add_library(mprpc ${SRC_LIST})

# 1. 添加 THREADED 宏定义（必须） 以实现调用zookeeper的同步api
# add_definitions(-DTHREADED)  # 全局生效
target_compile_definitions(mprpc PRIVATE THREADED)

target_link_libraries(mprpc muduo_net muduo_base pthread zookeeper_mt) 