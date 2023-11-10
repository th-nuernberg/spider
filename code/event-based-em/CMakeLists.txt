cmake_minimum_required(VERSION 2.8.11)
project(dvstracker)

add_definitions(-std=c++11)
set(CMAKE_BUILD_TYPE DEBUG)
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -g -llapack -lm")

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SRC
	src/main.cpp
        src/DVSBytestreamParser.cpp
        src/DVSTrackerConnection.cpp
        src/DVSTrackerControl.cpp
	src/UserFunction.cpp
	src/DVSEventWidget.cpp
	src/MainWindow.cpp
        src/DVSTrackerWidget.cpp
        src/DVSTrackerVisualizerWindow.cpp
        src/DVSTrackerControlWindow.cpp
        src/DVSVisualizerWindow.cpp
        src/DVSLogger.cpp
        src/qcustomplot.cpp
)

set(HEADERS
        inc/Utils.hpp
	inc/Datatypes.hpp
	inc/Commands.hpp
        inc/DVSBytestreamParser.hpp
        inc/DVSTrackerConnection.hpp
        inc/DVSTrackerControl.hpp
        inc/DVSTrackerDebugger.hpp
	inc/UserFunction.hpp
	inc/DVSEventWidget.hpp
        inc/DVSTrackerWidget.hpp
        inc/DVSTrackerVisualizerWindow.hpp
        inc/MainWindow.hpp
        inc/DVSTrackerControlWindow.hpp
        inc/DVSVisualizerWindow.hpp
        inc/DVSLogger.hpp
        inc/qcustomplot.h
)

include_directories(src)
include_directories(inc)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5PrintSupport REQUIRED)

add_executable(${PROJECT_NAME} ${SRC} ${HEADERS_MOC} ${HEADERS})
target_link_libraries(${PROJECT_NAME} Qt5::Widgets Qt5::Network Qt5::PrintSupport m lapack)

qt5_use_modules(dvstracker SerialPort)

