#include "OptionsManager.h"
#include "GUIManager.h"
#include "GameManager.h"
#include "GUIObjects/GUIButton.h"

const char* countingStarsNotes = "Counting Stars Audio/notes30s.json";
const char* countingStarsAudio = "Counting Stars Audio/CS_30s.ogg";

OptionsManager::OptionsManager(GUIManager& guiManager_, GameManager* gameManager_) : guiManager(guiManager_), samplesGUI(*guiManager.samplesOptionText),
 gameManager(gameManager_) {
	guiManager.MainMenu_StartButtonClick->setClickFunction([&] {
	gameManager->startGame(countingStarsNotes, countingStarsAudio);
	});
	guiManager.MainMenu_OptionsButtonClick->setClickFunction(
		[&] {
			guiManager.showOptionsMenu();
		}
	);
	guiManager.MainMenu_QuitButtonClick->setClickFunction([] {exit(0);});

	//Here we define what each button should do, what function it should call
	guiManager.samplesOptionLeftClick->setClickFunction([&] {DecrementSamplesOption();});
	guiManager.samplesOptionRightClick->setClickFunction([&] {IncrementSamplesOption();});
	guiManager.samplesBackClick->setClickFunction([&] {
		guiManager.showMainMenu();
	});
 }

void OptionsManager::IncrementSamplesOption()
{
	//In C++ the modulus operator doesn't have the desired effect with negative numbers (-1 % 4) = -1 not 3 (which is what we want)
	//So add the size of the text vector on aswell
	samplesOptionIndex = (samplesOptionIndex + 1 + samplesOptionsText.size()) % samplesOptionsText.size();
	samplesGUI.text = samplesOptionsText[samplesOptionIndex];
}

void OptionsManager::DecrementSamplesOption()
{
	samplesOptionIndex = (samplesOptionIndex - 1 + samplesOptionsText.size()) % samplesOptionsText.size();
	samplesGUI.text = samplesOptionsText[samplesOptionIndex];
}