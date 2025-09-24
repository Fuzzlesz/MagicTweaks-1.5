cmake_minimum_required(VERSION 3.24)

option(OUTDATED "Applies 1.5.97 offsets/IDs." ON)

# -------- Project ----------
project(
	MagicTweaks
	VERSION 2.2.2
	LANGUAGES CXX
)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

get_filename_component(
	Skyrim64Path
	"[HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\Bethesda Softworks\\Skyrim Special Edition;installed path]"
	ABSOLUTE CACHE
)

list(APPEND CMAKE_IGNORE_PATH "${PROJECT_BINARY_DIR}")
list(APPEND CMAKE_MODULE_PATH
	"${PROJECT_SOURCE_DIR}/tools/SKSE-CMakeModules/Modules"
)
list(APPEND CMAKE_MODULE_PATH
	"${PROJECT_SOURCE_DIR}/tools/Bethesda-CMakeModules/Modules"
)
include(SKSEPlugin)
set(SKSE_USE_XBYAK ON)
add_compile_definitions(SKSE_SUPPORT_XBYAK)

if(OUTDATED)
	add_compile_definitions(SKYRIM_OUTDATED)
else()
	add_compile_definitions(SKYRIM_AE)
	add_compile_definitions(SKYRIM_SUPPORT_AE)
endif()

SKSEPlugin_Add(
	${PROJECT_NAME}
	SOURCE_DIR src
	INCLUDE_DIR include
	SOURCES
		README.md
		vcpkg.json
	PRECOMPILE_HEADERS
		src/common/PCH.h
)

find_package(jsoncpp CONFIG REQUIRED)

target_link_libraries(
		"${PROJECT_NAME}"
		PRIVATE
			JsonCpp::JsonCpp
)

set_target_properties(CommonLibSSE PROPERTIES
	FOLDER External
)

set(DATA_DIR "${PROJECT_SOURCE_DIR}/data")

include(components.cmake)