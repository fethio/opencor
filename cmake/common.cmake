MACRO(INITIALISE_PROJECT)
#    SET(CMAKE_VERBOSE_MAKEFILE ON)
    SET(CMAKE_INCLUDE_CURRENT_DIR ON)

    # Check whether we want some EXECUTE_PROCESS() calls to be OUTPUT_QUIET

    IF(SHOW_INFORMATION_MESSAGE)
        SET(OUTPUT_QUIET)
    ELSE()
        SET(OUTPUT_QUIET OUTPUT_QUIET)
    ENDIF()

    # Make sure that we are using the compiler we support

    IF(WIN32)
        IF(   NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC"
           OR NOT ${MSVC_VERSION} EQUAL 1800)
            MESSAGE(FATAL_ERROR "${CMAKE_PROJECT_NAME} can only be built using MSVC 2013 on Windows...")
        ENDIF()
    ELSEIF(APPLE)
        IF(    NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"
           AND NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
            MESSAGE(FATAL_ERROR "${CMAKE_PROJECT_NAME} can only be built using (Apple) Clang on OS X...")
        ENDIF()
    ELSE()
        IF(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
            MESSAGE(FATAL_ERROR "${CMAKE_PROJECT_NAME} can only be built using GCC on Linux...")
        ENDIF()
    ENDIF()

    # Make sure that we are building on a supported architecture
    # Note: normally, we would check the value of CMAKE_SIZEOF_VOID_P, but in
    #       some cases it may not be set (e.g. when generating an Xcode project
    #       file), so we determine and retrieve that value ourselves...

    TRY_RUN(ARCHITECTURE_RUN ARCHITECTURE_COMPILE
            ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/cmake/architecture.c
            RUN_OUTPUT_VARIABLE ARCHITECTURE)

    IF(NOT ARCHITECTURE_COMPILE)
        MESSAGE(FATAL_ERROR "We could not determine your architecture. Please clean your ${CMAKE_PROJECT_NAME} environment and try again...")
    ELSEIF(NOT ${ARCHITECTURE} EQUAL 64)
        MESSAGE(FATAL_ERROR "${CMAKE_PROJECT_NAME} can only be built in 64-bit mode...")
    ENDIF()

    # By default, we are building a release version of OpenCOR, unless we are
    # explicitly asked for a debug version

    IF("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        IF(SHOW_INFORMATION_MESSAGE)
            SET(BUILD_INFORMATION "Building a debug version of ${CMAKE_PROJECT_NAME}")
        ENDIF()

        SET(RELEASE_MODE FALSE)
    ELSE()
        IF(SHOW_INFORMATION_MESSAGE)
            SET(BUILD_INFORMATION "Building a release version of ${CMAKE_PROJECT_NAME}")
        ENDIF()

        SET(RELEASE_MODE TRUE)
    ENDIF()

    # Required packages

    SET(REQUIRED_QT_MODULES
        WebEngineWidgets
    )

    FOREACH(REQUIRED_QT_MODULE ${REQUIRED_QT_MODULES})
        FIND_PACKAGE(Qt5${REQUIRED_QT_MODULE} REQUIRED)
    ENDFOREACH()

    IF(ENABLE_TESTS)
        FIND_PACKAGE(Qt5Test REQUIRED)
    ENDIF()

    # Keep track of some information about Qt

    SET(QT_BINARY_DIR ${_qt5Widgets_install_prefix}/bin)
    SET(QT_LIBRARY_DIR ${_qt5Widgets_install_prefix}/lib)
    SET(QT_PLUGINS_DIR ${_qt5Widgets_install_prefix}/plugins)
    SET(QT_RESOURCES_DIR ${_qt5Widgets_install_prefix}/resources)
    SET(QT_WEBENGINE_TRANSLATIONS_DIR ${_qt5Widgets_install_prefix}/translations/qtwebengine_locales)
    SET(QT_VERSION ${Qt5Widgets_VERSION})
    SET(QT_VERSION_MAJOR ${Qt5Widgets_VERSION_MAJOR})
    SET(QT_VERSION_MINOR ${Qt5Widgets_VERSION_MINOR})
    SET(QT_VERSION_PATCH ${Qt5Widgets_VERSION_PATCH})

    # On OS X, keep track of the Qt libraries against which we need to link

    IF(APPLE)
        IF(ENABLE_TESTS)
            SET(QT_TEST QtTest)
        ELSE()
            SET(QT_TEST)
        ENDIF()

        SET(OS_X_QT_LIBRARIES
            QtCLucene
            QtConcurrent
            QtCore
            QtDBus
            QtGui
            QtHelp
            QtMacExtras
            QtMultimedia
            QtMultimediaWidgets
            QtNetwork
            QtOpenGL
            QtPositioning
            QtPrintSupport
            QtQml
            QtQuick
            QtSensors
            QtSql
            QtSvg
            ${QT_TEST}
            QtWebChannel
            QtWebEngineCore
            QtWebEngineWidgets
            QtWidgets
            QtXml
            QtXmlPatterns
        )
    ENDIF()

    # Determine the effective build directory

    SET(PROJECT_BUILD_DIR ${CMAKE_BINARY_DIR})

    IF(APPLE AND "${CMAKE_GENERATOR}" STREQUAL "Xcode")
        # With Xcode, we have a configuration directory, but it messes up our
        # build system, so ask for all the binaries to be generated in our build
        # folder

        SET(XCODE TRUE)

        SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BUILD_DIR})
        SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BUILD_DIR})
        SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BUILD_DIR})

        SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_BUILD_DIR})
        SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${PROJECT_BUILD_DIR})
        SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${PROJECT_BUILD_DIR})

        SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_BUILD_DIR})
        SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${PROJECT_BUILD_DIR})
        SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${PROJECT_BUILD_DIR})
    ELSE()
        # Check whether there is a configuration directory (the case with MSVC)
        # and, if so, make use of it

        SET(XCODE FALSE)

        IF(NOT "${CMAKE_CFG_INTDIR}" STREQUAL ".")
            SET(PROJECT_BUILD_DIR ${PROJECT_BUILD_DIR}/${CMAKE_CFG_INTDIR})
        ENDIF()
    ENDIF()

    # Some general build settings
    # Note: MSVC enables C++11 support by default, so we just need to enable it
    #       on Linux and OS X...

    IF(WIN32)
        SET(CMAKE_CXX_FLAGS "/DWIN32 /D_WINDOWS /W3 /WX /GR /EHsc")
        # Note: MSVC has a /Wall flag, but it results in MSVC being very
        #       pedantic, so instead we use what MSVC recommends for production
        #       code, which is /W3 and which is also what CMake uses by
        #       default...

        SET(LINK_FLAGS_PROPERTIES "/STACK:10000000")
    ELSE()
        SET(CMAKE_CXX_FLAGS "-Wall -W -Werror -std=c++11")

        IF(APPLE)
            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
            SET(LINK_FLAGS_PROPERTIES "-stdlib=libc++")
        ELSE()
            SET(LINK_FLAGS_PROPERTIES)
        ENDIF()
    ENDIF()

    # On OS X, we want to be able to access Cocoa

    IF(APPLE)
        SET(LINK_FLAGS_PROPERTIES "${LINK_FLAGS_PROPERTIES} -framework AppKit")
    ENDIF()

    # Some build settings that depend on whether we want a release or a debug
    # version of OpenCOR

    IF(RELEASE_MODE)
        # Default compiler and linker settings

        IF(WIN32)
            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DNDEBUG /MD /O2 /Ob2")
        ELSE()
            IF(APPLE)
                SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -flto")
            ELSE()
                SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
            ENDIF()

            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffast-math")
        ENDIF()

        IF(NOT WIN32 AND NOT APPLE)
            SET(LINK_FLAGS_PROPERTIES "${LINK_FLAGS_PROPERTIES} -Wl,-s")
            # Note #1: -Wl,-s strips all the symbols, thus reducing the final
            #          size of OpenCOR or one its shared libraries...
            # Note #2: the above linking option has become obsolete on OS X...
        ENDIF()

        # Make sure that debugging is off in Qt

        ADD_DEFINITIONS(-DQT_NO_DEBUG)
    ELSE()
        # Default compiler and linker settings

        IF(WIN32)
            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_DEBUG /MDd /Zi /Ob0 /Od /RTC1")
            SET(LINK_FLAGS_PROPERTIES "${LINK_FLAGS_PROPERTIES} /DEBUG")
        ELSE()
            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
        ENDIF()

        # Make sure that debugging is on in Qt

        ADD_DEFINITIONS(-DQT_DEBUG)
    ENDIF()

    # Disable a warning that occurs on (the 64-bit version of) Windows
    # Note: the warning occurs in (at least) MSVC's algorithm header file. To
    #       disable it here means that we disable it for everything, but is
    #       there another solution?...

    IF(WIN32)
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4267")
    ENDIF()

    # Ask for Unicode to be used

    ADD_DEFINITIONS(-DUNICODE)

    IF(WIN32)
        ADD_DEFINITIONS(-D_UNICODE)
    ENDIF()

    # Ask for Qt deprecated uses to be reported

    ADD_DEFINITIONS(-DQT_DEPRECATED_WARNINGS)

    # Sample plugins support, if requested

    IF(ENABLE_SAMPLES)
        ADD_DEFINITIONS(-DENABLE_SAMPLES)
    ENDIF()

    # On OS X, make sure that we support 10.8 and later, unless a specific
    # deployment target has been specified

    IF(APPLE)
        IF("${CMAKE_OSX_DEPLOYMENT_TARGET}" STREQUAL "")
            SET(CMAKE_OSX_DEPLOYMENT_TARGET 10.8)
        ENDIF()

        IF(SHOW_INFORMATION_MESSAGE)
            SET(BUILD_INFORMATION "${BUILD_INFORMATION} for (Mac) OS X ${CMAKE_OSX_DEPLOYMENT_TARGET} and later")
        ENDIF()
    ENDIF()

    # Destination of our plugins so that we don't have to deploy OpenCOR on
    # Windows and Linux before being able to test it

    IF(APPLE)
        SET(DEST_PLUGINS_DIR ${PROJECT_BUILD_DIR}/${CMAKE_PROJECT_NAME}.app/Contents/PlugIns/${CMAKE_PROJECT_NAME})
    ELSE()
        SET(DEST_PLUGINS_DIR ${PROJECT_BUILD_DIR}/plugins/${CMAKE_PROJECT_NAME})
    ENDIF()

    # Default location of external dependencies

    IF(WIN32)
        SET(PLATFORM_DIR windows)
    ELSEIF(APPLE)
        SET(PLATFORM_DIR osx)
    ELSE()
        SET(PLATFORM_DIR linux)
    ENDIF()

    IF(WIN32)
        IF(RELEASE_MODE)
            SET(REMOTE_EXTERNAL_BINARIES_DIR ${PLATFORM_DIR}/release)
            SET(LOCAL_EXTERNAL_BINARIES_DIR bin/release)
        ELSE()
            SET(REMOTE_EXTERNAL_BINARIES_DIR ${PLATFORM_DIR}/debug)
            SET(LOCAL_EXTERNAL_BINARIES_DIR bin/debug)
        ENDIF()
    ELSE()
        SET(REMOTE_EXTERNAL_BINARIES_DIR ${PLATFORM_DIR})
        SET(LOCAL_EXTERNAL_BINARIES_DIR bin)
    ENDIF()

    # Set the RPATH information on Linux and OS X

    IF(APPLE)
        SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

        IF(ENABLE_TRAVIS_CI)
            SET(CMAKE_INSTALL_RPATH "/usr/local/opt/qt5/Frameworks;@executable_path/../Frameworks;@executable_path/../PlugIns/${CMAKE_PROJECT_NAME}")
            # Note: on Travis CI, we don't deploy the Qt frameworks hence our
            #       RPATH refers to the system's Qt frameworks instead...
        ELSE()
            SET(CMAKE_INSTALL_RPATH "@executable_path/../Frameworks;@executable_path/../PlugIns/${CMAKE_PROJECT_NAME}")
        ENDIF()
    ELSEIF(NOT WIN32)
        SET(CMAKE_INSTALL_RPATH "$ORIGIN/../lib:$ORIGIN/../plugins/${CMAKE_PROJECT_NAME}")
    ENDIF()

    # Show the build information, if allowed

    IF(SHOW_INFORMATION_MESSAGE)
        MESSAGE("${BUILD_INFORMATION} using Qt ${QT_VERSION}...")
    ENDIF()
ENDMACRO()

#===============================================================================

MACRO(KEEP_TRACK_OF_FILE FILE_NAME)
    # Keep track of the given file
    # Note: indeed, some files (e.g. versiondate.txt) are 'manually' generated
    #       and then used to build other files. Now, the 'problem' is that
    #       Ninja needs to know about those files (see CMake policy CMP0058),
    #       which we ensure it does through the below...

    ADD_CUSTOM_COMMAND(OUTPUT ${FILE_NAME}
                       COMMAND ${CMAKE_COMMAND} -E sleep 0)
ENDMACRO()

#===============================================================================

MACRO(UPDATE_LANGUAGE_FILES TARGET_NAME)
    # Update the translation (.ts) files (if they exist) and generate the
    # language (.qm) files, which will later on be embedded in the project

    SET(LANGUAGES fr)
    SET(INPUT_FILES)

    FOREACH(INPUT_FILE ${ARGN})
        LIST(APPEND INPUT_FILES ${INPUT_FILE})
    ENDFOREACH()

    FOREACH(LANGUAGE ${LANGUAGES})
        SET(LANGUAGE_FILE ${TARGET_NAME}_${LANGUAGE})
        SET(TS_FILE i18n/${LANGUAGE_FILE}.ts)
        SET(QM_FILE ${PROJECT_BUILD_DIR}/${LANGUAGE_FILE}.qm)

        IF(EXISTS ${PROJECT_SOURCE_DIR}/${TS_FILE})
            EXECUTE_PROCESS(COMMAND ${QT_BINARY_DIR}/lupdate -no-obsolete ${INPUT_FILES}
                                                             -ts ${TS_FILE}
                            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                            ${OUTPUT_QUIET})
            EXECUTE_PROCESS(COMMAND ${QT_BINARY_DIR}/lrelease ${PROJECT_SOURCE_DIR}/${TS_FILE}
                                                          -qm ${QM_FILE}
                            ${OUTPUT_QUIET})

            KEEP_TRACK_OF_FILE(${QM_FILE})
        ENDIF()
    ENDFOREACH()
ENDMACRO()

#===============================================================================

MACRO(ADD_PLUGIN PLUGIN_NAME)
    # Various initialisations

    SET(PLUGIN_NAME ${PLUGIN_NAME})

    SET(SOURCES)
    SET(HEADERS_MOC)
    SET(UIS)
    SET(INCLUDE_DIRS)
    SET(DEFINITIONS)
    SET(PLUGINS)
    SET(PLUGIN_BINARIES)
    SET(QT_MODULES)
    SET(EXTERNAL_BINARIES_DIR)
    SET(EXTERNAL_BINARIES)
    SET(TESTS)

    # Analyse the extra parameters

    SET(TYPE_OF_PARAMETER 0)

    FOREACH(PARAMETER ${ARGN})
        IF("${PARAMETER}" STREQUAL "THIRD_PARTY")
            # Disable all C/C++ warnings since building a third-party plugin may
            # generate some and this has nothing to do with us
            # Note: on Windows, we can't simply add /w since it will otherwise
            #       result in MSVC complaining about /W3 having been overridden
            #       by /w...

            IF(WIN32)
                STRING(REPLACE "/W3" "/w"
                       CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
                STRING(REPLACE "/W3" "/w"
                       CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
            ELSE()
                SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w")
                SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
            ENDIF()

            # Add a definition in case of compilation from within Qt Creator
            # using MSVC and JOM since the latter overrides some of our settings

            IF(WIN32)
                ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
            ENDIF()
        ELSEIF("${PARAMETER}" STREQUAL "SOURCES")
            SET(TYPE_OF_PARAMETER 1)
        ELSEIF("${PARAMETER}" STREQUAL "HEADERS_MOC")
            SET(TYPE_OF_PARAMETER 2)
        ELSEIF("${PARAMETER}" STREQUAL "UIS")
            SET(TYPE_OF_PARAMETER 3)
        ELSEIF("${PARAMETER}" STREQUAL "INCLUDE_DIRS")
            SET(TYPE_OF_PARAMETER 4)
        ELSEIF("${PARAMETER}" STREQUAL "DEFINITIONS")
            SET(TYPE_OF_PARAMETER 5)
        ELSEIF("${PARAMETER}" STREQUAL "PLUGINS")
            SET(TYPE_OF_PARAMETER 6)
        ELSEIF("${PARAMETER}" STREQUAL "PLUGIN_BINARIES")
            SET(TYPE_OF_PARAMETER 7)
        ELSEIF("${PARAMETER}" STREQUAL "QT_MODULES")
            SET(TYPE_OF_PARAMETER 8)
        ELSEIF("${PARAMETER}" STREQUAL "EXTERNAL_BINARIES_DIR")
            SET(TYPE_OF_PARAMETER 9)
        ELSEIF("${PARAMETER}" STREQUAL "EXTERNAL_BINARIES")
            SET(TYPE_OF_PARAMETER 10)
        ELSEIF("${PARAMETER}" STREQUAL "TESTS")
            SET(TYPE_OF_PARAMETER 11)
        ELSE()
            # Not one of the headers, so add the parameter to the corresponding
            # set

            IF(${TYPE_OF_PARAMETER} EQUAL 1)
                LIST(APPEND SOURCES ${PARAMETER})
            ELSEIF(${TYPE_OF_PARAMETER} EQUAL 2)
                LIST(APPEND HEADERS_MOC ${PARAMETER})
            ELSEIF(${TYPE_OF_PARAMETER} EQUAL 3)
                LIST(APPEND UIS ${PARAMETER})
            ELSEIF(${TYPE_OF_PARAMETER} EQUAL 4)
                LIST(APPEND INCLUDE_DIRS ${PARAMETER})
            ELSEIF(${TYPE_OF_PARAMETER} EQUAL 5)
                LIST(APPEND DEFINITIONS ${PARAMETER})
            ELSEIF(${TYPE_OF_PARAMETER} EQUAL 6)
                LIST(APPEND PLUGINS ${PARAMETER})
            ELSEIF(${TYPE_OF_PARAMETER} EQUAL 7)
                LIST(APPEND PLUGIN_BINARIES ${PARAMETER})
            ELSEIF(${TYPE_OF_PARAMETER} EQUAL 8)
                LIST(APPEND QT_MODULES ${PARAMETER})
            ELSEIF(${TYPE_OF_PARAMETER} EQUAL 9)
                SET(EXTERNAL_BINARIES_DIR ${PARAMETER})
            ELSEIF(${TYPE_OF_PARAMETER} EQUAL 10)
                LIST(APPEND EXTERNAL_BINARIES ${PARAMETER})
            ELSEIF(${TYPE_OF_PARAMETER} EQUAL 11)
                LIST(APPEND TESTS ${PARAMETER})
            ENDIF()
        ENDIF()
    ENDFOREACH()

    # Various include directories

    SET(PLUGIN_INCLUDE_DIRS ${INCLUDE_DIRS} PARENT_SCOPE)

    INCLUDE_DIRECTORIES(${INCLUDE_DIRS})

    # Resource files, if any
    # Note: ideally, we would have our resource files named i18n.qrc.in and
    #       ui.qrc for all our plugins, but this is causing problems on Linux
    #       with Qt apparently accepting only one file called i18n.qrc.in or
    #       ui.qrc. The end result is that, on a development machine, plugin
    #       resources (e.g. icons, translations) are not available while,
    #       strangely enough, they are on a deployment machine...

    SET(RESOURCES)
    SET(I18N_QRC_IN_FILENAME ${PROJECT_SOURCE_DIR}/res/${PLUGIN_NAME}_i18n.qrc.in)
    SET(UI_QRC_FILENAME ${PROJECT_SOURCE_DIR}/res/${PLUGIN_NAME}_ui.qrc)

    IF(EXISTS ${I18N_QRC_IN_FILENAME})
        STRING(REPLACE "${CMAKE_SOURCE_DIR}" "${PROJECT_BUILD_DIR}"
               I18N_QRC_FILENAME "${PROJECT_SOURCE_DIR}/res/${PLUGIN_NAME}_i18n.qrc")

        CONFIGURE_FILE(${I18N_QRC_IN_FILENAME}
                       ${I18N_QRC_FILENAME})

        LIST(APPEND RESOURCES ${I18N_QRC_FILENAME})

        # Update the translation (.ts) files and generate the language (.qm)
        # files, which will later on be embedded in the plugin

        UPDATE_LANGUAGE_FILES(${PLUGIN_NAME} ${SOURCES} ${HEADERS_MOC} ${UIS})
    ENDIF()

    IF(EXISTS ${UI_QRC_FILENAME})
        LIST(APPEND RESOURCES ${UI_QRC_FILENAME})
    ENDIF()

    # Definition to make sure that the plugin can be used by other plugins

    ADD_DEFINITIONS(-D${PLUGIN_NAME}_PLUGIN)

    # Some plugin-specific definitions

    FOREACH(DEFINITION ${DEFINITIONS})
        ADD_DEFINITIONS(-D${DEFINITION})
    ENDFOREACH()

    # Generate and add the different files needed by the plugin

    IF("${HEADERS_MOC}" STREQUAL "")
        SET(SOURCES_MOC)
    ELSE()
        QT5_WRAP_CPP(SOURCES_MOC ${HEADERS_MOC})
    ENDIF()

    IF("${UIS}" STREQUAL "")
        SET(SOURCES_UIS)
    ELSE()
        QT5_WRAP_UI(SOURCES_UIS ${UIS})
    ENDIF()

    IF("${RESOURCES}" STREQUAL "")
        SET(SOURCES_RCS)
    ELSE()
        QT5_ADD_RESOURCES(SOURCES_RCS ${RESOURCES})
    ENDIF()

    ADD_LIBRARY(${PROJECT_NAME} SHARED
        ${SOURCES}
        ${SOURCES_MOC}
        ${SOURCES_UIS}
        ${SOURCES_RCS}
    )

    SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME ${PLUGIN_NAME}
        LINK_FLAGS "${LINK_FLAGS_PROPERTIES}"
    )

    # OpenCOR plugins

    FOREACH(PLUGIN ${PLUGINS})
        TARGET_LINK_LIBRARIES(${PROJECT_NAME}
            ${PLUGIN}Plugin
        )
    ENDFOREACH()

    # OpenCOR binaries

    FOREACH(PLUGIN_BINARY ${PLUGIN_BINARIES})
        TARGET_LINK_LIBRARIES(${PROJECT_NAME}
            ${PLUGIN_BINARY}
        )
    ENDFOREACH()

    # Qt modules

    FOREACH(QT_MODULE ${QT_MODULES})
        FIND_PACKAGE(Qt5${QT_MODULE} REQUIRED)

        TARGET_LINK_LIBRARIES(${PROJECT_NAME}
            Qt5::${QT_MODULE}
        )
    ENDFOREACH()

    # External binaries

    IF(WIN32)
        SET(DEST_EXTERNAL_BINARIES_DIR bin)
    ELSEIF(APPLE)
        SET(DEST_EXTERNAL_BINARIES_DIR ${CMAKE_PROJECT_NAME}.app/Contents/Frameworks)
    ELSE()
        SET(DEST_EXTERNAL_BINARIES_DIR lib)
    ENDIF()

    SET(FULL_DEST_EXTERNAL_BINARIES_DIR ${PROJECT_BUILD_DIR}/${DEST_EXTERNAL_BINARIES_DIR})

    IF(NOT EXISTS ${FULL_DEST_EXTERNAL_BINARIES_DIR})
        FILE(MAKE_DIRECTORY ${FULL_DEST_EXTERNAL_BINARIES_DIR})
    ENDIF()

    FOREACH(EXTERNAL_BINARY ${EXTERNAL_BINARIES})
        IF("${EXTERNAL_BINARIES_DIR}" STREQUAL "")
            SET(FULL_EXTERNAL_BINARY ${EXTERNAL_BINARY})
        ELSE()
            SET(FULL_EXTERNAL_BINARY "${EXTERNAL_BINARIES_DIR}/${EXTERNAL_BINARY}")
        ENDIF()

        IF(EXISTS ${FULL_EXTERNAL_BINARY})
            # Copy the external binary to its destination directory, so we can
            # test things without first having to deploy OpenCOR
            # Note: on Windows, we also need to copy the build directory so that
            #       we can test things from within Qt Creator...

            SET(REAL_EXTERNAL_BINARY ${EXTERNAL_BINARY})

            IF(WIN32)
                # On Windows, we need to replace the extension of the external
                # library

                STRING(REPLACE "${CMAKE_IMPORT_LIBRARY_SUFFIX}" "${CMAKE_SHARED_LIBRARY_SUFFIX}"
                       REAL_EXTERNAL_BINARY "${REAL_EXTERNAL_BINARY}")

                COPY_FILE_TO_BUILD_DIR(DIRECT_COPY ${EXTERNAL_BINARIES_DIR} . ${REAL_EXTERNAL_BINARY})
            ENDIF()

            COPY_FILE_TO_BUILD_DIR(DIRECT_COPY ${EXTERNAL_BINARIES_DIR} ${DEST_EXTERNAL_BINARIES_DIR} ${REAL_EXTERNAL_BINARY})

            # Link the plugin to the external library

            IF(WIN32)
                TARGET_LINK_LIBRARIES(${PROJECT_NAME}
                    ${FULL_EXTERNAL_BINARY}
                )
            ELSE()
                TARGET_LINK_LIBRARIES(${PROJECT_NAME}
                    ${FULL_DEST_EXTERNAL_BINARIES_DIR}/${EXTERNAL_BINARY}
                )
            ENDIF()
        ENDIF()
    ENDFOREACH()

    # Location of our plugin

    IF(XCODE)
        SET(PLUGIN_BUILD_DIR ${PROJECT_BUILD_DIR})
    ELSE()
        STRING(REPLACE "${${CMAKE_PROJECT_NAME}_SOURCE_DIR}/" ""
               PLUGIN_BUILD_DIR "${PROJECT_SOURCE_DIR}")

        SET(PLUGIN_BUILD_DIR ${CMAKE_BINARY_DIR}/${PLUGIN_BUILD_DIR})

        IF(NOT "${CMAKE_CFG_INTDIR}" STREQUAL ".")
            SET(PLUGIN_BUILD_DIR ${PLUGIN_BUILD_DIR}/${CMAKE_CFG_INTDIR})
        ENDIF()
    ENDIF()

    # Copy the plugin to our plugins directory
    # Note: this is done so that we can, on Windows and Linux, test the use of
    #       plugins in OpenCOR without first having to package OpenCOR, as well
    #       as, on Windows, test things from within Qt Creator...

    SET(PLUGIN_FILENAME ${CMAKE_SHARED_LIBRARY_PREFIX}${PLUGIN_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX})

    ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME} POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy ${PLUGIN_BUILD_DIR}/${PLUGIN_FILENAME}
                                                        ${DEST_PLUGINS_DIR}/${PLUGIN_FILENAME})

    # A few OS X specific things

    IF(APPLE)
        # Clean up our plugin

        OS_X_CLEAN_UP_FILE_WITH_QT_LIBRARIES(${PROJECT_NAME} ${DEST_PLUGINS_DIR} ${PLUGIN_FILENAME})

        # Make sure that the plugin refers to our embedded version of the
        # binary plugins on which it depends

        FOREACH(PLUGIN_BINARY ${PLUGIN_BINARIES})
            STRING(REGEX REPLACE "^.*/" "" PLUGIN_BINARY "${PLUGIN_BINARY}")

            ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME} POST_BUILD
                               COMMAND install_name_tool -change ${PLUGIN_BINARY}
                                                                 @rpath/${PLUGIN_BINARY}
                                                                 ${DEST_PLUGINS_DIR}/${PLUGIN_FILENAME})
        ENDFOREACH()

        # Make sure that the plugin refers to our embedded version of the
        # external binaries on which it depends

        FOREACH(EXTERNAL_BINARY ${EXTERNAL_BINARIES})
            ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME} POST_BUILD
                               COMMAND install_name_tool -change ${EXTERNAL_BINARY}
                                                                 @rpath/${EXTERNAL_BINARY}
                                                                 ${DEST_PLUGINS_DIR}/${PLUGIN_FILENAME})
        ENDFOREACH()
    ENDIF()

    # Package the plugin, but only if we are not on OS X since it will have
    # already been copied

    IF(NOT APPLE)
        INSTALL(FILES ${PLUGIN_BUILD_DIR}/${PLUGIN_FILENAME}
                DESTINATION plugins/${CMAKE_PROJECT_NAME})
    ENDIF()

    # Create some tests, if any and if required

    IF(ENABLE_TESTS)
        FOREACH(TEST ${TESTS})
            # Keep track of the test (for later use by our main test program)

            FILE(APPEND ${TESTS_LIST_FILENAME} "${PLUGIN_NAME}|${TEST}|")

            # Build our test, if possible

            SET(TEST_NAME ${PLUGIN_NAME}_${TEST})

            SET(TEST_SOURCE tests/${TEST}.cpp)
            SET(TEST_HEADER_MOC tests/${TEST}.h)

            IF(    EXISTS ${PROJECT_SOURCE_DIR}/${TEST_SOURCE}
               AND EXISTS ${PROJECT_SOURCE_DIR}/${TEST_HEADER_MOC})
                # The test exists, so build it

                SET(TEST_SOURCES_MOC)
                # Note: we need to initialise TEST_SOURCES_MOC in case there is
                #       more than just one test. Indeed, if we were not to do
                #       that initialisation, then TEST_SOURCES_MOC would include
                #       the information of all the tests up to the one we want
                #       build...

                QT5_WRAP_CPP(TEST_SOURCES_MOC
                    ${TEST_HEADER_MOC}
                )

                QT5_ADD_RESOURCES(TEST_SOURCES_RCS ${TESTS_QRC_FILENAME})

                ADD_EXECUTABLE(${TEST_NAME}
                    ../../../tests/src/testsutils.cpp

                    ${SOURCES}
                    ${SOURCES_MOC}
                    ${SOURCES_UIS}
                    ${SOURCES_RCS}

                    ${TEST_SOURCE}
                    ${TEST_SOURCES_MOC}
                    ${TEST_SOURCES_RCS}
                )

                SET_TARGET_PROPERTIES(${TEST_NAME} PROPERTIES
                    OUTPUT_NAME ${TEST_NAME}
                    LINK_FLAGS "${LINK_FLAGS_PROPERTIES}"
                )

                # Plugins

                FOREACH(PLUGIN ${PLUGINS})
                    TARGET_LINK_LIBRARIES(${TEST_NAME}
                        ${PLUGIN}Plugin
                    )
                ENDFOREACH()

                # OpenCOR binaries

                FOREACH(PLUGIN_BINARY ${PLUGIN_BINARIES})
                    TARGET_LINK_LIBRARIES(${TEST_NAME}
                        ${PLUGIN_BINARY}
                    )
                ENDFOREACH()

                # Qt modules

                FOREACH(QT_MODULE ${QT_MODULES} Test)
                    TARGET_LINK_LIBRARIES(${TEST_NAME}
                        Qt5::${QT_MODULE}
                    )
                ENDFOREACH()

                # External binaries

                FOREACH(EXTERNAL_BINARY ${EXTERNAL_BINARIES})
                    IF("${EXTERNAL_BINARIES_DIR}" STREQUAL "")
                        SET(FULL_EXTERNAL_BINARY ${EXTERNAL_BINARY})
                    ELSE()
                        SET(FULL_EXTERNAL_BINARY "${EXTERNAL_BINARIES_DIR}/${EXTERNAL_BINARY}")
                    ENDIF()

                    IF(EXISTS ${FULL_EXTERNAL_BINARY})
                        IF(WIN32)
                            TARGET_LINK_LIBRARIES(${TEST_NAME}
                                ${FULL_EXTERNAL_BINARY}
                            )
                        ELSE()
                            TARGET_LINK_LIBRARIES(${TEST_NAME}
                                ${FULL_DEST_EXTERNAL_BINARIES_DIR}/${EXTERNAL_BINARY}
                            )
                        ENDIF()
                    ENDIF()
                ENDFOREACH()

                # Copy the test to our tests directory
                # Note: DEST_TESTS_DIR is defined in our main CMake file...

                SET(TEST_FILENAME ${TEST_NAME}${CMAKE_EXECUTABLE_SUFFIX})

                IF(WIN32)
                    ADD_CUSTOM_COMMAND(TARGET ${TEST_NAME} POST_BUILD
                                       COMMAND ${CMAKE_COMMAND} -E copy ${PLUGIN_BUILD_DIR}/${TEST_FILENAME}
                                                                        ${PROJECT_BUILD_DIR}/${TEST_FILENAME})
                ENDIF()

                ADD_CUSTOM_COMMAND(TARGET ${TEST_NAME} POST_BUILD
                                   COMMAND ${CMAKE_COMMAND} -E copy ${PLUGIN_BUILD_DIR}/${TEST_FILENAME}
                                                                    ${DEST_TESTS_DIR}/${TEST_FILENAME})

                # A few OS X specific things

                IF(APPLE)
                    # Clean up our plugin's tests

                    OS_X_CLEAN_UP_FILE_WITH_QT_LIBRARIES(${TEST_NAME} ${DEST_TESTS_DIR} ${TEST_FILENAME})

                    # Make sure that the plugin's tests refer to our embedded
                    # version of the binary plugins on which they depend

                    FOREACH(PLUGIN_BINARY ${PLUGIN_BINARIES})
                        STRING(REGEX REPLACE "^.*/" "" PLUGIN_BINARY "${PLUGIN_BINARY}")

                        ADD_CUSTOM_COMMAND(TARGET ${TEST_NAME} POST_BUILD
                                           COMMAND install_name_tool -change ${PLUGIN_BINARY}
                                                                             @rpath/${PLUGIN_BINARY}
                                                                             ${DEST_TESTS_DIR}/${TEST_FILENAME})
                    ENDFOREACH()

                    # Make sure that the plugin's tests refer to our embedded
                    # version of the external binaries on which they depend

                    FOREACH(EXTERNAL_BINARY ${EXTERNAL_BINARIES})
                        ADD_CUSTOM_COMMAND(TARGET ${TEST_NAME} POST_BUILD
                                           COMMAND install_name_tool -change ${EXTERNAL_BINARY}
                                                                             @rpath/${EXTERNAL_BINARY}
                                                                             ${DEST_TESTS_DIR}/${TEST_FILENAME})
                    ENDFOREACH()
                ENDIF()
            ELSE()
                MESSAGE(AUTHOR_WARNING "The '${TEST}' test for the '${PLUGIN_NAME}' plugin doesn't exist...")
            ENDIF()
        ENDFOREACH()
    ENDIF()
ENDMACRO()

#===============================================================================

MACRO(ADD_PLUGIN_BINARY PLUGIN_NAME)
    # Various initialisations

    SET(PLUGIN_NAME ${PLUGIN_NAME})

    SET(INCLUDE_DIRS)

    # Analyse the extra parameters

    SET(TYPE_OF_PARAMETER 0)

    FOREACH(PARAMETER ${ARGN})
        IF("${PARAMETER}" STREQUAL "INCLUDE_DIRS")
            SET(TYPE_OF_PARAMETER 1)
        ELSE()
            # Not one of the headers, so add the parameter to the corresponding
            # set

            IF(${TYPE_OF_PARAMETER} EQUAL 1)
                LIST(APPEND INCLUDE_DIRS ${PARAMETER})
            ENDIF()
        ENDIF()
    ENDFOREACH()

    # Various include directories

    SET(PLUGIN_INCLUDE_DIRS ${INCLUDE_DIRS} PARENT_SCOPE)

    INCLUDE_DIRECTORIES(${INCLUDE_DIRS})

    # Location of our plugins

    SET(PLUGIN_BINARY_DIR ${PROJECT_SOURCE_DIR}/${LOCAL_EXTERNAL_BINARIES_DIR})

    # Copy the plugin to our plugins directory
    # Note: this is done so that we can, on Windows and Linux, test the use of
    #       plugins in OpenCOR without first having to package and deploy
    #       everything...

    SET(PLUGIN_FILENAME ${CMAKE_SHARED_LIBRARY_PREFIX}${PLUGIN_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX})

    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy ${PLUGIN_BINARY_DIR}/${PLUGIN_FILENAME}
                                                     ${DEST_PLUGINS_DIR}/${PLUGIN_FILENAME})

    # Package the plugin, but only if we are not on OS X since it will have
    # already been copied

    IF(NOT APPLE)
        INSTALL(FILES ${PLUGIN_BINARY_DIR}/${PLUGIN_FILENAME}
                DESTINATION plugins/${CMAKE_PROJECT_NAME})
    ENDIF()
ENDMACRO()

#===============================================================================

MACRO(RETRIEVE_CONFIG_FILES)
    FOREACH(CONFIG_FILE ${ARGN})
        STRING(REPLACE "PLATFORM_DIR/" "${PLATFORM_DIR}/"
               CONFIG_FILE_ORIG "${CONFIG_FILE}")
        STRING(REPLACE "PLATFORM_DIR/" ""
               CONFIG_FILE_DEST "${CONFIG_FILE}")

        EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/${CONFIG_FILE_ORIG}
                                                         ${PROJECT_SOURCE_DIR}/${CONFIG_FILE_DEST})
    ENDFOREACH()
ENDMACRO()

#===============================================================================

MACRO(COPY_FILE_TO_BUILD_DIR PROJECT_TARGET ORIG_DIRNAME DEST_DIRNAME FILENAME)
    # Copy the file (renaming it, if needed) to the destination folder
    # Note: DIRECT_COPY is used to copy a file that doesn't first need to be
    #       built. This means that we can then use EXECUTE_PROCESS() rather than
    #       ADD_CUSTOM_COMMAND(), and thus reduce the length of the
    #       PROJECT_TARGET command, something that can be useful on Windows
    #       since the command might otherwise end up being too long for Windows
    #       to handle...

    IF("${ARGN}" STREQUAL "")
        IF("${PROJECT_TARGET}" STREQUAL "DIRECT_COPY")
            EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy ${ORIG_DIRNAME}/${FILENAME}
                                                             ${PROJECT_BUILD_DIR}/${DEST_DIRNAME}/${FILENAME})
        ELSE()
            ADD_CUSTOM_COMMAND(TARGET ${PROJECT_TARGET} POST_BUILD
                               COMMAND ${CMAKE_COMMAND} -E copy ${ORIG_DIRNAME}/${FILENAME}
                                                                ${PROJECT_BUILD_DIR}/${DEST_DIRNAME}/${FILENAME})
        ENDIF()
    ELSE()
        # An argument was passed so use it to rename the file, which is to be
        # copied

        IF("${PROJECT_TARGET}" STREQUAL "DIRECT_COPY")
            EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy ${ORIG_DIRNAME}/${FILENAME}
                                                             ${PROJECT_BUILD_DIR}/${DEST_DIRNAME}/${ARGN})
        ELSE()
            ADD_CUSTOM_COMMAND(TARGET ${PROJECT_TARGET} POST_BUILD
                               COMMAND ${CMAKE_COMMAND} -E copy ${ORIG_DIRNAME}/${FILENAME}
                                                                ${PROJECT_BUILD_DIR}/${DEST_DIRNAME}/${ARGN})
        ENDIF()
    ENDIF()
ENDMACRO()

#===============================================================================

MACRO(WINDOWS_DEPLOY_QT_LIBRARIES)
    FOREACH(LIBRARY ${ARGN})
        # Copy the Qt library to both the build and build/bin folders, so we can
        # test things both from within Qt Creator and without first having to
        # deploy OpenCOR

        SET(LIBRARY_RELEASE_FILENAME ${CMAKE_SHARED_LIBRARY_PREFIX}${LIBRARY}${CMAKE_SHARED_LIBRARY_SUFFIX})
        SET(LIBRARY_DEBUG_FILENAME ${CMAKE_SHARED_LIBRARY_PREFIX}${LIBRARY}d${CMAKE_SHARED_LIBRARY_SUFFIX})

        IF(NOT EXISTS ${QT_BINARY_DIR}/${LIBRARY_DEBUG_FILENAME})
            # No debug version of the Qt library exists, so use its release
            # version instead

            SET(LIBRARY_DEBUG_FILENAME ${LIBRARY_RELEASE_FILENAME})
        ENDIF()

        IF(RELEASE_MODE)
            SET(LIBRARY_FILENAME ${LIBRARY_RELEASE_FILENAME})
        ELSE()
            SET(LIBRARY_FILENAME ${LIBRARY_DEBUG_FILENAME})
        ENDIF()

        COPY_FILE_TO_BUILD_DIR(DIRECT_COPY ${QT_BINARY_DIR} . ${LIBRARY_FILENAME})
        COPY_FILE_TO_BUILD_DIR(DIRECT_COPY ${QT_BINARY_DIR} bin ${LIBRARY_FILENAME})

        # Deploy the Qt library

        INSTALL(FILES ${QT_BINARY_DIR}/${LIBRARY_FILENAME}
                DESTINATION bin)
    ENDFOREACH()
ENDMACRO()

#===============================================================================

MACRO(WINDOWS_DEPLOY_QT_PLUGIN PLUGIN_CATEGORY)
    FOREACH(PLUGIN_NAME ${ARGN})
        # Copy the Qt plugin to the plugins folder

        SET(PLUGIN_ORIG_DIRNAME ${QT_PLUGINS_DIR}/${PLUGIN_CATEGORY})
        SET(PLUGIN_DEST_DIRNAME plugins/${PLUGIN_CATEGORY})
        SET(PLUGIN_RELEASE_FILENAME ${CMAKE_SHARED_LIBRARY_PREFIX}${PLUGIN_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX})
        SET(PLUGIN_DEBUG_FILENAME ${CMAKE_SHARED_LIBRARY_PREFIX}${PLUGIN_NAME}d${CMAKE_SHARED_LIBRARY_SUFFIX})

        IF(RELEASE_MODE)
            SET(PLUGIN_FILENAME ${PLUGIN_RELEASE_FILENAME})
        ELSE()
            SET(PLUGIN_FILENAME ${PLUGIN_DEBUG_FILENAME})
        ENDIF()

        COPY_FILE_TO_BUILD_DIR(DIRECT_COPY ${PLUGIN_ORIG_DIRNAME} ${PLUGIN_DEST_DIRNAME} ${PLUGIN_FILENAME})

        # Deploy the Qt plugin

        INSTALL(FILES ${PLUGIN_ORIG_DIRNAME}/${PLUGIN_FILENAME}
                DESTINATION ${PLUGIN_DEST_DIRNAME})
    ENDFOREACH()
ENDMACRO()

#===============================================================================

MACRO(WINDOWS_DEPLOY_QT_WEB_ENGINE_PROCESS)
#---ISSUE908--- TO BE DONE...

    # Copy (and deploy) the Qt WebEngine process to both the build and build/bin
    # folders, so we can test things both from within Qt Creator and without
    # first having to deploy OpenCOR

    SET(QT_WEB_ENGINE_PROCESS_FILENAME QtWebEngineProcess.exe)

    COPY_FILE_TO_BUILD_DIR(DIRECT_COPY ${QT_BINARY_DIR} . ${QT_WEB_ENGINE_PROCESS_FILENAME})
    COPY_FILE_TO_BUILD_DIR(DIRECT_COPY ${QT_BINARY_DIR} bin ${QT_WEB_ENGINE_PROCESS_FILENAME})

    INSTALL(FILES ${QT_BINARY_DIR}/${QT_WEB_ENGINE_PROCESS_FILENAME}
            DESTINATION bin)

    # Do the same with Qt WebEngine's resources

    SET(RESOURCES_DIR resources)

    FILE(GLOB RESOURCE_FILEPATHS ${QT_RESOURCES_DIR}/*.*)

    FOREACH(RESOURCE_FILEPATH ${RESOURCE_FILEPATHS})
        GET_FILENAME_COMPONENT(RESOURCE_FILENAME ${RESOURCE_FILEPATH} NAME)

        COPY_FILE_TO_BUILD_DIR(DIRECT_COPY ${QT_RESOURCES_DIR} ./${RESOURCES_DIR} ${RESOURCE_FILENAME})
        COPY_FILE_TO_BUILD_DIR(DIRECT_COPY ${QT_RESOURCES_DIR} bin/${RESOURCES_DIR} ${RESOURCE_FILENAME})

        INSTALL(FILES ${QT_RESOURCES_DIR}/${RESOURCE_FILENAME}
                DESTINATION bin/${RESOURCES_DIR})
    ENDFOREACH()

    # Do the same with Qt WebEngine's translations

    SET(WEBENGINE_TRANSLATIONS_DIR translations/qtwebengine_locales)

    FILE(GLOB TRANSLATION_FILEPATHS ${QT_WEBENGINE_TRANSLATIONS_DIR}/*.*)

    FOREACH(TRANSLATION_FILEPATH ${TRANSLATION_FILEPATHS})
        GET_FILENAME_COMPONENT(TRANSLATION_FILENAME ${TRANSLATION_FILEPATH} NAME)

        COPY_FILE_TO_BUILD_DIR(DIRECT_COPY ${QT_WEBENGINE_TRANSLATIONS_DIR} ./${WEBENGINE_TRANSLATIONS_DIR} ${TRANSLATION_FILENAME})
        COPY_FILE_TO_BUILD_DIR(DIRECT_COPY ${QT_WEBENGINE_TRANSLATIONS_DIR} bin/${WEBENGINE_TRANSLATIONS_DIR} ${TRANSLATION_FILENAME})

        INSTALL(FILES ${QT_WEBENGINE_TRANSLATIONS_DIR}/${TRANSLATION_FILENAME}
                DESTINATION bin/${WEBENGINE_TRANSLATIONS_DIR})
    ENDFOREACH()
ENDMACRO()

#===============================================================================

MACRO(WINDOWS_DEPLOY_LIBRARY DIRNAME FILENAME)
    # Deploy the library file

    INSTALL(FILES ${DIRNAME}/${FILENAME}
            DESTINATION bin)
ENDMACRO()

#===============================================================================

MACRO(LINUX_DEPLOY_QT_LIBRARY DIRNAME ORIG_FILENAME DEST_FILENAME)
    # Copy the Qt library to the build/lib folder, so we can test things without
    # first having to deploy OpenCOR
    # Note: this is particularly useful when the Linux machine has different
    #       versions of Qt...

    COPY_FILE_TO_BUILD_DIR(DIRECT_COPY ${DIRNAME} lib ${ORIG_FILENAME} ${DEST_FILENAME})

    # Strip the library of all its local symbols

    IF(RELEASE_MODE)
        ADD_CUSTOM_COMMAND(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
                           COMMAND strip -x lib/${DEST_FILENAME})
    ENDIF()

    # Deploy the Qt library

    INSTALL(FILES ${PROJECT_BUILD_DIR}/lib/${DEST_FILENAME}
            DESTINATION lib)
ENDMACRO()

#===============================================================================

MACRO(LINUX_DEPLOY_QT_PLUGIN PLUGIN_CATEGORY)
    FOREACH(PLUGIN_NAME ${ARGN})
        # Copy the Qt plugin to the plugins folder

        SET(PLUGIN_ORIG_DIRNAME ${QT_PLUGINS_DIR}/${PLUGIN_CATEGORY})
        SET(PLUGIN_DEST_DIRNAME plugins/${PLUGIN_CATEGORY})
        SET(PLUGIN_FILENAME ${CMAKE_SHARED_LIBRARY_PREFIX}${PLUGIN_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX})

        COPY_FILE_TO_BUILD_DIR(DIRECT_COPY ${PLUGIN_ORIG_DIRNAME} ${PLUGIN_DEST_DIRNAME} ${PLUGIN_FILENAME})

        # Strip the Qt plugin of all its local symbols

        IF(RELEASE_MODE)
            ADD_CUSTOM_COMMAND(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
                               COMMAND strip -x ${PLUGIN_DEST_DIRNAME}/${PLUGIN_FILENAME})
        ENDIF()

        # Deploy the Qt plugin

        INSTALL(FILES ${PROJECT_BUILD_DIR}/${PLUGIN_DEST_DIRNAME}/${PLUGIN_FILENAME}
                DESTINATION ${PLUGIN_DEST_DIRNAME})
    ENDFOREACH()
ENDMACRO()

#===============================================================================

MACRO(LINUX_DEPLOY_QT_WEB_ENGINE_PROCESS)
#---ISSUE908--- TO BE DONE...
ENDMACRO()

#===============================================================================

MACRO(LINUX_DEPLOY_LIBRARY DIRNAME FILENAME)
    # Strip the library of all its local symbols

    IF(RELEASE_MODE)
        ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME} POST_BUILD
                           COMMAND strip -x ${DIRNAME}/${FILENAME})
    ENDIF()

    # Deploy the library file

    INSTALL(FILES ${DIRNAME}/${FILENAME}
            DESTINATION lib)
ENDMACRO()

#===============================================================================

MACRO(OS_X_CLEAN_UP_FILE_WITH_QT_LIBRARIES PROJECT_TARGET DIRNAME FILENAME)
    # Strip the Qt file of all its local symbols

    SET(FULL_FILENAME ${DIRNAME}/${FILENAME})

    IF(RELEASE_MODE)
        ADD_CUSTOM_COMMAND(TARGET ${PROJECT_TARGET} POST_BUILD
                           COMMAND strip -x ${FULL_FILENAME})
    ENDIF()

    # Clean up the Qt file's id

    ADD_CUSTOM_COMMAND(TARGET ${PROJECT_TARGET} POST_BUILD
                       COMMAND install_name_tool -id ${FILENAME}
                                                     ${FULL_FILENAME})
ENDMACRO()

#===============================================================================

MACRO(OS_X_DEPLOY_QT_FILE ORIG_DIRNAME DEST_DIRNAME FILENAME)
    # Copy the Qt file

    ADD_CUSTOM_COMMAND(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy ${ORIG_DIRNAME}/${FILENAME}
                                                        ${DEST_DIRNAME}/${FILENAME})

    # Clean up the Qt file

    OS_X_CLEAN_UP_FILE_WITH_QT_LIBRARIES(${CMAKE_PROJECT_NAME} ${DEST_DIRNAME} ${FILENAME})
ENDMACRO()

#===============================================================================

MACRO(OS_X_DEPLOY_QT_LIBRARIES)
    FOREACH(LIBRARY_NAME ${ARGN})
        # Deploy the Qt library

        SET(QT_FRAMEWORK_DIR ${LIBRARY_NAME}.framework/Versions/${QT_VERSION_MAJOR})

        OS_X_DEPLOY_QT_FILE(${QT_LIBRARY_DIR}/${QT_FRAMEWORK_DIR}
                            ${PROJECT_BUILD_DIR}/${CMAKE_PROJECT_NAME}.app/Contents/Frameworks/${QT_FRAMEWORK_DIR}
                            ${LIBRARY_NAME})
    ENDFOREACH()
ENDMACRO()

#===============================================================================

MACRO(OS_X_DEPLOY_QT_PLUGIN PLUGIN_CATEGORY)
    FOREACH(PLUGIN_NAME ${ARGN})
        # Deploy the Qt plugin

        OS_X_DEPLOY_QT_FILE(${QT_PLUGINS_DIR}/${PLUGIN_CATEGORY}
                            ${PROJECT_BUILD_DIR}/${CMAKE_PROJECT_NAME}.app/Contents/PlugIns/${PLUGIN_CATEGORY}
                            ${CMAKE_SHARED_LIBRARY_PREFIX}${PLUGIN_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX})
    ENDFOREACH()
ENDMACRO()

#===============================================================================

MACRO(OS_X_DEPLOY_QT_WEB_ENGINE_PROCESS)
    # Retrieve the Helpers and Resources folders from the original
    # QtWebEngineCore framework and create a symbolic link for them

    SET(VERSION_DIR Versions/${QT_VERSION_MAJOR})
    SET(QT_WEB_ENGINE_CORE_DIR QtWebEngineCore.framework)
    SET(QT_WEB_ENGINE_CORE_VERSION_DIR ${QT_WEB_ENGINE_CORE_DIR}/${VERSION_DIR})
    SET(LOCAL_FRAMEWORKS_DIR ${PROJECT_BUILD_DIR}/${CMAKE_PROJECT_NAME}.app/Contents/Frameworks)
    SET(LOCAL_QT_WEB_ENGINE_CORE_DIR ${LOCAL_FRAMEWORKS_DIR}/${QT_WEB_ENGINE_CORE_DIR})
    SET(LOCAL_QT_WEB_ENGINE_CORE_VERSION_DIR ${LOCAL_FRAMEWORKS_DIR}/${QT_WEB_ENGINE_CORE_VERSION_DIR})

    FOREACH(DIRNAME Helpers Resources)
        ADD_CUSTOM_COMMAND(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy_directory ${QT_LIBRARY_DIR}/${QT_WEB_ENGINE_CORE_VERSION_DIR}/${DIRNAME}
                                                                      ${LOCAL_QT_WEB_ENGINE_CORE_VERSION_DIR}/${DIRNAME})

        ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME} POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E create_symlink ${VERSION_DIR}/${DIRNAME}
                                                                      ${DIRNAME}
                           WORKING_DIRECTORY ${LOCAL_QT_WEB_ENGINE_CORE_DIR})
    ENDFOREACH()
ENDMACRO()

#===============================================================================

MACRO(OS_X_DEPLOY_LIBRARY DIRNAME LIBRARY_NAME)
    # Strip the library of all its local symbols

    SET(LIBRARY_FILEPATH ${PROJECT_BUILD_DIR}/${CMAKE_PROJECT_NAME}.app/Contents/Frameworks/${CMAKE_SHARED_LIBRARY_PREFIX}${LIBRARY_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX})

    IF(RELEASE_MODE)
        ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME} POST_BUILD
                           COMMAND strip -x ${LIBRARY_FILEPATH})
    ENDIF()

    # Make sure that the library refers to our embedded version of the libraries
    # on which it depends

    FOREACH(DEPENDENCY_NAME ${ARGN})
        SET(DEPENDENCY_FILENAME ${CMAKE_SHARED_LIBRARY_PREFIX}${DEPENDENCY_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX})

        ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME} POST_BUILD
                           COMMAND install_name_tool -change ${DEPENDENCY_FILENAME}
                                                             @rpath/${DEPENDENCY_FILENAME}
                                                             ${LIBRARY_FILEPATH})
    ENDFOREACH()
ENDMACRO()

#===============================================================================

MACRO(RETRIEVE_BINARY_FILE_FROM LOCATION DIRNAME FILENAME SHA1_VALUE)
    # Create the destination folder, if needed

    STRING(REPLACE "${PLATFORM_DIR}" "bin"
           REAL_DIRNAME "${CMAKE_SOURCE_DIR}/${DIRNAME}")

    IF(NOT EXISTS ${REAL_DIRNAME})
        FILE(MAKE_DIRECTORY ${REAL_DIRNAME})
    ENDIF()

    # Make sure that the file, if it exists, has the expected SHA-1 value

    SET(REAL_FILENAME ${REAL_DIRNAME}/${FILENAME})

    IF(EXISTS ${REAL_FILENAME})
        FILE(SHA1 ${REAL_FILENAME} REAL_SHA1_VALUE)

        IF(NOT "${REAL_SHA1_VALUE}" STREQUAL "${SHA1_VALUE}")
            # The file doesn't have the expected SHA-1 value, so remove it

            FILE(REMOVE ${REAL_FILENAME})
        ENDIF()
    ENDIF()

    # Retrieve the file from the OpenCOR website, if needed
    # Note: we would normally provide the SHA-1 value to the FILE(DOWNLOAD)
    #       call, but this would create an empty file to start with and if the
    #       file cannot be downloaded for some reason or another, then we would
    #       still have that file and CMake would then complain about its SHA-1
    #       value being wrong (as well as not being able to download the file),
    #       so we handle everything ourselves...

    IF(NOT EXISTS ${REAL_FILENAME})
        IF(SHOW_INFORMATION_MESSAGE)
            MESSAGE("Retrieving '${DIRNAME}/${FILENAME}'...")
        ENDIF()

        # We retrieve the compressed version of the file

        SET(COMPRESSED_FILENAME ${FILENAME}.tar.gz)
        SET(REAL_COMPRESSED_FILENAME ${REAL_DIRNAME}/${COMPRESSED_FILENAME})

        FILE(DOWNLOAD "${LOCATION}/${DIRNAME}/${COMPRESSED_FILENAME}" ${REAL_COMPRESSED_FILENAME}
             SHOW_PROGRESS STATUS STATUS)

        # Uncompress the compressed version of the file, should we have managed
        # to retrieve it

        LIST(GET STATUS 0 STATUS_CODE)

        IF(${STATUS_CODE} EQUAL 0)
            EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E tar -xzf ${REAL_COMPRESSED_FILENAME}
                            WORKING_DIRECTORY ${REAL_DIRNAME} OUTPUT_QUIET)
            FILE(REMOVE ${REAL_COMPRESSED_FILENAME})
        ELSE()
            FILE(REMOVE ${REAL_COMPRESSED_FILENAME})
            # Note: this is in case we had an HTTP error of sorts, in which case
            #       we would end up with an empty file...

            MESSAGE(FATAL_ERROR "The compressed version of ${FILENAME} could not be retrieved...")
        ENDIF()

        # Check that the file, if we managed to retrieve it, has the expected
        # SHA-1 value

        IF(EXISTS ${REAL_FILENAME})
            FILE(SHA1 ${REAL_FILENAME} REAL_SHA1_VALUE)

            IF(NOT "${REAL_SHA1_VALUE}" STREQUAL "${SHA1_VALUE}")
                FILE(REMOVE ${REAL_FILENAME})

                MESSAGE(FATAL_ERROR "${FILENAME} does not have the expected SHA-1 value...")
            ENDIF()
        ELSE()
            FILE(REMOVE ${REAL_COMPRESSED_FILENAME})

            MESSAGE(FATAL_ERROR "${FILENAME} could not be uncompressed...")
        ENDIF()
    ENDIF()
ENDMACRO()

#===============================================================================

MACRO(RETRIEVE_BINARY_FILE DIRNAME FILENAME SHA1_VALUE)
    RETRIEVE_BINARY_FILE_FROM("http://www.opencor.ws/binaries" ${DIRNAME} ${FILENAME} ${SHA1_VALUE})
ENDMACRO()
