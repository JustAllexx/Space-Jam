#include "GUIObject.h"
#include <Utilities/ShaderLoader.h>

GUIObject::GUIObject(Program& GUIShader_) : GUIShader(GUIShader_) {
	GLuint shaderID = GUIShader.getProgramID();
	isTextPos = glGetUniformLocation(shaderID, "isText");
}