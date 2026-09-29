option(VKVG_TEST_DIRECT_DRAW
	"(Experimental)Draw directly on backend surface, if off surface is blitted."
	OFF
)
if(VKVG_TEST_DIRECT_DRAW)
	add_definitions(-DVKVG_TEST_DIRECT_DRAW)
endif()

option(VKVG_TEST_OFFSCREEN "Draw offscreen without presenting result." OFF)
if(VKVG_TEST_OFFSCREEN)
	add_definitions(-DVKVG_TEST_OFFSCREEN)
endif()

set(
	TESTS_DATA_ROOT
	"${CMAKE_SOURCE_DIR}/tests/data/"
	CACHE
		STRING
		"Data path for tests"
)

function(buildtest TEST_FILE)
	get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
	add_executable(${TEST_NAME} ${TEST_FILE} $<TARGET_OBJECTS:tests_common>)
	set_target_properties(${TEST_NAME}
		PROPERTIES
			OUTPUT_NAME ${TEST_NAME}
			C_STANDARD 11
			LINKER_LANGUAGE C
	)
	target_compile_definitions("${TEST_NAME}"
		PUBLIC TESTS_DATA_ROOT="${TESTS_DATA_ROOT}"
	)
	target_include_directories(${TEST_NAME}
		PRIVATE
			${Vulkan_INCLUDE_DIRS}
			${GLFW3_INCLUDE_DIR}
			"${CMAKE_CURRENT_SOURCE_DIR}/../include"
			"${CMAKE_CURRENT_SOURCE_DIR}/../src"
			"${CMAKE_CURRENT_SOURCE_DIR}/common"
			"${CMAKE_CURRENT_SOURCE_DIR}/../subprojects/vkh/include"
			"${CMAKE_CURRENT_SOURCE_DIR}/../subprojects/vkh/src"
	)
	target_link_libraries(${TEST_NAME}
		PRIVATE
			${Vulkan_LIBRARIES}
			${GLFW3_LIBRARY}
			Threads::Threads
	)
	target_link_libraries(${TEST_NAME} PRIVATE vkvg vkh)

endfunction()

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

add_library(tests_common OBJECT
	common/vkengine.c
	common/test.c
	common/rnd.c
	common/tinycthread.c
)

target_include_directories(tests_common
	PRIVATE
		${Vulkan_INCLUDE_DIRS}
		${GLFW3_INCLUDE_DIR}
		"${CMAKE_CURRENT_SOURCE_DIR}/../include"
		# "${CMAKE_CURRENT_SOURCE_DIR}/../src"
		"${CMAKE_CURRENT_SOURCE_DIR}/common"
		"${CMAKE_CURRENT_SOURCE_DIR}/../subprojects/vkh/include"
		"${CMAKE_CURRENT_SOURCE_DIR}/../subprojects/vkh/src"
)

file(GLOB_RECURSE DATAS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "data/*")
foreach(DATA_FILE ${DATAS})
	get_filename_component(copy-dest-dir
		${CMAKE_CURRENT_BINARY_DIR}/${DATA_FILE}
		DIRECTORY
	)
	set(copy-output ${CMAKE_CURRENT_BINARY_DIR}/${DATA_FILE})
	add_custom_command(
		OUTPUT ${copy-output}
		COMMAND ${CMAKE_COMMAND} -E make_directory ${copy-dest-dir}
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${CMAKE_CURRENT_SOURCE_DIR}/${DATA_FILE}" "${copy-output}"
		COMMENT "Copying ${DATA_FILE} to ${copy-output}"
		DEPENDS ${DATA_FILE}
		VERBATIM
	)
endforeach()
add_custom_target("${PROJECT_NAME}_DataCopy" ALL DEPENDS ${DATAS})

# build test apps
file(GLOB TESTS
	"*.c"
	"perfs/*.c"
	"multithreading/*.c"
)

if(VKVG_SVG)
	list(APPEND TESTS "vkvg-svg/svg-viewer.c")
endif()

foreach(TEST ${TESTS})
	buildtest(${TEST})
endforeach()
