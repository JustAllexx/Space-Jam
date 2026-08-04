#include "GUIObject.h"

GUIObject::GUIObject(GLuint inGUIShader) : GUIShader(inGUIShader) {
	isTextPos = glGetUniformLocation(GUIShader, "isText");
}