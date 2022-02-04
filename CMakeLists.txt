cmake_minimum_required(VERSION 3.16)
project(Tetris)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXECUTABLE_SUFFIX ".html")

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS} /home/ktoztam/webassembly/emsdk/upstream/emscripten/system/include)


add_executable(Tetris main.cpp)
target_link_libraries(Tetris ${SDL2_LIBRARIES})

set_target_properties(Tetris PROPERTIES LINK_FLAGS "--preload-file cs8x8.bmp --preload-file save.txt --preload-file score.txt")