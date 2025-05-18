if (APPLE OR USE_UIKIT)
set(plugin_googleglass_LIBS "-framework CoreBluetooth")
set(googleglass_srcs googleglass.c googleglass.m)
else()
pkg_check_modules (BLUEZ REQUIRED bluez)
include_directories(${BLUEZ_INCLUDE_DIRS})
link_directories(${BLUEZ_LIBRARY_DIRS})
include_directories(/usr/src/bluez-source)
add_compile_definitions(VERSION=${BLUEZ_VERSION}) 
set(googleglass_srcs googleglass.c /usr/src/bluez-source/btio/btio.c /usr/src/bluez-source/src/shared/timeout-glib.c /usr/src/bluez-source/attrib/utils.c /usr/src/bluez-source/src/shared/util.c /usr/src/bluez-source/lib/uuid.c /usr/src/bluez-source/src/shared/log.c /usr/src/bluez-source/src/log.c /usr/src/bluez-source/src/shared/io-glib.c /usr/src/bluez-source/src/shared/queue.c /usr/src/bluez-source/attrib/gatt.c /usr/src/bluez-source/attrib/att.c /usr/src/bluez-source/attrib/gattrib.c /usr/src/bluez-source/src/shared/crypto.c /usr/src/bluez-source/src/shared/att.c)
endif()

module_add_library(plugin_googleglass ${googleglass_srcs})

if (APPLE OR USE_UIKIT)
else()
target_link_libraries(plugin_googleglass bluetooth)
endif()

#add_subdirectory(imageitembuilder build)
#add_dependencies(plugin_googleglass imageitembuilder) should go to nordicfirmware?
