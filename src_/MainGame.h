#pragma once

#include <SDL2/SDL.h>
#include <GL/glew.h>

#include <Bengine/Bengine.h>
#include <Bengine/GLSLProgram.h>
#include <Bengine/GLTexture.h>
#include <Bengine/Sprite.h>
#include <Bengine/Window.h>
#include <Bengine/InputManager.h>
#include <Bengine/Timing.h>

#include <Bengine/SpriteBatch.h>

#include <Bengine/Camera2D.h>

#include "Bullet.h"

#include <vector>


struct sBall {
    float px, py; // Position vector
    float vx, vy; // Velocity vector
    float ax, ay; // Acceleration vector
    float radius;

    int id; // ID number
};

enum class GameState {PLAY, EXIT};

//Our example game class, just for testing purposes right now
class MainGame
{
public:
    MainGame();
    ~MainGame();

    void run();

private:
    void initSystems();
    void initShaders();
    void initGame();
    void addBall(float x, float y, float r = 5.0f);
    void update();
    void gameLoop();
    void processInput();
    void drawGame();

    Bengine::Window _window;
    int _screenWidth;
    int _screenHeight;
    GameState _gameState;

    Bengine::GLSLProgram _colorProgram;
    Bengine::Camera2D _camera;

    Bengine::SpriteBatch _spriteBatch;

    Bengine::InputManager _inputManager;
    Bengine::FpsLimiter _fpsLimiter;

    std::vector<Bullet> _bullets;
    std::vector<sBall> vecBalls;
    size_t selectedBallIndex = -1;
    //std::vector<std::pair<float, float>> modelCircle; // A circle in its simplest form
    
    float _maxFPS;
    float _fps;
    float _time;
};

