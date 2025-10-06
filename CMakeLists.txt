cmake_minimum_required(VERSION 3.16)
project(GaelicClash LANGUAGES CXX VERSION 1.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(gaa_core STATIC source/gaa/match.cpp)
target_include_directories(gaa_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/source)
target_compile_features(gaa_core PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(gaa_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(gaa_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(gaelic_clash_cli source/cli_main.cpp)
target_link_libraries(gaelic_clash_cli PRIVATE gaa_core)
set_target_properties(gaelic_clash_cli PROPERTIES OUTPUT_NAME GaelicClash)

if(MSVC)
    target_compile_options(gaelic_clash_cli PRIVATE /W4 /permissive-)
else()
    target_compile_options(gaelic_clash_cli PRIVATE -Wall -Wextra -Wpedantic)
endif()

option(ENABLE_GUI "Build the Raylib-powered match presentation" OFF)

if(ENABLE_GUI)
    include(FetchContent)
    set(RAYLIB_VERSION 5.0)
    FetchContent_Declare(
        raylib
        URL https://github.com/raysan5/raylib/releases/download/${RAYLIB_VERSION}/raylib-${RAYLIB_VERSION}.tar.gz
    )
    set(FETCHCONTENT_QUIET OFF)
    FetchContent_MakeAvailable(raylib)

    add_executable(gaelic_clash_gui source/gui_main.cpp)
    target_link_libraries(gaelic_clash_gui PRIVATE gaa_core raylib)
    target_compile_features(gaelic_clash_gui PRIVATE cxx_std_20)
    if(MSVC)
        target_compile_options(gaelic_clash_gui PRIVATE /W4 /permissive-)
    else()
        target_compile_options(gaelic_clash_gui PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    set_target_properties(gaelic_clash_gui PROPERTIES OUTPUT_NAME GaelicClashGUI)
endif()

include(GNUInstallDirs)

install(TARGETS gaelic_clash_cli
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(TARGET gaelic_clash_gui)
    install(TARGETS gaelic_clash_gui
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

install(FILES README.md
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
)

set(CPACK_PACKAGE_NAME "GaelicClash")
set(CPACK_PACKAGE_VENDOR "Gaelic Clash Studio")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Fast-paced Gaelic football simulator")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Gaelic Clash")

if(WIN32)
    set(CPACK_GENERATOR "ZIP;NSIS")
    set(CPACK_NSIS_DISPLAY_NAME "Gaelic Clash Simulator")
else()
    set(CPACK_GENERATOR "TGZ;ZIP")
endif()

set(_executables "GaelicClash" "Gaelic Clash CLI")
if(TARGET gaelic_clash_gui)
    list(APPEND _executables "GaelicClashGUI" "Gaelic Clash GUI")
endif()
set(CPACK_PACKAGE_EXECUTABLES ${_executables})

include(CPack)
