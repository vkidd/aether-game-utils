if(WIN32)
	# Create a regular windowed application instead of the default console subsystem target
	set(AE_EXE_TYPE WIN32)
	# Use main instead of WinMain
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup")
endif()

add_executable(${AE_CURRENT_EXECUTABLE_NAME}  ${AE_EXE_TYPE} ${AE_CURRENT_SOURCE_FILES})
add_dependencies(${AE_CURRENT_EXECUTABLE_NAME} ${AE_LIBRARY_NAME})
target_link_libraries(${AE_CURRENT_EXECUTABLE_NAME} ${AE_LIBRARIES})

if(APPLE)
	set_target_properties(${AE_CURRENT_EXECUTABLE_NAME} PROPERTIES
		MACOSX_BUNDLE ON
		XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "${AE_CODESIGN_IDENTITY}"
		OUTPUT_NAME ${AE_CURRENT_BUNDLE_NAME}
		RESOURCE "${AE_RESOURCES}"
	)

	set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR})
	if("${CMAKE_GENERATOR}" STREQUAL "Xcode")
		# CMAKE_CURRENT_BINARY_DIR does not include CONFIG path with Xcode builds
		set(CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE})
	endif()
	install(CODE "
		include(BundleUtilities)
		set(BU_CHMOD_BUNDLE_ITEMS TRUE)
		fixup_bundle(\"${CMAKE_INSTALL_PREFIX}/${AE_CURRENT_BUNDLE_NAME}.app\" \"\" \"${AE_DEPS_LIBRARY_DIRS}\")
	" COMPONENT Runtime)
endif()

if(WIN32)
	if(NOT ${AE_RESOURCES} STREQUAL "")
		FILE(COPY "${AE_RESOURCES}" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
	endif()
endif()
