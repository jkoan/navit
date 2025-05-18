if(nordicfirmware)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../../../../)
add_executable (imageitembuilder imageitembuilder.c)
add_custom_command(TARGET imageitembuilder POST_BUILD
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/imageitembuilder
    COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/imageitem_def.h ${CMAKE_CURRENT_SOURCE_DIR}/../nordicmodule/
    COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/imageheaders.h ${CMAKE_CURRENT_SOURCE_DIR}/../nordicmodule/
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Create imageitem_def.h and allheaders.h"
)
endif()