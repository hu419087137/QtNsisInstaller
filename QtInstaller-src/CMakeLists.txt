cmake_minimum_required(VERSION 3.16)

project(QtInstaller VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6 packages
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Svg LinguistTools)

# Source files
set(SOURCES
    main.cpp
    InstallerWizard.cpp
    InstallerWizard.h
    InstallerWizard.ui
    CustomTitleBar.ui
    WelcomePage.ui
    LicensePage.ui
    InstallPathPage.ui
    ComponentsPage.ui
    InstallProgressPage.ui
    FinishDialog.ui
)

# Resource files
set(RESOURCES
    resources.qrc
)

# Create executable
add_executable(QtInstaller
    ${SOURCES}
    ${RESOURCES}
)

# Link Qt libraries
target_link_libraries(QtInstaller PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Svg
)

# ---------------------------------------------------------------------------
# Translations
# ---------------------------------------------------------------------------
set(TS_FILES "${CMAKE_CURRENT_SOURCE_DIR}/i18n/qtinstaller_zh_CN.ts")

# Update TS files from source (creates update_translations target)
qt_add_lupdate(QtInstaller TS_FILES ${TS_FILES})

# Compile TS → QM and embed as resources under :/i18n/
qt_add_lrelease(QtInstaller
    TS_FILES ${TS_FILES}
    QM_FILES_OUTPUT_VARIABLE QM_FILES
)
qt_add_resources(QtInstaller "translations"
    PREFIX "/i18n"
    BASE "${CMAKE_CURRENT_BINARY_DIR}"
    FILES ${QM_FILES}
)

# Windows-specific settings
if(WIN32)
    # Link dwmapi for dark mode detection
    target_link_libraries(QtInstaller PRIVATE dwmapi)

    # Set Windows executable properties
    set_target_properties(QtInstaller PROPERTIES
        WIN32_EXECUTABLE TRUE
        OUTPUT_NAME "QtInstaller"
    )

    # Optional: Add application icon
    # set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/installer.rc")
    # target_sources(QtInstaller PRIVATE ${APP_ICON_RESOURCE_WINDOWS})

    # Version information
    set_target_properties(QtInstaller PROPERTIES
        VERSION ${PROJECT_VERSION}
        COMPANY_NAME "Your Company"
        PRODUCT_NAME "Qt Installer Wizard"
        FILE_DESCRIPTION "Modern Qt-based Installer"
    )
endif()

# Installation rules
install(TARGETS QtInstaller
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Qt deployment (optional, for packaging)
if(WIN32)
    # Find windeployqt
    get_target_property(_qmake_executable Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET QtInstaller POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                --no-compiler-runtime
                --no-translations
                "$<TARGET_FILE:QtInstaller>"
            COMMENT "Running windeployqt..."
        )
    endif()

    # --- Locate 7-Zip and copy to output directory ---
    # 7za.exe must sit beside QtInstaller.exe at runtime so QProcess can find it.
    #
    # Priority 1: standalone 7za.exe placed in <source>/tools/ (no DLL needed)
    # Priority 2: 7za.exe from a standard 7-Zip installation
    # Priority 3: 7z.exe from a standard 7-Zip installation (copies 7z.dll too)
    find_program(SEVENZ_EXECUTABLE
        NAMES 7za.exe 7za
        PATHS
            "${CMAKE_CURRENT_SOURCE_DIR}/tools"
            "C:/Program Files/7-Zip"
            "C:/Program Files (x86)/7-Zip"
        NO_DEFAULT_PATH
    )

    if(NOT SEVENZ_EXECUTABLE)
        find_program(SEVENZ_EXECUTABLE
            NAMES 7z.exe 7z
            PATHS
                "C:/Program Files/7-Zip"
                "C:/Program Files (x86)/7-Zip"
            NO_DEFAULT_PATH
        )
        if(SEVENZ_EXECUTABLE)
            # 7z.exe requires 7z.dll in the same directory
            get_filename_component(_sevenz_dir "${SEVENZ_EXECUTABLE}" DIRECTORY)
            set(SEVENZ_DLL "${_sevenz_dir}/7z.dll")
        endif()
    endif()

    if(SEVENZ_EXECUTABLE)
        message(STATUS "7-Zip found: ${SEVENZ_EXECUTABLE}")
        add_custom_command(TARGET QtInstaller POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${SEVENZ_EXECUTABLE}"
                "$<TARGET_FILE_DIR:QtInstaller>/7za.exe"
            COMMENT "Copying 7-Zip executable to output..."
        )
        if(DEFINED SEVENZ_DLL AND EXISTS "${SEVENZ_DLL}")
            add_custom_command(TARGET QtInstaller POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${SEVENZ_DLL}"
                    "$<TARGET_FILE_DIR:QtInstaller>/7z.dll"
                COMMENT "Copying 7z.dll to output..."
            )
        endif()
    else()
        message(WARNING
            "7-Zip not found. The installer cannot extract the application archive without it.\n"
            "Fix options:\n"
            "  1. Install 7-Zip: https://www.7-zip.org/\n"
            "  2. Place standalone 7za.exe in: ${CMAKE_CURRENT_SOURCE_DIR}/tools/7za.exe")
    endif()
endif()
