# @Author Matthias P. Nowak
# @date 2018-12-14
# 
# Description
#
# This modules adds "add_precompiled_header(<target> <header>)"
# the header file has to be specified as a relative path from the 
# including CMakeLists.txt # in the c++ files, the header has to be 
# included as "#include <header>", since the module will prepend the 
# list of include directories with one in the build folder, where the 
# precompile header is generated.
# 
# This file is based on the work of Lars, only the GCC part is taken.
# Support is added for shared libraries. GCC can find a GCH-file before 
# finding the related header in another folder and recognises the 
# precompiled header.
#
# Caveats:
# - cmake is somewhat vague which properties are lists, and which ones 
#   are not.
# - Most properties in Cmake are populated when all CMakeLists.txt are 
#   processed, those properties are only available at the "generator"
#   stage.
# - Precompiled headers are not recompiled, if an included file is changed,
#   as there is no dependency discovery implemented.
#
# significant parts were taken from 
# https://github.com/larsch/cmake-precompiled-header
#
# Copyright (C) 2009-2017 Lars Christensen <larsch@belunktum.dk>
#
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation files
# (the 'Software') deal in the Software without restriction,
# including without limitation the rights to use, copy, modify, merge,
# publish, distribute, sublicense, and/or sell copies of the Software,
# and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
# BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
# ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

message(STATUS "cmake function add_precompiled_header included")

## uncomment the following line for debugging of targets
#include("cmake/debugTarget.cmake")

#
# Use 'add_precompiled_header(<target> <header file> HDRS ... )' for each target. 
# CXX files should have an '#include <header.h>' with "header.h" being
# replaced by your header file. The header file should employ guards type
# '#ifdef XX \n #define XX'
# The files after HDRS are included files that trigger recompilation
#
function(add_precompiled_header _target _input)
  #message("got request for precompiled ${_input} for ${_target} lang ${lang}")
  cmake_parse_arguments(_PCH "" "" "HDRS" ${ARGN})
  if(_PCH_HDRS)
    #message("additional headers are ${_PCH_HDRS}")
    set(_pch_headers)
    foreach(_ah ${_PCH_HDRS})
      list(APPEND _pch_headers "${_ah}")
    endforeach()
  endif()
  
  if(CMAKE_COMPILER_IS_GNUCXX)
    # for the moment ignore all other compilers
    if(COMMAND debugTarget)
      debugTarget(${_target})
    endif()
    get_filename_component(_name ${_input} NAME)
    #message(STATUS "name is ${_name}")
    
    # where should the precompiled header go
    set(_pch_header "${CMAKE_CURRENT_SOURCE_DIR}/${_input}")
    set(_pchdir "${CMAKE_CURRENT_BINARY_DIR}/${_target}_inc")
    set(_pchfile "${_pchdir}/${_name}.gch")
    file(MAKE_DIRECTORY "${_pchdir}")
    
    # this puts the precompiled header in the search path
    target_include_directories(${_target} BEFORE PUBLIC "${_pchdir}")

    # we build the command for compiling the precompiled header as a list
    set(_compile ${CMAKE_CXX_COMPILER})

    # collecting some compiler flags
    get_property(_pic TARGET ${_target} PROPERTY POSITION_INDEPENDENT_CODE)
    if(_pic)
      get_property(_type TARGET ${_target} PROPERTY TYPE)
      if(${_type} STREQUAL "EXECUTABLE")
        list(APPEND _compile ${CMAKE_CXX_COMPILE_OPTIONS_PIE})
      elseif(${_type} STREQUAL "SHARED_LIBRARY")
        # the <target>_EXPORTS is nowhere else to be found
        list(APPEND _compile ${CMAKE_CXX_COMPILE_OPTIONS_PIC} -D${_target}_EXPORTS)
      else()
        message("type ${_type}")
      endif()
    endif(_pic)

    # 
    # using a separate list, which flags can be added several times
    # using Remove_doubles at the end of collecting
    # 

    get_property(_ext TARGET ${_target} PROPERTY CXX_STANDARD)
    if(_ext)
      list(APPEND _compile ${CMAKE_CXX${_ext}_EXTENSION_COMPILE_OPTION})
    endif()
      
    #
    # We look though many properties using a foreach loop for those
    #
    
    get_property(_sources TARGET ${_target} PROPERTY SOURCES)    
    foreach(_source ${_sources})
     
      # since it is not a target, we need to add the .gch as an extra dependency
      get_source_file_property(_object_depends "${_source}" OBJECT_DEPENDS)
      if(NOT _object_depends)
        set(_object_depends)
      endif()
      list(INSERT _object_depends 0 "${_pchfile}")      
      set_source_files_properties(${_source} PROPERTIES
      OBJECT_DEPENDS "${_object_depends}")   
         
    endforeach(_source ${_sources})
    
    ##### BUG #####
    # The generator expression has no escape for a space, it will turn up
    # wrong in Ninja. But the list has to be separated with certain prefixes
    # and by some whitespace. The tabulator \t does the trick.
    #
    
    #in case they show up
    
    string(TOUPPER ${CMAKE_BUILD_TYPE} _bt)
    #TODO: don't know what to do with compile features
    #set(_compile_features $<TARGET_PROPERTY:${_target},COMPILE_FEATURES>)
    #list(APPEND _compile $<$<BOOL:${_compile_features}>:-D$<JOIN:${_compile_features},\t-D>>)    

    set(_compile_flags $<TARGET_PROPERTY:${_target},COMPILE_FLAGS>)
    list(APPEND _compile $<$<BOOL:${_compile_flags}>:-XCFL$<JOIN:${_compile_flags},\t-XCFL>>)    
    list(APPEND _compile $<JOIN:$<TARGET_PROPERTY:${_target},COMPILE_OPTIONS>,\t>)
    list(APPEND _compile ${CMAKE_CXX_FLAGS_${_bt}})

    set(_compile_defs $<TARGET_PROPERTY:${_target},COMPILE_DEFINITIONS>)
    list(APPEND _compile $<$<BOOL:${_compile_defs}>:-D$<JOIN:${_compile_defs},\t-D>>)    

    set(_incl_dirs $<TARGET_PROPERTY:${_target},INCLUDE_DIRECTORIES>)
    list(APPEND _compile $<$<BOOL:${_incl_dirs}>:-I$<JOIN:${_incl_dirs},\t-I>>)    

    set(_compile1 "-o" "${_pchfile}" "${_pch_header}")
    
    add_custom_command(
      OUTPUT "${_pchfile}"
      COMMAND ${_compile}  ${_compile1} #it automatically expands the list to arguments
      DEPENDS ${_pch_header} ${_pch_headers}
      COMMENT "Precompiling ${_name} for ${_target} (C++)")    
 
    # not necessary, but good for developing
    target_compile_options(${_target} PUBLIC -Winvalid-pch)
    
   endif(CMAKE_COMPILER_IS_GNUCXX)
endfunction()
