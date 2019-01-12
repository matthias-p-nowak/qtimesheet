set(_target qtimesheet)

message("--- for '${_target}' ---")

find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

include_directories(.)

# resource file compilation with the splash png
qt5_add_resources(QRCS qts.qrc)
# qt5_wrap_cpp(MOCS mainwindow.hh)
if(WIN32)
	set(_RCFILE appicon.rc)
endif()

add_executable(${_target} ${_target}.cc mainwindow.cc timerecord.cc ${QRCS} ${MOCS} ${_RCFILE})
add_precompiled_header(${_target} ${_target}.hh HDRS mainwindow.hh timerecord.hh)
qt5_use_modules(${_target} Widgets)

if(WIN32)
  set_target_properties(${_target} PROPERTIES WIN32_EXECUTABLE TRUE)
  add_custom_command(TARGET ${_target} POST_BUILD
  COMMAND windeployqt.exe ${_target}.exe
  WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  USES_TERMINAL
  )
  set(_NSIS "C:\\Program Files (x86)\\NSIS\\makensis.exe")
  set(_NSIS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/install.nsi")
 add_custom_target(${_target}_install ALL
	DEPENDS ${_target} ${_NSIS_SRC}
	WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
	COMMAND ${_NSIS} /NOCD /V4 ${_NSIS_SRC}
	 USES_TERMINAL
 )
endif()

#add_custom_command(TARGET ${_target} POST_BUILD
  #COMMAND ${_target}
  #WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  #USES_TERMINAL
  #)
