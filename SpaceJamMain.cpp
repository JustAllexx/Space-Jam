#include "SpaceJamMain.h"
#include "DrawObjects/DrawObject.h"
#include "SceneManager.h"
#include "AudioManager.h"
#include "DrawObjects/PlayerController.h"
#include "GUIManager.h"
#include "GUIObjects/GUIButton.h"
#include "GameManager.h"
#include "Renderer.h"
#include "OptionsManager.h"

#include "Utilities/ShaderLoader.h"
#include "Utilities/Framebuffer.h"
#include "Utilities/ObjectLoader.h"
#include <cmath>
#include <optional>
#include <stdexcept>
#include <glm/ext/vector_float3.hpp>
#include "Application.h"
//Use radians instead of degrees
#define GLM_FORCE_RADIANS


//The main function is in charge of instantiating glut and creating the window and calling the initialisation of the other classes
int main(int argc, char** argv) {
	Application app(argc, argv);
	app.Run();
	return 0;
}
