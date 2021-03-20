#pragma once

namespace Bengine {

    ///Calculates FPS and also limits FPS
    class FpsLimiter {
    public:
        FpsLimiter();

        // Initializes the FPS limiter. For now, this is
        // analogous to setMaxFPS
        void init(float maxFPS);

        // Sets the desired max FPS
        void setMaxFPS(float maxFPS);

        void begin();

        // end() will return the current FPS as a float
        float end();

        // Gets the frame time in milliseconds for the last frame.
        float getFrameTime() const { return _frameTime; }

        // Gets the change in time from the start of the last frame to the end of the last frame, normalized from 0 to 1. (from https://github.com/Barnold1953/GraphicsTutorials/blob/f5f6e1f37170344c560a50300f0dd4db72619c48/BallGame/MainGame.cpp#L40 )
        float getDeltaTime() const { return _frameTime / (1000.0f / _maxFPS /* Convert fps to frametime in milliseconds (1 / fps is frametime, then *1000 is milliseconds) */); }
    private:
        // Calculates the current FPS
        void calculateFPS();

        // Variables
        float _fps;
        float _maxFPS;
        float _frameTime;
        unsigned int _startTicks;
    };

}
