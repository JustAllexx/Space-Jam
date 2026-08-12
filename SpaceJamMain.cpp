#include "Application.h"
//Use radians instead of degrees
#define GLM_FORCE_RADIANS


//The main function is in charge of instantiating glut and creating the window and calling the initialisation of the other classes
int main(int argc, char** argv) {
	Application app(argc, argv);
	app.Run();
	return 0;
}
