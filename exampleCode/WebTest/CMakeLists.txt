CMAKE_MINIMUM_REQUIRED( VERSION 2.8 ) 
PROJECT( cvs1977 )

FIND_PACKAGE( raspicam REQUIRED )
FIND_PACKAGE( OpenCV REQUIRED )

SET( CMAKE_BUILD_TYPE Debug )

IF( OpenCV_FOUND AND raspicam_CV_FOUND )

    MESSAGE( STATUS "COMPILING WEB TESTS" )

    ADD_EXECUTABLE( cvs1977 Deamon.cpp )  
    TARGET_LINK_LIBRARIES( cvs1977 ${raspicam_CV_LIBS} )
    TARGET_LINK_LIBRARIES( cvs1977 -lmicrohttpd )
    TARGET_LINK_LIBRARIES( cvs1977 -ldaemon )

    CONFIGURE_FILE( ${CMAKE_SOURCE_DIR}/cvs1977_init.sh ${CMAKE_BINARY_DIR}/initscripts/cvs1977 @ONLY )

    INSTALL( PROGRAMS ${CMAKE_BINARY_DIR}/initscripts/cvs1977 DESTINATION /etc/init.d/ )
    INSTALL( TARGETS cvs1977 DESTINATION /usr/bin/ )
ELSE()

    MESSAGE(FATAL_ERROR "OPENCV NOT FOUND IN YOUR SYSTEM")

ENDIF()
