if (ANDROID)

	FILE(GLOB SRC_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*/*/*/*")

	# Generate Android string resource files from gettext .po.in files.
	# Note: Android language resources should have two letter ISO 639-1 code, and, optionally, iso-3166-1-alpha-2 region code,
	# so we skip all translation files which do not conform these requirements.
	FILE(GLOB POIN_FILES1 RELATIVE ${CMAKE_SOURCE_DIR}/po "${CMAKE_SOURCE_DIR}/po/??.po.in")
	FILE(GLOB POIN_FILES2 RELATIVE ${CMAKE_SOURCE_DIR}/po "${CMAKE_SOURCE_DIR}/po/??_??.po.in")
	set(POIN_FILES ${POIN_FILES1} ${POIN_FILES2})
	foreach (POIN_FILE ${POIN_FILES})
		string(REPLACE ".po.in" "" LANG ${POIN_FILE})
		string(REPLACE "_" "-r" LANG ${LANG})
		set(LANG_RESOURCES ${LANG_RESOURCES} "${CMAKE_CURRENT_BINARY_DIR}/res/values-${LANG}/strings.xml")
		compose_xslt_transform_command(TRANS "${CMAKE_CURRENT_SOURCE_DIR}/po2xml.xslt" "${CMAKE_CURRENT_SOURCE_DIR}/res/values/strings.xml" "${CMAKE_CURRENT_BINARY_DIR}/res/values-${LANG}/strings.xml" "po_file=${CMAKE_SOURCE_DIR}/po/${POIN_FILE}")
		ADD_CUSTOM_COMMAND(
			OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/res/values-${LANG}/strings.xml"
			DEPENDS "${CMAKE_BINARY_DIR}/po/navit.pot" "${CMAKE_SOURCE_DIR}/po/${POIN_FILE}" "${CMAKE_CURRENT_SOURCE_DIR}/po2xml.xslt" "${CMAKE_CURRENT_SOURCE_DIR}/res/values/strings.xml"
			COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/res/values-${LANG}
			${TRANS}
			)
	endforeach(POIN_FILE ${POIN_FILES})

	add_custom_target( android_resources
		DEPENDS ${MODULES_NAME} ${NAVIT_SUPPORT_LIBS} ${LANG_RESOURCES}
		COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/res/raw/
		#COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/libs/${ANDROID_ARCH}
		#COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_BINARY_DIR}/navit/libnavit.so ${CMAKE_CURRENT_BINARY_DIR}/libs/${ANDROID_ARCH}
		COMMAND echo BLA ${CMAKE_CURRENT_SOURCE_DIR} BLA
		COMMAND ${CMAKE_COMMAND}
		-D GLOB_EXP="${PROJECT_BINARY_DIR}/navit/navit*.xml"
		-D DST=${CMAKE_CURRENT_SOURCE_DIR}/res/raw
		-P ${PROJECT_SOURCE_DIR}/cmake/copy_files.cmake
		COMMAND ${CMAKE_COMMAND}
		-D GLOB_EXP="${PROJECT_BINARY_DIR}/navit/icons/*.png"
		-D DST=${CMAKE_CURRENT_SOURCE_DIR}/res/drawable-nodpi
		-D LOWER=1
		-P ${PROJECT_SOURCE_DIR}/cmake/copy_files.cmake
		COMMAND ${CMAKE_COMMAND}
		-D GLOB_EXP="${PROJECT_BINARY_DIR}/po/*.mo"
		-D DST=${CMAKE_CURRENT_SOURCE_DIR}/res/raw
		-D LOWER=1
		-P ${PROJECT_SOURCE_DIR}/cmake/copy_files.cmake
		)

	add_dependencies (navit android_resources)
	add_dependencies (android_resources images locales)
endif(ANDROID)
