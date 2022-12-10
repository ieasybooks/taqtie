include(FetchContent)

cmake_minimum_required(VERSION 3.5)

project(taqtie VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Core)

set(PROJECT_SOURCES
        main.cpp

        enums/section_columns.h

        helpers/file_helpers.h
        helpers/file_helpers.cpp
        helpers/process_helpers.h
        helpers/process_helpers.cpp
        helpers/time_helpers.h
        helpers/time_helpers.cpp

        models/section_info.h
        models/section_info.cpp

        readers/sections_reader.h
        readers/sections_reader.cpp

        windows/main_window.h
        windows/main_window.cpp
        windows/main_window.ui
        windows/about_window.h
        windows/about_window.cpp
        windows/about_window.ui

        wrappers/ffmpeg_wrapper.h
        wrappers/ffmpeg_wrapper.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(taqtie
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET taqtie APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(taqtie SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(taqtie
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(taqtie PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(taqtie PRIVATE Qt${QT_VERSION_MAJOR}::Core)

set_target_properties(taqtie PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER taqtie.ieasybooks.com
    MACOSX_BUNDLE_BUNDLE_NAME تقطيع
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS taqtie
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(taqtie)
endif()

FetchContent_Declare(
  QXlsx
  GIT_REPOSITORY https://github.com/QtExcel/QXlsx.git
  GIT_TAG        b5585decd4e1d30ddd6563556c1c6a8da6753492
  SOURCE_SUBDIR  QXlsx
)
FetchContent_MakeAvailable(QXlsx)
target_link_libraries(taqtie PRIVATE QXlsx::QXlsx)
