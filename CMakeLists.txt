CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
# set project's name
PROJECT(Swiff)
# find qt package
FIND_PACKAGE(Qt4 REQUIRED)
# set include directories
include_directories (${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/src)
# include qt files
SET (QT_USE_QTNETWORK TRUE)
SET (QT_USE_QTSVG TRUE)
INCLUDE(${QT_USE_FILE})
#add compilation definitions
ADD_DEFINITIONS(-Wall -O3)
# set library sources
SET(SRCS
  src/AboutDialog.cpp
  src/LicenseWindow.cpp
  src/MainWindow.cpp
  src/Main.cpp
  src/PreviewWidget.cpp
  src/SettingsDialog.cpp
  src/SwiffExporter.cpp
  src/SwiffABCData.cpp
  src/SwiffAction.cpp
  src/SwiffActionScriptDecompiler.cpp
  src/SwiffActionScriptEngine.cpp
  src/SwiffBitmap.cpp
  src/SwiffBitStream.cpp
  src/SwiffButton.cpp
  src/SwiffDoABC.cpp
  src/SwiffDocument.cpp
  src/SwiffEdge.cpp
  src/SwiffExpression.cpp
  src/SwiffFilter.cpp
  src/SwiffFrame.cpp
  src/SwiffFont.cpp
  src/SwiffPlaceObject.cpp
  src/SwiffPlaceObject2.cpp
  src/SwiffPlaceObject3.cpp
  src/SwiffRemoveObject.cpp
  src/SwiffRemoveObject2.cpp
  src/SwiffSetBackgroundColor.cpp
  src/SwiffSprite.cpp
  src/SwiffShape.cpp
  src/SwiffShapePrivate.cpp
  src/SwiffSound.cpp
  src/SwiffTag.cpp
  src/SwiffText.cpp
  src/SwiffVideo.cpp
  src/SwiffWidget.cpp
)
# add ui files
SET(UI_FILES
  src/AboutDialog.ui
  src/LicenseWindow.ui
  src/MainWindow.ui
  src/SettingsDialog.ui
)
# set library moc headers
SET(MOC_HDRS
  src/AboutDialog.h
  src/LicenseWindow.h
  src/MainWindow.h
  src/PreviewWidget.h
  src/SettingsDialog.h
  src/SwiffSprite.h
  src/SwiffWidget.h
)
# wrap moc headers
QT4_WRAP_CPP(MOC_SRCS ${MOC_HDRS})
# wrap ui files
QT4_WRAP_UI(UI_SRCS ${UI_FILES})
# add resources
QT4_ADD_RESOURCES(RESOURCES resources.qrc)
# add executable
ADD_EXECUTABLE(Swiff WIN32 ${SRCS} ${MOC_SRCS} ${UI_SRCS} ${RESOURCES} resources.rc)
# link executable
TARGET_LINK_LIBRARIES(Swiff ${QT_LIBRARIES})
