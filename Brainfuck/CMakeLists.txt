project(Brainfuck)

add_executable(${PROJECT_NAME} main.cpp lifter.cpp lifter.hpp)

target_link_libraries(${PROJECT_NAME} PRIVATE VTIL)