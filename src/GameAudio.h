#pragma once

class GameAudio {
public:
    GameAudio();
    ~GameAudio();
    GameAudio(const GameAudio&) = delete;
    GameAudio& operator=(const GameAudio&) = delete;

    void update();
    void footstep(bool crouched);
    void jump();

private:
    struct Output;
    Output* output = nullptr;
};
