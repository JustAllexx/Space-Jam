#pragma once
#include "DrawObject.h"

//Forward Declaration
class AudioManager;

//Highlights used in game, these show up on screen when the noteblock is close to being "hittable"
class NoteHighlight : public DrawObject {
private:
	AudioManager* audioManager;
	float highlightTime;
	float noteTime;
public:
	NoteHighlight(float inNoteTime, DrawObject* inParentNote, AudioManager* audioIn);
	void Update(float deltaTime);
};