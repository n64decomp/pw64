cmake_minimum_required(VERSION 3.24)

project(PilotwingsPort LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ENABLE_SCRIPTING OFF CACHE BOOL "" FORCE)
set(ENABLE_TCC_COMPILER OFF CACHE BOOL "" FORCE)
set(INCLUDE_MPQ_SUPPORT OFF CACHE BOOL "" FORCE)
set(LUS_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GFX_DEBUG_DISASSEMBLER OFF CACHE BOOL "" FORCE)

# Begin with OpenGL; avoid optional Vulkan/shaderc dependencies.
set(CMAKE_DISABLE_FIND_PACKAGE_Vulkan TRUE)

add_subdirectory(libultraship)

add_executable(pw64_host port/main.cpp)
target_include_directories(pw64_host BEFORE PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/port/controller-compat"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/kernel"
)
target_compile_definitions(pw64_host PRIVATE F3DEX_GBI_2)
target_link_libraries(pw64_host PRIVATE libultraship)

add_library(pw64_controller OBJECT
    src/kernel/controller.c
	port/controller_host.c
)

target_link_libraries(pw64_host PRIVATE pw64_controller)

target_include_directories(pw64_controller BEFORE PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/port/controller-compat"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/kernel"
)

set_target_properties(pw64_controller PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED YES
)

add_library(pw64_math OBJECT
    src/kernel/vector.c
	src/kernel/math.c
)

target_include_directories(pw64_math BEFORE PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/port/controller-compat"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/kernel"
)

set_target_properties(pw64_math PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED YES
)

target_link_libraries(pw64_host PRIVATE
    pw64_math
)

if(UNIX AND NOT APPLE)
    target_link_libraries(pw64_host PRIVATE m)
endif()

add_executable(pw64_math_test
    port/math_test.c
)

target_link_libraries(pw64_math_test PRIVATE
    pw64_math
)

target_include_directories(pw64_math_test BEFORE PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/port/controller-compat"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/kernel"
)

set_target_properties(pw64_math_test PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED YES
)

if(UNIX AND NOT APPLE)
    target_link_libraries(pw64_math_test PRIVATE m)
endif()

add_library(pw64_matrix OBJECT
    src/kernel/matrix.c
)

target_include_directories(pw64_matrix BEFORE PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/port/controller-compat"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/kernel"
)

set_target_properties(pw64_matrix PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED YES
)

target_link_libraries(pw64_matrix PRIVATE
    libultraship
)

add_executable(pw64_matrix_test
    port/matrix_test.c
)

target_include_directories(pw64_matrix_test BEFORE PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/port/controller-compat"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/kernel"
)

target_link_libraries(pw64_matrix_test PRIVATE
    pw64_matrix
    pw64_math
    libultraship
)

set_target_properties(pw64_matrix_test PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED YES
)

target_link_libraries(pw64_host PRIVATE
    pw64_matrix
)

if(UNIX AND NOT APPLE)
    target_link_libraries(pw64_matrix_test PRIVATE m)
endif()