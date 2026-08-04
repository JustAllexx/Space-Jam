CMAKE_MINIMUM_REQUIRED(VERSION 4.4)
PROJECT(SpaceJam)

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

ADD_EXECUTABLE(
    SpaceJam SpaceJamMain.cpp AudioManager.cpp GameManager.cpp GUIManager.cpp ObjectLoader.cpp FontLoader.cpp SceneManager.cpp OptionsManager.cpp YIN.cpp Physics/CollisionBox.cpp
    DrawObjects/DrawObject.cpp DrawObjects/NoteHighlight.cpp DrawObjects/NoteTarget.cpp DrawObjects/PlayerController.cpp
    GUIObjects/GUIObject.cpp GUIObjects/GUIImage.cpp GUIObjects/GUIButton.cpp
)

SET(JSONCPP_WITH_TESTS OFF CACHE BOOL "" FORCE)
SET(JSONCPP_WITH_POST_BUILD_UNITTEST OFF CACHE BOOL "" FORCE)

ADD_SUBDIRECTORY(Libraries/jsoncpp-1.9.8)

FIND_PACKAGE(glm REQUIRED)
FIND_PACKAGE(OpenAL REQUIRED)
FIND_PACKAGE(PkgConfig REQUIRED)
FIND_PACKAGE(Freetype REQUIRED)
FIND_PACKAGE(OpenGL REQUIRED)
FIND_PACKAGE(GLEW REQUIRED)
FIND_PACKAGE(GLUT REQUIRED)
PKG_CHECK_MODULES(SNDFILE REQUIRED sndfile)

TARGET_INCLUDE_DIRECTORIES(SpaceJam SYSTEM PUBLIC
    /home/alex/Documents/Projects/Space-Jam/Libraries/stb_image/include
    ${SNDFILE_INCLUDE_DIRS}
)

TARGET_LINK_LIBRARIES(SpaceJam PRIVATE
    ${SNDFILE_LIBRARIES}
    Freetype::Freetype
    OpenGL::GL
    OpenGL::GLU
    GLEW::GLEW
    GLUT::GLUT
    OpenAL::OpenAL
    glm::glm 
    jsoncpp_lib
)

TARGET_COMPILE_OPTIONS(SpaceJam PUBLIC
    -Wall
    -Wextra 
    -Wpedantic
    -Wconversion
    -Wshadow
)