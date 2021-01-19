cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(filedup CXX)

# NOTE: it works without linking to Boost.TypeErasure ?
find_package(Boost REQUIRED COMPONENTS filesystem program_options)


set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")


# --------------- libfiledup ---------------
set(libfiledup_HEADERS_DIR "${INCLUDE_DIR}/libfiledup")
set(libfiledup_HEADERS
    ${libfiledup_HEADERS_DIR}/exception.hpp
    ${libfiledup_HEADERS_DIR}/filedup.hpp
    ${libfiledup_HEADERS_DIR}/options.hpp
    ${libfiledup_HEADERS_DIR}/detail/file_hash.hpp
)

set(libfiledup_SRC_DIR "${SRC_DIR}/libfiledup")
set(libfiledup_SRC
    ${libfiledup_SRC_DIR}/filedup.cpp
    ${libfiledup_SRC_DIR}/options.cpp
    ${libfiledup_SRC_DIR}/detail/file_hash.cpp
)

add_library(libfiledup STATIC ${libfiledup_SRC} ${libfiledup_HEADERS})
target_include_directories(libfiledup PUBLIC ${INCLUDE_DIR})
target_link_libraries(libfiledup PUBLIC Boost::filesystem)
target_compile_features(libfiledup PUBLIC cxx_std_14)
set_property(TARGET libfiledup PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")


# --------------- filedup_console ---------------
set(filedup_console_SRC_DIR "${SRC_DIR}/filedup_console")
set(filedup_console_SRC ${filedup_console_SRC_DIR}/main.cpp)

add_executable(filedup_console ${filedup_console_SRC})
target_include_directories(filedup_console PRIVATE ${filedup_console_SRC_DIR})
target_link_libraries(filedup_console PRIVATE libfiledup Boost::program_options)
set_property(TARGET filedup_console PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# NOTE: Use smth like this: ?
#if(WIN32)
#   if(VCPKG_LIBRARY_LINKAGE == static)
#       set_property(TARGET libfiledup PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
#       set_property(TARGET filedup_console PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
#   endif()
#endif()
