cmake_minimum_required(VERSION 3.5)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()
message("Build type: ${CMAKE_BUILD_TYPE}")

if(NOT DEFINED PLAYER_NAME)
    set(PLAYER_NAME "generic-webplayer")
endif()
if(NOT DEFINED PLAYER_NICE_NAME)
    set(PLAYER_NICE_NAME "Generic WebPlayer")
endif()
if(NOT DEFINED PLAYER_WEBAPP_URL)
    set(PLAYER_WEBAPP_URL "about:blank")
endif()

project(${PLAYER_NAME} LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Widgets WebEngineWidgets REQUIRED)

configure_file(
  ${PROJECT_SOURCE_DIR}/config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

add_executable(${PLAYER_NAME}
  config.h
  main.cpp
  player.cpp
  player.h
  player.ui
  playerwebdialog.cpp
  playerwebdialog.h
  playerwebdialog.ui
)

target_link_libraries(${PLAYER_NAME} PRIVATE Qt5::Widgets Qt5::WebEngineWidgets)

install(TARGETS ${PLAYER_NAME})
