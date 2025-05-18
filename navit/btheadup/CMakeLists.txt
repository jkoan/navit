if (APPLE OR USE_UIKIT)
set(plugin_btheadup_LIBS "-framework CoreBluetooth")
set(btheadup_srcs btheadup.c btheadup.m)
else()
pkg_check_modules (BLUEZ REQUIRED bluez)
include_directories(${BLUEZ_INCLUDE_DIRS})
link_directories(${BLUEZ_LIBRARY_DIRS})
include_directories(/usr/src/bluez-source)
add_compile_definitions(VERSION=${BLUEZ_VERSION}) 
set(btheadup_srcs btheadup.c /usr/src/bluez-source/btio/btio.c /usr/src/bluez-source/src/shared/timeout-glib.c /usr/src/bluez-source/attrib/utils.c /usr/src/bluez-source/src/shared/util.c /usr/src/bluez-source/lib/uuid.c /usr/src/bluez-source/src/shared/log.c /usr/src/bluez-source/src/log.c /usr/src/bluez-source/src/shared/io-glib.c /usr/src/bluez-source/src/shared/queue.c /usr/src/bluez-source/attrib/gatt.c /usr/src/bluez-source/attrib/att.c /usr/src/bluez-source/attrib/gattrib.c /usr/src/bluez-source/src/shared/crypto.c /usr/src/bluez-source/src/shared/att.c)
endif()

module_add_library(plugin_btheadup ${btheadup_srcs})

if (APPLE OR USE_UIKIT)
else()
target_link_libraries(plugin_btheadup bluetooth)
endif()

#add_subdirectory(imageitembuilder build)
#add_dependencies(plugin_btheadup imageitembuilder) should go to nordicfirmware?

if (nordicfirmware)
  include(ExternalProject)
  ExternalProject_Add(nordicfirmware
    DOWNLOAD_COMMAND  ""
    BUILD_ALWAYS true
    SOURCE_DIR        "${CMAKE_SOURCE_DIR}/navit/plugin/btheadup/nordicmodule/pca10059/s140/armgcc"
    BINARY_DIR        "${CMAKE_SOURCE_DIR}/navit/plugin/btheadup/nordicmodule/pca10059/s140/armgcc"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND     "make"
    INSTALL_COMMAND   "make flash; make flash_softdevice"
    TEST_COMMAND      ""
    DEPENDS imageitembuilder
  )
   add_subdirectory(nordicmodule)
endif()
