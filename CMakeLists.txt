cmake_minimum_required(VERSION 3.24)

project(
    SmartSpellTomeSorting
    VERSION 1.0.0
    LANGUAGES CXX
)

find_package(CommonLibSSE CONFIG REQUIRED)

add_commonlibsse_plugin(
    ${PROJECT_NAME}
    NAME "Smart Spell Tome Sorting"
    AUTHOR "SulfurNitride"
    MINIMUM_SKSE_VERSION 2.0.20
    SOURCES
        src/Plugin.cpp
        src/Settings.cpp
        src/Settings.h
        src/TomeMetadata.cpp
        src/TomeMetadata.h
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_precompile_headers(${PROJECT_NAME} PRIVATE src/PCH.h)
target_compile_definitions(
    ${PROJECT_NAME}
    PRIVATE SMART_TOME_SORTING_VERSION="${PROJECT_VERSION}"
)

function(enable_strict_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive- /EHsc)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

enable_strict_warnings(${PROJECT_NAME})

set(MOD_STAGE "${CMAKE_BINARY_DIR}/package")
set(MOD_PLUGIN_DIR "${MOD_STAGE}/SKSE/Plugins")
set(MOD_ARCHIVE "${CMAKE_BINARY_DIR}/${PROJECT_NAME}-${PROJECT_VERSION}.zip")
set(MOD_DIST_ARCHIVE "${CMAKE_SOURCE_DIR}/dist/${PROJECT_NAME}-${PROJECT_VERSION}.zip")

add_custom_target(
    package_mod
    COMMAND "${CMAKE_COMMAND}" -E rm -rf "${MOD_STAGE}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${MOD_PLUGIN_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E copy
            "$<TARGET_FILE:${PROJECT_NAME}>"
            "${MOD_PLUGIN_DIR}/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
    COMMAND "${CMAKE_COMMAND}" -E copy
            "${CMAKE_SOURCE_DIR}/package/SmartSpellTomeSorting.toml"
            "${MOD_PLUGIN_DIR}/SmartSpellTomeSorting.toml"
    COMMAND "${CMAKE_COMMAND}" -E rm -f "${MOD_ARCHIVE}"
    COMMAND "${CMAKE_COMMAND}" -E chdir "${MOD_STAGE}"
            "${CMAKE_COMMAND}" -E tar cf "${MOD_ARCHIVE}" --format=zip -- SKSE
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_SOURCE_DIR}/dist"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${MOD_ARCHIVE}" "${MOD_DIST_ARCHIVE}"
    BYPRODUCTS "${MOD_ARCHIVE}" "${MOD_DIST_ARCHIVE}"
    DEPENDS ${PROJECT_NAME}
    COMMENT "Creating ${PROJECT_NAME}-${PROJECT_VERSION}.zip"
    VERBATIM
)
