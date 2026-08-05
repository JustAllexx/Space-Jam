#pragma once

#include <functional>
#include <GL/glew.h>
#include <GL/glut.h>

class Program;

class GUIObject {
protected:
	Program& GUIShader;
	GLuint isTextPos;
public:
	GUIObject(Program& GUIShader);
	virtual void Render() {
		return;
	}
};

//The class which describes elements on the screen that are clickable by a mouse
//When check collission is called this all members of this class are iterated through to see if the click was within the region
class Clickable {
protected:
	// Describes the shape of the click region (from the centre)
	int top, bottom, left, right;
public:
	bool hovered; //Is the Element being hovered over
	bool checkCollision(int mousePosX, int mousePosY) const noexcept {
		return bottom < mousePosY && 
		       mousePosY < top &&
			   left < mousePosX &&
			   mousePosX < right;
	} //Check if a collission has occured
	//Each clickable object contains 2 function pointers, these pointers contain the memory location of a function, these functions are called whenever the button is clicked or hovered over
	//Setter functions for the Click function pointer and the hover function pointer
	void setClickFunction(std::function<void()> newClickFunction) noexcept {onClick = newClickFunction;};
	void setHoverFunction(std::function<void()> newHoverFunction) noexcept {onHover = newHoverFunction;};
	//Function pointers
	std::function<void()> onClick;
	std::function<void()> onHover;
};