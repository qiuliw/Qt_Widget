cmake_minimum_required(VERSION 3.16)

project(xplay VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# MSVC 编译器特殊配置
if(MSVC)
    # 启用 /Zc:__cplusplus 以正确报告 C++版本
    add_compile_options(/Zc:__cplusplus)
    # 可选：禁用某些警告或放宽标准（如需要）
    add_compile_options(/permissive-)
    # 强制 MSVC 使用 UTF-8 解析源码
    add_compile_options(/utf-8)  
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets OpenGLWidgets Multimedia)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets OpenGLWidgets Multimedia)

set(PROJECT_SOURCES
        main.cpp
        widget.cpp
        widget.h
        XDecode.cpp
        XDecode.h
        XDemux.cpp
        XDemux.h
        XVideoWidget.cpp
        XVideoWidget.h
        XResample.cpp
        XResample.h
        XAudioPlay.cpp
        XAudioPlay.h
        XAudioThread.cpp
        XAudioThread.h
        XVideoThread.cpp
        XVideoThread.h
        IVideoCall.h
        XDemuxThread.cpp
        XDemuxThread.h
)
# 头文件目录
include_directories(${PROJECT_SOURCE_DIR})

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(xplay
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET xplay APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(xplay SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(xplay
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(xplay PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt6::OpenGLWidgets Qt6::Multimedia)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.xplay)
endif()
set_target_properties(xplay PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS xplay
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(xplay)
endif()


# ffmpeg
set(FFMPEG_ROOT_DIR  "D:/Software/ffmpeg-7.1.1-full_build-shared")
target_include_directories(${PROJECT_NAME} PRIVATE
        ${FFMPEG_ROOT_DIR}/include
)
target_link_directories(${PROJECT_NAME} PRIVATE ${FFMPEG_ROOT_DIR}/lib)
target_link_libraries(${PROJECT_NAME} PRIVATE
        avcodec
        avformat
        avutil
        swscale
        swresample
)


# 如果需要控制台输出，添加以下行
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE OFF # 生成可执行文件，其窗口上有 WinMain 入口点。WinMain会吞噬掉控制台输出,关闭即可
    )
endif()