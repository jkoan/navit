include(FetchContent)
set(CMAKE_TLS_VERIFY true)

function(download_file url)
FetchContent_Declare(nordicsdk
URL ${url}
SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/nordic
)
FetchContent_MakeAvailable(nordicsdk)
endfunction(download_file)

download_file(
https://www.nordicsemi.com/-/media/Software-and-other-downloads/SDKs/nRF5/Binaries/nRF5_SDK_17.1.0_ddde560.zip
)

#add_custom_command(TARGET nordicfirmware PRE_BUILD
#    COMMAND echo which arm-none-eabi-gcc OUTPUT_VARIABLE ARMCROSS_HOME 
#    COMMAND sed 's/GNU_INSTALL_ROOT*/GNU_INSTALL_ROOT${ARMCROSS_HOME}/g' ${CMAKE_CURRENT_SOURCE_DIR}/nordicmodule/nordic/components/toolchain/gcc/Makefile.posix
#    COMMENT "Set arm-none-eabi bindir"
#)

function(dump_cmake_variables)
    get_cmake_property(_variableNames VARIABLES)
    list (SORT _variableNames)
    foreach (_variableName ${_variableNames})
        if (ARGV0)
            unset(MATCHED)
            string(REGEX MATCH ${ARGV0} MATCHED ${_variableName})
            if (NOT MATCHED)
                continue()
            endif()
        endif()
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
endfunction()


#message(STATUS "Downloaded nordic SDK 17.1.0 at ${nordicsdk_SOURCE_DIR}")
#execute_process(COMMAND "${CMAKE_COMMAND}" "-E" "environment")
#dump_cmake_variables()
#message(STATUS "sourcedir: ${CMAKE_CURRENT_SOURCE_DIR}")



