# CMake 最低版本号要求
cmake_minimum_required (VERSION 2.8)

SET(PRJ_NAME playpcm)

# 项目信息
project (${PRJ_NAME})

# 查找目录下的所有源文件
# 并将名称保存到 DIR_SRCS 变量
aux_source_directory(. DIR_SRCS)

# 添加 includ 子目录
#add_subdirectory(include)

# 添加包含路径
#include_directories(./includ)

# 指定生成目标
add_executable(${PRJ_NAME} ${DIR_SRCS})

# 添加链接库
#target_link_libraries(${PRJ_NAME} portaudio)
target_link_libraries(${PRJ_NAME} m)
target_link_libraries(${PRJ_NAME} asound)
target_link_libraries(${PRJ_NAME} pthread)
