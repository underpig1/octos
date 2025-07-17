cmake_minimum_required(VERSION 3.15)
project(Octos LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")
add_executable(octos WIN32 ${SOURCES})

target_compile_definitions(octos PRIVATE WEBVIEW2_NO_IDL UNICODE _UNICODE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /FS")

target_link_libraries(octos PRIVATE
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
    urlmon
    windowsapp
    Crypt32
)

find_package(wil CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(unofficial-webview2 CONFIG REQUIRED)

set_target_properties(octos PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out"
)

add_custom_command(TARGET octos POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:octos>/"
    COMMENT "Copying asset files to output directory"
)