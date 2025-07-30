cmake_minimum_required(VERSION 3.16)

project(qml_prew VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置C++标准为C++17（对应qmake的CONFIG += c++17）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # 强制要求编译器支持C++17

# # 针对MSVC编译器添加/Zc:__cplusplus选项（对应qmake的 -Zc:__cplusplus）
# if(MSVC)
#     add_compile_options(/Zc:__cplusplus)
# endif()

find_package(Qt6 6.2 COMPONENTS Quick REQUIRED)

qt_add_executable(appqml_prew
    main.cpp
)

qt_add_qml_module(appqml_prew
    URI qml_prew
    VERSION 1.0
    QML_FILES main.qml 
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appqml_prew PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appqml_prew
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appqml_prew
    PRIVATE Qt6::Quick)

include(GNUInstallDirs)
install(TARGETS appqml_prew
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
