#include "NoteHighlight.h"
#include "../AudioManager.h"

//Constructor for the note highlight
NoteHighlight::NoteHighlight(float inNoteTime, DrawObject* inParentNote, AudioManager* audioIn) :
	DrawObject("Models/noteOutline.obj", "Textures/green.png", 0.f, 1.f, false, glm::vec3(inParentNote->pos.x, inParentNote->pos.y, 0.f),
	glm::vec3(2.f, 2.f, 2.f), glm::vec3(0.f, 1.570796327f, 0.f))
{
	DrawObject* parentNote = inParentNote;
	pos = glm::vec3(parentNote->pos.x, parentNote->pos.y, 0.f);
	scale = glm::vec3(2.f, 2.f, 2.f);
	//Reset this line later after updating rotation
	//rotation = glm::quat(glm::vec3(0.f, 0.f, 0.f));
	rotation = glm::vec3(0.f, 1.570796327f, 0.f);
	audioManager = audioIn;
	highlightTime = 1.f;
	ambient = 1.f;
	noteTime = inNoteTime;
}

//Updates similarly to the noteObject update function, based on time
void NoteHighlight::Update([[maybe_unused]] float deltaTime)
{
	float playPos = audioManager->getPlayPos();
	float difTime = (highlightTime - (noteTime - playPos)) / highlightTime;
	//Opacity increases as the NoteObject gets closer to the player
	opacity = std::min(std::max(difTime, 0.f), 1.f);
	//Should be deleted after the point when the note should've been hit
	if (playPos > noteTime) {
		bToDelete = true;
	}
}
