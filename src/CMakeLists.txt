set(_target qtimesheet)

message("--- for '${_target}' ---")

find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

include_directories(.)

# resource file compilation with the splash png
qt5_add_resources(QRCS qts.qrc)
# qt5_wrap_cpp(MOCS mainwindow.hh)

add_executable(${_target} ${_target}.cc mainwindow.cc timerecord.cc ${QRCS} ${MOCS})
add_precompiled_header(${_target} ${_target}.hh HDRS mainwindow.hh timerecord.hh)
qt5_use_modules(${_target} Widgets)

#add_custom_command(TARGET ${_target} POST_BUILD
  #COMMAND ${_target}
  #WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  #USES_TERMINAL
  #)
