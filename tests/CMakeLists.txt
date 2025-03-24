OPTION(VKVG_TEST_DIRECT_DRAW "(Experimental)Draw directly on backend surface, if off surface is blitted." OFF)
IF (VKVG_TEST_DIRECT_DRAW)
	ADD_DEFINITIONS (-DVKVG_TEST_DIRECT_DRAW)
ENDIF ()

OPTION(VKVG_TEST_OFFSCREEN "Draw offscreen without presenting result." OFF)
IF (VKVG_TEST_OFFSCREEN)
	ADD_DEFINITIONS (-DVKVG_TEST_OFFSCREEN)
ENDIF ()

set(TESTS_DATA_ROOT "${CMAKE_SOURCE_DIR}/tests/data/" CACHE STRING "Data path for tests")

FUNCTION (buildtest TEST_FILE)
	GET_FILENAME_COMPONENT(TEST_NAME ${TEST_FILE} NAME_WE)
	ADD_EXECUTABLE(${TEST_NAME} ${TEST_FILE} $<TARGET_OBJECTS:tests_common>)
	SET_TARGET_PROPERTIES(${TEST_NAME} PROPERTIES
			OUTPUT_NAME ${TEST_NAME}
			C_STANDARD 11
			LINKER_LANGUAGE C
	)
        target_compile_definitions("${TEST_NAME}" PUBLIC
            TESTS_DATA_ROOT="${TESTS_DATA_ROOT}"
        )
	TARGET_INCLUDE_DIRECTORIES(${TEST_NAME}
		PRIVATE
			${Vulkan_INCLUDE_DIRS}
			${GLFW3_INCLUDE_DIR}
			"${CMAKE_CURRENT_SOURCE_DIR}/../include"
			"${CMAKE_CURRENT_SOURCE_DIR}/../src"
			"${CMAKE_CURRENT_SOURCE_DIR}/common"
			"${CMAKE_CURRENT_SOURCE_DIR}/../vkh/include"
			"${CMAKE_CURRENT_SOURCE_DIR}/../vkh/src"
	)
	TARGET_LINK_LIBRARIES(${TEST_NAME}
		PRIVATE
			${Vulkan_LIBRARIES}
			${GLFW3_LIBRARY}
			Threads::Threads
	)
	TARGET_LINK_LIBRARIES(${TEST_NAME} PRIVATE vkvg)

ENDFUNCTION (buildtest)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

ADD_LIBRARY(tests_common OBJECT common/vkengine.c common/test.c common/rnd.c common/tinycthread.c)

TARGET_INCLUDE_DIRECTORIES(tests_common
 	PRIVATE
 		${Vulkan_INCLUDE_DIRS}
 		${GLFW3_INCLUDE_DIR}
 		"${CMAKE_CURRENT_SOURCE_DIR}/../include"
 		#"${CMAKE_CURRENT_SOURCE_DIR}/../src"
 		"${CMAKE_CURRENT_SOURCE_DIR}/common"
 		"${CMAKE_CURRENT_SOURCE_DIR}/../vkh/include"
 		"${CMAKE_CURRENT_SOURCE_DIR}/../vkh/src"
)

file(GLOB_RECURSE DATAS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "data/*")
FOREACH(DATA_FILE ${DATAS})
	GET_FILENAME_COMPONENT(copy-dest-dir ${CMAKE_CURRENT_BINARY_DIR}/${DATA_FILE} DIRECTORY)
	SET(copy-output ${CMAKE_CURRENT_BINARY_DIR}/${DATA_FILE})
	ADD_CUSTOM_COMMAND(
		OUTPUT  ${copy-output}
		COMMAND ${CMAKE_COMMAND} -E make_directory ${copy-dest-dir}
		COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/${DATA_FILE}" "${copy-output}"
		COMMENT "Copying ${DATA_FILE} to ${copy-output}"
		DEPENDS ${DATA_FILE}
		VERBATIM
		)
ENDFOREACH()
add_custom_target("${PROJECT_NAME}_DataCopy" ALL DEPENDS ${DATAS})

#build test apps
file(GLOB TESTS
	"*.c"
	"perfs/*.c"
	"multithreading/*.c"
)

IF (VKVG_SVG)
	LIST (APPEND TESTS "vkvg-svg/svg-viewer.c")
ENDIF()

FOREACH(TEST ${TESTS})
	buildtest(${TEST})
ENDFOREACH()




