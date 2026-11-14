#include "GameAudio.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>

namespace {
constexpr int sampleRate = 48000;
constexpr int channels = 2;
constexpr int blockFrames = 1024;
constexpr float tau = 6.2831853f;
}

struct GameAudio::Output {
    struct Block {
        std::array<short, blockFrames * channels> samples{};
        WAVEHDR header{};
    };

    HWAVEOUT device = nullptr;
    bool writeFailed = false;
    std::array<Block, 3> blocks;
    int ambientSample = 0;
    int stepAge = sampleRate;
    int jumpAge = sampleRate;
    float stepVolume = 0.0f;
    float lowNoise = 0.0f;
    std::uint32_t random = 1729;

    void queue(Block& block) {
        for (int frame = 0; frame < blockFrames; ++frame) {
            const float t = static_cast<float>(ambientSample) / sampleRate;
            ambientSample = (ambientSample + 1) % (20 * sampleRate);
            const float pulse = 0.7f + 0.3f * std::sin(tau * 0.4f * t);
            const float drone = pulse * (0.10f * std::sin(tau * 48.0f * t) +
                                          0.06f * std::sin(tau * 51.0f * t)) +
                                0.045f * std::sin(tau * 157.0f * t);

            random = random * 1664525u + 1013904223u;
            const float noise = static_cast<float>(random >> 16) / 32767.5f - 1.0f;
            lowNoise += 0.16f * (noise - lowNoise);
            float value = drone + 0.021f * lowNoise;
            if (stepAge < sampleRate / 6) {
                const float s = static_cast<float>(stepAge++) / sampleRate;
                const float envelope = std::min(1.0f, s * 500.0f) * std::exp(-32.0f * s);
                value += stepVolume * envelope * (0.65f * std::sin(tau * 86.0f * s) + lowNoise);
            }
            if (jumpAge < sampleRate / 4) {
                const float s = static_cast<float>(jumpAge++) / sampleRate;
                const float envelope = std::min(1.0f, s * 180.0f) * std::exp(-17.0f * s);
                value += envelope * (0.19f * noise + 0.18f * std::sin(tau * (95.0f * s + 340.0f * s * s)));
            }
            const auto sample = static_cast<short>(std::clamp(value, -1.0f, 1.0f) * 32767.0f);
            block.samples[frame * channels] = sample;
            block.samples[frame * channels + 1] = sample;
        }
        const auto result = waveOutWrite(device, &block.header, sizeof(block.header));
        if (result != MMSYSERR_NOERROR) {
            if (!writeFailed) std::cerr << "Audio buffer error: " << result << '\n';
            writeFailed = true;
        }
    }
};

GameAudio::GameAudio() {
    auto* candidate = new Output;
    constexpr int bytesPerFrame = channels * sizeof(short);
    WAVEFORMATEX format{WAVE_FORMAT_PCM, channels, sampleRate,
                       sampleRate * bytesPerFrame, bytesPerFrame, 16, 0};
    const auto result = waveOutOpen(&candidate->device, WAVE_MAPPER, &format, 0, 0, CALLBACK_NULL);
    if (result != MMSYSERR_NOERROR) {
        char error[MAXERRORLENGTH]{};
        waveOutGetErrorTextA(result, error, sizeof(error));
        std::cerr << "Audio output unavailable (" << result << "): " << error << '\n';
        delete candidate;
        return;
    }
    output = candidate;
    for (auto& block : output->blocks) {
        block.header.lpData = reinterpret_cast<char*>(block.samples.data());
        block.header.dwBufferLength = sizeof(short) * block.samples.size();
        const auto prepared = waveOutPrepareHeader(output->device, &block.header, sizeof(block.header));
        if (prepared == MMSYSERR_NOERROR) output->queue(block);
        else std::cerr << "Audio buffer preparation error: " << prepared << '\n';
    }
}

GameAudio::~GameAudio() {
    if (!output) return;
    waveOutReset(output->device);
    for (auto& block : output->blocks) {
        if (block.header.dwFlags & WHDR_PREPARED)
            waveOutUnprepareHeader(output->device, &block.header, sizeof(block.header));
    }
    waveOutClose(output->device);
    delete output;
}

void GameAudio::update() {
    if (!output) return;
    for (auto& block : output->blocks) {
        if (block.header.dwFlags & WHDR_DONE) output->queue(block);
    }
}

void GameAudio::footstep(bool crouched) {
    if (!output) return;
    output->stepAge = 0;
    output->stepVolume = crouched ? 0.15f : 0.45f;
}

void GameAudio::jump() {
    if (output) output->jumpAge = 0;
}
#else
GameAudio::GameAudio() = default;
GameAudio::~GameAudio() = default;
void GameAudio::update() {}
void GameAudio::footstep(bool) {}
void GameAudio::jump() {}
#endif
