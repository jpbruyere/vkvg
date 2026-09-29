cmake_minimum_required(VERSION 3.16)

set(LANG "C")
set(CMAKE_${LANG}_STANDARD 11)
set(CMAKE_${LANG}_EXTENSIONS on)

project(vkvg VERSION 0.3.0 DESCRIPTION "Vulkan Vector Graphic" LANGUAGES C CXX)

add_compile_definitions("VKVG_VERSION_MAJOR=${vkvg_VERSION_MAJOR}"
	"VKVG_VERSION_MINOR=${vkvg_VERSION_MINOR}"
	"VKVG_VERSION_REVISION=${vkvg_VERSION_PATCH}"
)

include(CheckSymbolExists)
include(CheckIncludeFile)
include(GNUInstallDirs)
include(CMakeDependentOption)
include(CheckIPOSupported)
check_ipo_supported(RESULT IPO_SUPPORT OUTPUT ipo_err)

if(IPO_SUPPORT)
	message(STATUS "IPO enabled.")
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
	message(STATUS "IPO not supported compilation might be slow... <${ipo_err}>")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

if(BUILD_SHARED_LIBS)
	add_definitions(-DVKVG_SHARED_BUILD)
endif()

option(ENABLE_PROFILING "compile with the -pg options..." OFF)
if(ENABLE_PROFILING)
	add_compile_options(-pg
		-fno-reorder-functions
		-fno-inline
		-O1
		-fthread-jumps
		-falign-functions
		-falign-jumps
		-falign-loops
		-falign-labels
	)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_definitions(-DDEBUG)
	option(ENABLE_VALIDATION "enable vulkan validation layer" ON)
	option(VKVG_USE_MESA_OVERLAY "enable VK_LAYER_MESA_overlay if present." OFF)
	if(VKVG_USE_MESA_OVERLAY)
		add_definitions(-DVKVG_USE_MESA_OVERLAY)
	endif()
	option(ENABLE_DBG_UTILS "enable VK_EXT_debug_utils extension" ON)
	option(VKVG_WIRED_FILL
		"enable wired polygon draw to check vertices and primitives"
		OFF
	)
	if(UNIX)
		add_compile_options(-Wall
			-Wno-extra
			-Wno-unknown-pragmas
			-Wno-missing-braces
			-Wno-unused-variable
			-Wno-switch
			-Wno-unused-value
		)
		if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
			add_compile_options(-g3) # include macro info in debug
		endif()
	elseif(MSVC)
		add_compile_options(/TC /W4 /wd4201 /wd4204 /wd4221 /wd4100) # c11 compliant
	endif()
else()
	unset(ENABLE_VALIDATION CACHE)
	unset(ENABLE_DBG_UTILS CACHE)
	unset(VKVG_WIRED_FILL CACHE)
	unset(VKVG_USE_MESA_OVERLAY CACHE)
	if(ANDROID)
		add_compile_options(-O3 -w -pedantic)
	elseif(UNIX)
		#ADD_COMPILE_OPTIONS(-O3 -w -pedantic -fvisibility=hidden -fno-rtti)
		#ADD_COMPILE_OPTIONS(-flto -pthread -lpthread -Wl,--no-as-needed)
		#ADD_COMPILE_OPTIONS(-fno-rtti)
		#ADD_COMPILE_OPTIONS(-w -flto)
		#ADD_COMPILE_OPTIONS(-fvisibility=hidden)
	elseif(MSVC)
		add_compile_options(/O2 /TC /W0 /Od)
	endif()
endif()

option(ENABLE_GTESTS "enable google unit tests" ON)

if(ENABLE_GTESTS)
	add_subdirectory(tests/gunit_tests)
endif()

option(VKVG_RECORDING "enable experimental recording functions" OFF)

option(VKVG_PREMULT_ALPHA "use premultiplied alpha for internal rendering" ON)

if(VKVG_PREMULT_ALPHA)
	add_definitions(-DVKVG_PREMULT_ALPHA)
	set(GLSLDEFS -DVKVG_PREMULT_ALPHA)
endif()

option(VKVG_DBG_STATS "record contexts statistics in the device" OFF)
if(VKVG_DBG_STATS)
	add_definitions(-DVKVG_DBG_STATS)
endif()

option(VKVG_USE_GLUTESS "Fill non-zero with glu tesselator" ON)

CMAKE_DEPENDENT_OPTION(
	VKVG_SVG
	"render svg with vkvg-svg library"
	OFF
	"UNIX"
	OFF
)

if(NOT TARGET vkh)
	add_subdirectory(subprojects/vkh)
endif()

# use c++ vulkan memory allocator library (https://gpuopen.com/vulkan-memory-allocator/)
if(VKH_USE_VMA)
	add_definitions(-DVKH_USE_VMA)
endif()

if(ENABLE_VALIDATION)
	add_definitions(-DVKVG_USE_VALIDATION)
	option(ENABLE_RENDERDOC "enable renderdoc" OFF)
	if(ENABLE_RENDERDOC)
		add_definitions(-DVKVG_USE_RENDERDOC)
	endif()
endif()
if(VKVG_WIRED_FILL)
	add_definitions(-DVKVG_WIRED_DEBUG)
endif()

find_package(Vulkan REQUIRED COMPONENTS glslangValidator)
find_package(Python REQUIRED)

find_package(Freetype QUIET)
find_package(Fontconfig QUIET)
find_package(HarfBuzz QUIET)
find_package(glfw3 CONFIG QUIET)

if(glfw3_FOUND)
	set(GLFW3_FOUND TRUE)
	set(GLFW3_LIBRARY glfw)
else()
	find_package(GLFW3 QUIET)
endif()
find_package(Doxygen QUIET)

CMAKE_DEPENDENT_OPTION(
	VKVG_USE_FREETYPE
	"use freetype to load and render font glyphs"
	ON
	"Freetype_FOUND"
	OFF
)
CMAKE_DEPENDENT_OPTION(
	VKVG_USE_FONTCONFIG
	"use FontConfig to resolve font names"
	ON
	"Fontconfig_FOUND"
	OFF
)
CMAKE_DEPENDENT_OPTION(
	VKVG_USE_HARFBUZZ
	"use Harbuzz for text layouting"
	ON
	"HarfBuzz_FOUND"
	OFF
)
CMAKE_DEPENDENT_OPTION(
	VKVG_BUILD_TESTS
	"build tests with glfw"
	ON
	"GLFW3_FOUND"
	OFF
)
CMAKE_DEPENDENT_OPTION(
	VKVG_BUILD_DOCS
	"build documentation"
	OFF
	"DOXYGEN_FOUND"
	OFF
)

CMAKE_DEPENDENT_OPTION(
	VKVG_LCD_FONT_FILTER
	"enable freetype lcd font filtering"
	OFF
	"VKVG_USE_FONTCONFIG"
	OFF
)

# todo if cmake vers >= 3.23: make dependent option on Vulkan_VERSION >= 1_2:
option(VKVG_ENABLE_VK_SCALAR_BLOCK_LAYOUT
	"Enable scalar block layout support."
	ON
)
if(VKVG_ENABLE_VK_SCALAR_BLOCK_LAYOUT)
	add_definitions(-DVKVG_ENABLE_VK_SCALAR_BLOCK_LAYOUT)
	list(APPEND GLSLDEFS
		-DVKVG_ENABLE_VK_SCALAR_BLOCK_LAYOUT
	)
endif()

option(VKVG_ENABLE_VK_TIMELINE_SEMAPHORE
	"Enable vulkan TimelineSemaphore support."
	OFF
)
if(VKVG_ENABLE_VK_TIMELINE_SEMAPHORE)
	add_definitions(-DVKVG_ENABLE_VK_TIMELINE_SEMAPHORE)
endif()

if(ENABLE_DBG_UTILS)
	add_definitions(-DVKVG_DBG_UTILS)
endif()
if(VKVG_USE_FREETYPE)
	add_definitions(-DVKVG_USE_FREETYPE)
endif()
if(VKVG_USE_FONTCONFIG)
	add_definitions(-DVKVG_USE_FONTCONFIG)
endif()
if(VKVG_USE_HARFBUZZ)
	add_definitions(-DVKVG_USE_HARFBUZZ)
endif()
if(VKVG_LCD_FONT_FILTER)
	add_definitions(-DVKVG_LCD_FONT_FILTER)
endif()

## SHADER COMPILATION

set(SHADERS_H "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders.h")

if(Vulkan_glslangValidator_FOUND AND Python_FOUND)
	message(STATUS "Compiling shaders")
	set(SHADER_DIR "shaders")
	set(SHADER_FILES
		${SHADER_DIR}/*.frag
		${SHADER_DIR}/*.vert
		${SHADER_DIR}/*.geom
		${SHADER_DIR}/*.comp
	)

	file(GLOB_RECURSE SHADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${SHADER_FILES})

	foreach(SHADER ${SHADERS})
		set(shader-input ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER})
		set(shader-output ${CMAKE_CURRENT_BINARY_DIR}/${SHADER}.spv.h)
		get_filename_component(SPV ${SHADER} NAME)
		add_custom_command(
			OUTPUT ${shader-output}
			COMMAND ${CMAKE_COMMAND} -E make_directory
				"${CMAKE_CURRENT_BINARY_DIR}/${SHADER_DIR}"
			COMMAND ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} ${GLSLDEFS} -V --vn ${SPV}
				${shader-input} -o ${shader-output}
			COMMENT "Compiling ${shader-input}"
			DEPENDS ${SHADER}
		)
		set(SHADER_OUTPUTS ${SHADER_OUTPUTS} ${shader-output})
	endforeach()

	add_custom_target(BuildShaders ALL DEPENDS ${SHADER_OUTPUTS})

	if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.17.0")
		add_custom_target(ClearShadersHeader
			COMMAND ${CMAKE_COMMAND} -E rm -f ${SHADERS_H}
		)
	else()
		add_custom_target(ClearShadersHeader
			COMMAND ${CMAKE_COMMAND} -E remove -f ${SHADERS_H}
		)
	endif()

	add_custom_target(BuildShadersHeader
		DEPENDS
			ClearShadersHeader
			${SHADER_OUTPUTS}
	)

	add_custom_command(
		TARGET BuildShadersHeader
		PRE_BUILD
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${SHADER_DIR}
		COMMAND ${Python_EXECUTABLE} ${CMAKE_SOURCE_DIR}/scripts/headerconcat.py
			--replace-dots -o ${SHADERS_H} ${SHADER_OUTPUTS}
	)

	set_source_files_properties(${SHADERS_H} PROPERTIES GENERATED 1)
	# add_definitions( -DDEBUG_VK_PERF=true )
else()
	message(STATUS
		"glslangvalidator or python was not found, using shader from git tree"
	)
endif()

if(VKVG_USE_GLUTESS)
	add_definitions(-DVKVG_FILL_NZ_GLUTESS)
	add_subdirectory(subprojects/glutess)
endif()

file(GLOB VKVG_SRC src/*.c src/deps/*.c)
file(GLOB VKVG_HEADERS src/*.h)
set(VKVG_PUBLIC_HEADERS "include/vkvg.h;include/vkvg-svg.h")

if(VKVG_SVG)
	add_definitions(-DVKVG_SVG)
	file(GLOB SVG_SRC subprojects/vkvg-svg/src/*.c)
else()
	file(GLOB SVG_SRC src/nsvg/*.c)
endif()

list(APPEND VKVG_SRC ${SVG_SRC})

if(VKVG_RECORDING)
	add_definitions(-DVKVG_RECORDING)
	file(GLOB RECORDING_SRC src/recording/*.c)
	list(APPEND VKVG_SRC ${RECORDING_SRC})
endif()

configure_file(vkvg.pc.in vkvg.pc @ONLY)
install(
	FILES ${CMAKE_CURRENT_BINARY_DIR}/vkvg.pc
	DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig
)

get_directory_property(vkvg_has_root_project PARENT_DIRECTORY)

list(APPEND VKVG_SRC ${SHADERS} ${SHADERS_H})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${VKVG_SRC})

add_library("${PROJECT_NAME}" ${VKVG_SRC} ${VKVG_HEADERS})

set_target_properties(${PROJECT_NAME}
	PROPERTIES
		OUTPUT_NAME ${PROJECT_NAME}
		VERSION ${vkvg_VERSION_MAJOR}.${vkvg_VERSION_MINOR}
		SOVERSION ${vkvg_VERSION_MAJOR}
		C_STANDARD 11
		C_STANDARD_REQUIRED ON
		C_EXTENSIONS OFF
		POSITION_INDEPENDENT_CODE "${BUILD_SHARED_LIBS}"
		PUBLIC_HEADER "${VKVG_PUBLIC_HEADERS}"
		C_VISIBILITY_PRESET hidden
		VISIBILITY_INLINES_HIDDEN ON
)

target_include_directories("${PROJECT_NAME}"
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src
		${CMAKE_CURRENT_SOURCE_DIR}/subprojects/vkh/src
		$<$<BOOL:${VKVG_USE_FREETYPE}>:${FREETYPE_INCLUDE_DIRS}>
		$<$<BOOL:${VKVG_USE_HARFBUZZ}>:${HarfBuzz_INCLUDE_DIRS}>
		$<$<BOOL:${VKVG_USE_FONTCONFIG}>:${Fontconfig_INCLUDE_DIRS}>
		$<$<NOT:$<BOOL:${VKVG_SVG}>>:${CMAKE_CURRENT_SOURCE_DIR}/src/nsvg>
		${CMAKE_CURRENT_SOURCE_DIR}/subprojects/vkh/include
	PUBLIC
		${Vulkan_INCLUDE_DIRS}
		${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(UNIX AND BUILD_SHARED_LIBS)
	#make libm public for shared build to solve libm missing for common_test object lib linking even
	#if -lm present on cc command.
	target_link_libraries("${PROJECT_NAME}" PUBLIC m)
endif()

if(VKVG_USE_FREETYPE)
	target_link_libraries("${PROJECT_NAME}" PUBLIC ${FREETYPE_LIBRARY})
endif()
if(VKVG_USE_FONTCONFIG)
	target_link_libraries("${PROJECT_NAME}" PRIVATE ${Fontconfig_LIBRARIES})
endif()
if(VKVG_USE_HARFBUZZ)
	target_link_libraries("${PROJECT_NAME}" PRIVATE ${HarfBuzz_LIBRARIES})
endif()
if(VKVG_USE_GLUTESS)
	target_link_libraries("${PROJECT_NAME}" PRIVATE glutess)
endif()

target_link_libraries("${PROJECT_NAME}"
	PRIVATE vkh
	PUBLIC ${Vulkan_LIBRARIES}
)
if(SHADERS_H)
	add_dependencies("${PROJECT_NAME}" BuildShadersHeader)
endif()

install(
	TARGETS
		"${PROJECT_NAME}"
		"${PROJECT_NAME}"
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

if(VKVG_BUILD_TESTS)
	add_subdirectory(tests)
	add_subdirectory(samples)
elseif(VKVG_BUILD_OFFSCREEN_TEST)
	add_executable(test_offscreen "${CMAKE_CURRENT_SOURCE_DIR}/tests/offscreen.c")
	target_include_directories(test_offscreen
		PRIVATE
			${Vulkan_INCLUDE_DIRS}
			${CMAKE_CURRENT_SOURCE_DIR}/include
	)
	target_link_libraries(test_offscreen
		vkvg
		${Vulkan_LIBRARIES}
	)
endif()

if(VKVG_BUILD_DOCS)
	set(DOXYGEN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/doc)
	set(DOXYGEN_INPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
	set(DOXYGEN_IMAGE_DIR "${DOXYGEN_DIR}/images")
	set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/doc)
	set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/html/index.html)

	set(DOXYFILE_IN ${DOXYGEN_DIR}/Doxygen.in)
	set(DOXYFILE_OUT ${DOXYGEN_OUTPUT_DIR}/Doxyfile)

	# configure_file(${DOXYGEN_DIR}/dox_layout.xml ${DOXYGEN_OUTPUT_DIR}/dox_layout.xml @ONLY)

	configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

	add_custom_target(copy-doc-files ALL
		COMMAND ${CMAKE_COMMAND} -E copy_directory "${DOXYGEN_DIR}/doxy_theme"
			${DOXYGEN_OUTPUT_DIR}
	)

	add_custom_command(
		OUTPUT ${DOXYGEN_INDEX_FILE}
		DEPENDS
			"${VKVG_PUBLIC_HEADERS}"
			copy-doc-files
		COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
		MAIN_DEPENDENCY ${DOXYFILE_OUT}
		${DOXYFILE_IN}
		COMMENT "Generating docs"
	)
	add_custom_target(doxygen ALL DEPENDS ${DOXYGEN_INDEX_FILE})

	find_package(Sphinx)

	if(Sphinx_FOUND)

		set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/doc)
		set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR}/doc/sphinx)
		set(SPHINX_INDEX_FILE ${SPHINX_BUILD}/index.html)

		add_custom_command(
			OUTPUT ${SPHINX_INDEX_FILE}
			COMMAND ${SPHINX_EXECUTABLE} -b html
				-Dbreathe_projects.VKVG=${DOXYGEN_OUTPUT_DIR}/xml ${SPHINX_SOURCE}
				${SPHINX_BUILD}
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			DEPENDS
				doxygen
				# Other docs files you want to track should go here (or in some variable)
				${SPHINX_SOURCE}/index.rst
				${DOXYGEN_INDEX_FILE}
			MAIN_DEPENDENCY ${SPHINX_SOURCE}/conf.py
			COMMENT "Generating documentation with Sphinx"
		)

		add_custom_target(sphinx ALL DEPENDS ${SPHINX_INDEX_FILE})
	endif()
endif()

if(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")
	set(CPACK_PACKAGE_NAME ${PROJECT_NAME})

	set(CPACK_PACKAGE_DESCRIPTION "Vulkan vector graphic library")
	set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
		"2d vector drawing library using vulkan"
	)
	set(CPACK_PACKAGE_VENDOR "jp Bruyere")
	set(CPACK_PACKAGE_CONTACT "jp_bruyere@hotmail.com")

	set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
	set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
	set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
	set(CPACK_PACKAGE_FILE_NAME
		"${CMAKE_PROJECT_NAME}_${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}"
	)
	set(CPACK_SOURCE_PACKAGE_FILE_NAME
		"${CMAKE_PROJECT_NAME}_${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}"
	)

	set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.md")
	include(InstallRequiredSystemLibraries)
	#SET(CPACK_COMPONENTS_ALL Libraries ApplicationData)
	#dependencies for this service menu
	if(UNIX)
		set(CPACK_PACKAGING_INSTALL_PREFIX "/tmp")
		set(CPACK_SET_DESTDIR "on")
		set(CPACK_ADD_LDCONFIG_CALL 1)
		set(CPACK_DEBIAN_PACKAGE_DEPENDS
			"libharfbuzz-gobject0, libfontconfig1, libfreetype6"
		)
		set(CPACK_GENERATOR "DEB")

		set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
		set(CPACK_DEBIAN_PACKAGE_SECTION "libraries")
		set(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})

	elseif(MSVC)
		set(CPACK_GENERATOR NSIS)
		set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY ${PROJECT_NAME})
		set(CPACK_NSIS_MODIFY_PATH "ON")

	endif()

	include(CPack)
endif()
