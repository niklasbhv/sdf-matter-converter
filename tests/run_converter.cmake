# Runs the converter and checks the files it produced.
#
# Usage:
#   cmake -DCONVERTER=<path> -DARGS=<;-list> -DEXPECT=<;-list of globs> -P run_converter.cmake
#
# Every pattern in EXPECT has to match at least one non-empty file.
# Matching files are deleted before the run, so results of earlier runs cannot satisfy the check.
# JSON files additionally have to parse.

cmake_minimum_required(VERSION 3.22.1)

foreach(pattern IN LISTS EXPECT)
    file(GLOB stale "${pattern}")
    if(stale)
        file(REMOVE ${stale})
    endif()
endforeach()

execute_process(
        COMMAND "${CONVERTER}" ${ARGS}
        RESULT_VARIABLE result
        OUTPUT_VARIABLE output
        ERROR_VARIABLE error)
message("${output}${error}")

if(NOT result EQUAL 0)
    message(FATAL_ERROR "Converter exited with ${result}")
endif()

foreach(pattern IN LISTS EXPECT)
    file(GLOB matches "${pattern}")
    if(NOT matches)
        message(FATAL_ERROR "Expected output missing: ${pattern}")
    endif()
    foreach(match IN LISTS matches)
        file(SIZE "${match}" size)
        if(size EQUAL 0)
            message(FATAL_ERROR "Output is empty: ${match}")
        endif()
        if(match MATCHES "\\.json$")
            file(READ "${match}" content)
            string(JSON type ERROR_VARIABLE json_error TYPE "${content}")
            if(json_error)
                message(FATAL_ERROR "Output is not valid JSON: ${match}\n${json_error}")
            endif()
        endif()
    endforeach()
endforeach()
