message("--- whtbrd starter ---")

find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

include_directories(.)

# resource file compilation with the splash png
# qt5_add_resources(QRCS startup.qrc)
# qt5_wrap_cpp(MOCS startup.hh)

add_executable(qtimesheet qtimesheet.cc ${QRCS} ${MOCS})
qt5_use_modules(qtimesheet Widgets)
add_precompiled_header(qtimesheet qtimesheet.hh HDRS )
