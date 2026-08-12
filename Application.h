#pragma once

#include <memory>
#include <map>

#include "GameManager.h"
#include "Renderer.h"

class Application {
private:
    static Application* inst;
    static std::map<unsigned char, bool> keyMap;

    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<GameManager> gameManager;
public:
    Application(int argc, char** argv);

    void Run();

    void newFrame(int value);
    void display();
    void reshape(int x, int y);
    void mouse(int button, int state, int x, int y);
    void mouseMotion(int x, int y);
    void keyPress(unsigned char key, int x, int y);
    void keyUp(unsigned char key, int x, int y);

    static void newFrameCallback(int value) {inst->newFrame(value);}
    static void displayCallback() {inst->display();}
    static void reshapeCallback(int x, int y) {inst->reshape(x, y);}
    static void mouseCallback(int button, int state, int x, int y) {inst->mouse(button, state, x, y);}
    static void mouseMotionCallback(int x, int y) {inst->mouseMotion(x, y);}
    static void keyPressCallback(unsigned char key, int x, int y) {inst->keyPress(key, x, y);}
    static void keyUpCallback(unsigned char key, int x, int y) {inst->keyUp(key, x, y);}
};