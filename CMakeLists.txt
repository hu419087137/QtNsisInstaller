cmake_minimum_required(VERSION 3.21)
project(QtNsisInstaller VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_add_executable(QtNsisInstaller
    main.cpp
    InstallerWizard.h
    InstallerWizard.cpp
    WelcomePage.h
    WelcomePage.cpp
    WelcomePage.ui
    LicensePage.h
    LicensePage.cpp
    LicensePage.ui
    InstallDirPage.h
    InstallDirPage.cpp
    InstallDirPage.ui
    ProgressPage.h
    ProgressPage.cpp
    ProgressPage.ui
    FinishPage.h
    FinishPage.cpp
    FinishPage.ui
)

target_link_libraries(QtNsisInstaller PRIVATE Qt6::Widgets)

set_target_properties(QtNsisInstaller PROPERTIES
    WIN32_EXECUTABLE TRUE
)
