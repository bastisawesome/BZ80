function(get_version_from_git)
    find_package(Git QUIET)
    if(NOT Git_FOUND)
        message(WARNING "Git not found")
        return()
    endif()

    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags --dirty --match "v*" --always
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_TAG
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE GIT_RESULT
    )

    if(NOT GIT_RESULT EQUAL 0)
        message(WARNING "Failed to get git tag; Has Git been initialised?")
        return()
    endif()

    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short=7 HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT_SHORT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    string(REGEX REPLACE "^v" "" CLEAN_TAG "${GIT_TAG}")
    if(CLEAN_TAG MATCHES "^([0-9]+)\\.([0-9]+)\\.([0-9]+)(-.*)?$")
        set(MAJOR ${CMAKE_MATCH_1})
        set(MINOR ${CMAKE_MATCH_2})
        set(PATCH ${CMAKE_MATCH_3})

        if("${CMAKE_MATCH_4}" STREQUAL "")
            set(DIRTY "")
        else()
            set(DIRTY "-${GIT_COMMIT_SHORT_HASH}")
        endif()
    else()
        set(MAJOR 0)
        set(MINOR 0)
        set(PATCH 1)
        set(DIRTY "-${GIT_COMMIT_SHORT_HASH}")
    endif()

    set(PROJECT_VERSION_MAJOR ${MAJOR} FORCE)
    set(PROJECT_VERSION_MAJOR ${MAJOR} PARENT_SCOPE)
    set(PROJECT_VERSION_MINOR ${MINOR} FORCE)
    set(PROJECT_VERSION_MINOR ${MINOR} PARENT_SCOPE)
    set(PROJECT_VERSION_PATCH ${PATCH} FORCE)
    set(PROJECT_VERSION_PATCH ${PATCH} PARENT_SCOPE)

    set(FULL_VERSION "${MAJOR}.${MINOR}.${PATCH}${DIRTY}" FORCE)
    set(FULL_VERSION "${MAJOR}.${MINOR}.${PATCH}${DIRTY}" PARENT_SCOPE)
    set(PROJECT_VERSION "${MAJOR}.${MINOR}.${PATCH}" FORCE)
    set(PROJECT_VERSION "${MAJOR}.${MINOR}.${PATCH}" PARENT_SCOPE)
endfunction()
