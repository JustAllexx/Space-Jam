#pragma once

#include <vector>
#include <string>

class GUIManager;
class GUIButton;
class GameManager;

//The OptionsManager is in charge of changing values when the user changes a setting in the Options Menu
//This class also defines all the button behaviours for every GUI Element on the screen
class OptionsManager {
private:
	GUIManager& guiManager;
	GUIButton& samplesGUI;
	GameManager& gameManager;

	size_t samplesOptionIndex{0};
	std::vector<std::string> samplesOptionsText{{
		"1024 Samples",
		"2048 Samples",
		"4096 Samples",
		"512 Samples"
	}};
public:
	OptionsManager(GUIManager& guiManager, GameManager& gameManager);
	void IncrementSamplesOption();
	void DecrementSamplesOption();
};