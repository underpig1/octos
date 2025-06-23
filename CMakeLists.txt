cmake_minimum_required(VERSION 3.15)
project(Octos LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(/FS)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")
add_executable(main ${SOURCES})

target_compile_definitions(main PRIVATE WEBVIEW2_NO_IDL UNICODE _UNICODE)
target_compile_options(main PRIVATE /FS)

target_link_libraries(main PRIVATE
    unofficial::webview2::webview2
    WIL::WIL
    nlohmann_json::nlohmann_json
    ole32
    uuid
    user32
    shlwapi
    dcomp
    d3d11
    shell32
    comctl32
    advapi32
)

# Find vcpkg packages
find_package(wil CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(unofficial-webview2 CONFIG REQUIRED)

set_target_properties(main PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
set_target_properties(main PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out"
)

add_custom_command(TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:main>/"
    COMMENT "Copying asset files to output directory"
)