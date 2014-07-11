# Install script for directory: /Users/qhyccd/codespace/QHYCCD_LinuxV4/src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
    "/Users/qhyccd/codespace/QHYCCD_LinuxV4/build/src/libqhyccd.0.dylib"
    "/Users/qhyccd/codespace/QHYCCD_LinuxV4/build/src/libqhyccd.0.1.dylib"
    "/Users/qhyccd/codespace/QHYCCD_LinuxV4/build/src/libqhyccd.dylib"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libqhyccd.0.dylib"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libqhyccd.0.1.dylib"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libqhyccd.dylib"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      execute_process(COMMAND "/usr/bin/install_name_tool"
        -id "libqhyccd.0.1.dylib"
        "${file}")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/usr/bin/strip" "${file}")
      endif()
    endif()
  endforeach()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/lib/libqhyccd/firmware/IC8300.HEX;/usr/local/lib/libqhyccd/firmware/IMG0H.HEX;/usr/local/lib/libqhyccd/firmware/IMG2P.HEX;/usr/local/lib/libqhyccd/firmware/IMG2S.HEX;/usr/local/lib/libqhyccd/firmware/IMG50.HEX;/usr/local/lib/libqhyccd/firmware/QHY10.HEX;/usr/local/lib/libqhyccd/firmware/QHY11.HEX;/usr/local/lib/libqhyccd/firmware/QHY12.HEX;/usr/local/lib/libqhyccd/firmware/QHY16.HEX;/usr/local/lib/libqhyccd/firmware/QHY16000.HEX;/usr/local/lib/libqhyccd/firmware/QHY2.HEX;/usr/local/lib/libqhyccd/firmware/QHY20.HEX;/usr/local/lib/libqhyccd/firmware/QHY21.HEX;/usr/local/lib/libqhyccd/firmware/QHY22.HEX;/usr/local/lib/libqhyccd/firmware/QHY23.HEX;/usr/local/lib/libqhyccd/firmware/QHY2E.HEX;/usr/local/lib/libqhyccd/firmware/QHY5.HEX;/usr/local/lib/libqhyccd/firmware/QHY5II.HEX;/usr/local/lib/libqhyccd/firmware/QHY5LOADER.HEX;/usr/local/lib/libqhyccd/firmware/QHY6.HEX;/usr/local/lib/libqhyccd/firmware/QHY7.HEX;/usr/local/lib/libqhyccd/firmware/QHY8.HEX;/usr/local/lib/libqhyccd/firmware/QHY8L.HEX;/usr/local/lib/libqhyccd/firmware/QHY8M.HEX;/usr/local/lib/libqhyccd/firmware/QHY8PRO.HEX;/usr/local/lib/libqhyccd/firmware/QHY9S.HEX")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/lib/libqhyccd/firmware" TYPE FILE FILES
    "/Users/qhyccd/codespace/QHYCCD_LinuxV4/firmware/IC8300.HEX"
    "/Users/qhyccd/codespace/QHYCCD_LinuxV4/firmware/IMG0H.HEX"
    "/Users/qhyccd/codespace/QHYCCD_LinuxV4/firmware/IMG2P.HEX"
    "/Users/qhyccd/codespace/QHYCCD_LinuxV4/firmware/IMG2S.HEX"
    "/Users/qhyccd/codespace/QHYCCD_LinuxV4/firmware/IMG50.HEX"
    "/Users/qhyccd/codespace/QHYCCD_LinuxV4/firmware/QHY10.HEX"
    "/Users/qhyccd/codespace/QHYCCD_LinuxV4/firmware/QHY11.HEX"
    "/Users/qhyccd/codespace/QHYCCD_LinuxV4/firmware/QHY12.HEX"
    "/Users/qhyccd/codespace/QHYCCD_LinuxV4/firmware/QHY16.HEX"
    "/Users/qhyccd/codespace/QHYCCD_LinuxV4/firmware/QHY16000.HEX"
    "/Users/qhyccd/codespace/QHYCCD_LinuxV4/firmware/QHY2.HEX"
    "/Users/qhyccd/codespace/QHYCCD_LinuxV4/firmware/QHY20.HEX"
    "/Users/qhyccd/codespace/QHYCCD_LinuxV4/firmware/QHY21.HEX"
    "/Users/qhyccd/codespace/QHYCCD_LinuxV4/firmware/QHY22.HEX"
    "/Users/qhyccd/codespace/QHYCCD_LinuxV4/firmware/QHY23.HEX"
    "/Users/qhyccd/codespace/QHYCCD_LinuxV4/firmware/QHY2E.HEX"
    "/Users/qhyccd/codespace/QHYCCD_LinuxV4/firmware/QHY5.HEX"
    "/Users/qhyccd/codespace/QHYCCD_LinuxV4/firmware/QHY5II.HEX"
    "/Users/qhyccd/codespace/QHYCCD_LinuxV4/firmware/QHY5LOADER.HEX"
    "/Users/qhyccd/codespace/QHYCCD_LinuxV4/firmware/QHY6.HEX"
    "/Users/qhyccd/codespace/QHYCCD_LinuxV4/firmware/QHY7.HEX"
    "/Users/qhyccd/codespace/QHYCCD_LinuxV4/firmware/QHY8.HEX"
    "/Users/qhyccd/codespace/QHYCCD_LinuxV4/firmware/QHY8L.HEX"
    "/Users/qhyccd/codespace/QHYCCD_LinuxV4/firmware/QHY8M.HEX"
    "/Users/qhyccd/codespace/QHYCCD_LinuxV4/firmware/QHY8PRO.HEX"
    "/Users/qhyccd/codespace/QHYCCD_LinuxV4/firmware/QHY9S.HEX"
    )
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/libqhyccd" TYPE FILE FILES
    "/Users/qhyccd/codespace/QHYCCD_LinuxV4/src/qhyccd.h"
    "/Users/qhyccd/codespace/QHYCCD_LinuxV4/src/qhyccdcamdef.h"
    "/Users/qhyccd/codespace/QHYCCD_LinuxV4/src/qhyccderr.h"
    )
endif()

