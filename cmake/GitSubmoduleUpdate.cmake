FUNCTION (updateGitSubmodule SUBMOD_NAME)
	MESSAGE(STATUS "Git Submodule ${SUBMOD_NAME} update")
	EXECUTE_PROCESS(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive ${SUBMOD_NAME}
					WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
					RESULT_VARIABLE GIT_SUBMOD_RESULT)
	if(NOT GIT_SUBMOD_RESULT EQUAL "0")
		message(FATAL_ERROR "git submodule update --init --recursive ${SUBMOD_NAME} failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
	endif()
ENDFUNCTION (updateGitSubmodule)


FIND_PACKAGE(Git QUIET)
