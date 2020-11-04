#include "MainGame.h"
#include <Bengine/BengineErrors.h>
#include <Bengine/ResourceManager.h>

#include <iostream>
#include <string>

//Constructor, just initializes private member variables
MainGame::MainGame() : 
    _screenWidth(1024),
    _screenHeight(768), 
    _time(0.0f),
    _gameState(GameState::PLAY),
    _maxFPS(60.0f)
{
    _camera.init(_screenWidth, _screenHeight);
}

//Destructor
MainGame::~MainGame()
{
}

//This runs the game
void MainGame::run() {
    initSystems();

    initGame();
 
    //This only returns when the game ends
    gameLoop();
}

//Initialize SDL and Opengl and whatever else we need
void MainGame::initSystems() {

    Bengine::init();

    _window.create("Game Engine", _screenWidth, _screenHeight, 0);

    initShaders();

    _spriteBatch.init();
    _fpsLimiter.init(_maxFPS);
}

void MainGame::initShaders() {
    _colorProgram.compileShaders("Shaders/colorShading.vert", "Shaders/colorShading.frag");
    _colorProgram.addAttribute("vertexPosition");
    _colorProgram.addAttribute("vertexColor");
    _colorProgram.addAttribute("vertexUV");
    _colorProgram.linkShaders();
}

void MainGame::initGame() {
    // Define Circle Model
    // modelCircle.push_back({ 0.0f, 0.0f });
    // int nPoints = 20;
    // for (int i = 0; i < nPoints; i++) {
    //     modelCircle.push_back({ cosf(i / (float)(nPoints - 1) * 2.0f * PI), sinf(i / (float)(nPoints - 1) * 2.0f * PI) });
    // }

    float fDefaultRad = 4.0f;
    addBall(_screenWidth * 0.25f, _screenHeight * 0.5f, fDefaultRad);
    addBall(_screenWidth * 0.75f, _screenHeight * 0.5f, fDefaultRad);
}

void MainGame::addBall(float x, float y, float r) {
    sBall b;
    b.px = x; b.py = y;
    b.vx = 0; b.vy = 0;
    b.ax = 0; b.ay = 0;
    b.radius = r;

    //b.id = vecBalls.size();
    vecBalls.emplace_back(b);
}

void MainGame::update() {
    _camera.update();

    // Update all bullets
    for (size_t i = 0; i < _bullets.size();) {
        if (_bullets[i].update() == true) {
            _bullets[i] = _bullets.back();
            _bullets.pop_back();
        } else {
            i++;
        }
    }

    auto doCirclesOverlap = [](float x1, float y1, float x2, float y2, float r1, float r2) {
        return fabs((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2)) <= (r1+r2)*(r1+r2);
    };
    // Update balls
    for (size_t i = 0; i < vecBalls.size(); i++) {
        sBall& ball = vecBalls[i];
        for (size_t j = 0; j < vecBalls.size(); j++) {
            sBall& target = vecBalls[j];
            if (i != j) { // Else, don't test against ourself
                if (doCirclesOverlap(ball.px, ball.py, ball.radius, target.px, target.py, target.radius)) {
                    // We have a collision

                    // Static resolution: move ball and target out of their overlapping part

                    // Distance between ball centers
                    float fDistance = sqrtf((ball.px - target.px)*(ball.px - target.px) + (ball.py - target.py)*(ball.py - target.py));
                    float fOverlap = 0.5f * (fDistance - ball.radius - target.radius);
                    
                    // Displace current ball
                    ball.px -= fOverlap * (ball.px - target.px) / fDistance;
                    ball.py -= fOverlap * (ball.py - target.py) / fDistance;
                    
                    // Displace target ball
                    target.px += fOverlap * (ball.px - target.px) / fDistance;
                    target.py += fOverlap * (ball.py - target.py) / fDistance;
                    
                }
            }
        }
    }
}

//This is the main game loop for our program
void MainGame::gameLoop() {

    //Will loop until we set _gameState to EXIT
    while (_gameState != GameState::EXIT) {
       
        _fpsLimiter.begin();

        processInput();
        _time += 0.1;

        update();

        drawGame();

        _fps = _fpsLimiter.end();

        //print only once every 10 frames
        static int frameCounter = 0;
        frameCounter++;
        if (frameCounter == 10) {
            std::cout << _fps << std::endl;
            frameCounter = 0;
        }
    }
}

//Processes input with SDL
void MainGame::processInput() {
    SDL_Event evnt;

    const float CAMERA_SPEED = 2.0f;
    const float SCALE_SPEED = 0.1f;

    //Will keep looping until there are no more events to process
    while (SDL_PollEvent(&evnt)) {
        switch (evnt.type) {
            case SDL_QUIT:
                _gameState = GameState::EXIT;
                break;
            case SDL_MOUSEMOTION:
                _inputManager.setMouseCoords((float)evnt.motion.x, (float)evnt.motion.y);
                break;
            case SDL_KEYDOWN:
                _inputManager.pressKey(evnt.key.keysym.sym);
                break;
            case SDL_KEYUP:
                _inputManager.releaseKey(evnt.key.keysym.sym);
                break;
            case SDL_MOUSEBUTTONDOWN:
                _inputManager.pressKey(evnt.button.button);
                break;
            case SDL_MOUSEBUTTONUP:
                _inputManager.releaseKey(evnt.button.button);
                break;
        }
    }

    if (_inputManager.isKeyDown(SDLK_w)) {
        _camera.setPosition(_camera.getPosition() + glm::vec2(0.0f, CAMERA_SPEED));
    }
    if (_inputManager.isKeyDown(SDLK_s)) {
        _camera.setPosition(_camera.getPosition() + glm::vec2(0.0f, -CAMERA_SPEED));
    }
    if (_inputManager.isKeyDown(SDLK_a)) {
        _camera.setPosition(_camera.getPosition() + glm::vec2(-CAMERA_SPEED, 0.0f));
    }
    if (_inputManager.isKeyDown(SDLK_d)) {
        _camera.setPosition(_camera.getPosition() + glm::vec2(CAMERA_SPEED, 0.0f));
    }
    if (_inputManager.isKeyDown(SDLK_q)) {
        _camera.setScale(_camera.getScale() + SCALE_SPEED);
    }
    if (_inputManager.isKeyDown(SDLK_e)) {
        _camera.setScale(_camera.getScale() - SCALE_SPEED);
    }

    if (_inputManager.isKeyDown(SDL_BUTTON_LEFT)) {
        glm::vec2 mouseCoords = _inputManager.getMouseCoords();
        mouseCoords = _camera.convertScreenToWorld(mouseCoords);
        
        glm::vec2 playerPosition(0.0f);
        glm::vec2 direction = mouseCoords - playerPosition;
        direction = glm::normalize(direction);

        //_bullets.emplace_back(playerPosition, direction, 5.00f, 1000);
        
        if (_inputManager.isKeyPressed(SDL_BUTTON_LEFT)) {   
            auto isPointInCircle = [](float x1, float y1, float r1, float px, float py) {
                return fabs((x1 - px)*(x1 - px) + (y1 - py)*(y1 - py)) < (r1 * r1);
            };
            selectedBallIndex = -1;
            for (size_t i = 0; i < vecBalls.size(); i++) {
                sBall& ball = vecBalls[i];
                if (isPointInCircle(ball.px, ball.py, ball.radius, mouseCoords.x, mouseCoords.y)) {
                    selectedBallIndex = i;
                    break;
                }
            }
        }

        if (selectedBallIndex != -1) {
            sBall& b = vecBalls[selectedBallIndex];
            b.px = mouseCoords.x;
            b.py = mouseCoords.y;
        }
    }
    else {
        selectedBallIndex = -1;
    }
}

//Draws the game using the almighty OpenGL
void MainGame::drawGame() {

    //Set the base depth to 1.0
    glClearDepth(1.0);
    //Clear the color and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //Enable the shader
    _colorProgram.use();

    //We are using texture unit 0
    glActiveTexture(GL_TEXTURE0);
    //Get the uniform location
    GLint textureLocation = _colorProgram.getUniformLocation("mySampler");
    //Tell the shader that the texture is in texture unit 0
    glUniform1i(textureLocation, 0);

    //Set the camera matrix
    GLint pLocation = _colorProgram.getUniformLocation("P");
    glm::mat4 cameraMatrix = _camera.getCameraMatrix();

    glUniformMatrix4fv(pLocation, 1, GL_FALSE, &(cameraMatrix[0][0]));

    _spriteBatch.begin();

    glm::vec4 pos(0.0f, 0.0f, 50.0f, 50.0f);
    glm::vec4 uv(0.0f, 0.0f, 1.0f, 1.0f);
    static Bengine::GLTexture texture = Bengine::ResourceManager::getTexture("Textures/jimmyJump_pack/PNG/CharacterRight_Standing.png");
    Bengine::ColorRGBA8 color;
    color.r = 255;
    color.g = 255;
    color.b = 255;
    color.a = 255;

    _spriteBatch.draw(pos, uv, texture.id, 0.0f, color);

    for (size_t i = 0; i < _bullets.size(); i++) {
        _bullets[i].draw(_spriteBatch);
    }

    for (auto ball : vecBalls) {        
        glm::vec4 uv(0.0f, 0.0f, 1.0f, 1.0f);
        static Bengine::GLTexture texture = Bengine::ResourceManager::getTexture("Textures/jimmyJump_pack/PNG/Bullet.png");
        Bengine::ColorRGBA8 color;
        color.r = 255;
        color.g = 255;
        color.b = 255;
        color.a = 255;

        glm::vec4 posAndSize = glm::vec4(ball.px, ball.py, texture.width * (ball.radius / 2), texture.height * (ball.radius / 2));

        _spriteBatch.draw(posAndSize, uv, texture.id, 0.0f, color, atan2f(ball.vy, ball.vx));
    }

    _spriteBatch.end();

    _spriteBatch.renderBatch();

    //unbind the texture
    glBindTexture(GL_TEXTURE_2D, 0);

    //disable the shader
    _colorProgram.unuse();

    //Swap our buffer and draw everything to the screen!
    _window.swapBuffer();
}    
