CMAKE_MINIMUM_REQUIRED(VERSION 4.4)
PROJECT(SpaceJam)

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

ADD_EXECUTABLE(
    SpaceJam main.cpp AudioManager.cpp GameManager.cpp GUIManager.cpp SceneManager.cpp OptionsManager.cpp PitchDetection.cpp bloom.cpp Renderer.cpp Application.cpp
    Physics/CollisionBox.cpp
    Utilities/FontLoader.cpp Utilities/ObjectLoader.cpp Utilities/ShaderLoader.cpp Utilities/Framebuffer.cpp
    DrawObjects/DrawObject.cpp DrawObjects/NoteHighlight.cpp DrawObjects/NoteTarget.cpp DrawObjects/PlayerController.cpp
    GUIObjects/GUIObject.cpp GUIObjects/GUIImage.cpp GUIObjects/GUIButton.cpp
)

SET(JSONCPP_WITH_TESTS OFF CACHE BOOL "" FORCE)
SET(JSONCPP_WITH_POST_BUILD_UNITTEST OFF CACHE BOOL "" FORCE)

ADD_SUBDIRECTORY(Libraries/jsoncpp-1.9.8)

FIND_PACKAGE(glm REQUIRED) #0.9.9.8+ds-7
FIND_PACKAGE(OpenAL REQUIRED)
FIND_PACKAGE(PkgConfig REQUIRED)
FIND_PACKAGE(Freetype REQUIRED)
FIND_PACKAGE(OpenGL REQUIRED)
FIND_PACKAGE(GLEW REQUIRED)
FIND_PACKAGE(GLUT REQUIRED)
PKG_CHECK_MODULES(SNDFILE REQUIRED sndfile)
PKG_CHECK_MODULES(FFTW3 REQUIRED IMPORTED_TARGET fftw3)

TARGET_INCLUDE_DIRECTORIES(SpaceJam SYSTEM PUBLIC
    /home/alex/Projects/Space-Jam/Libraries/stb_image/include
    ${SNDFILE_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}
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
    PkgConfig::FFTW3
)

TARGET_COMPILE_OPTIONS(SpaceJam PUBLIC
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Wsign-conversion
    -Wshadow
    -Wnull-dereference
    -Wformat=2
    -Wundef
    -Wdouble-promotion
    -Wimplicit-fallthrough
    -Wnon-virtual-dtor
    -Woverloaded-virtual
    -Wold-style-cast
    -Wcast-align
    -Wcast-qual
    -Wpointer-arith
    -Wunused
)