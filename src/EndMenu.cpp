#include "EndMenu.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <string_view>

namespace {
constexpr int canvasWidth = 640;
constexpr int canvasHeight = 360;
constexpr int buttonX = 238, buttonY = 195, buttonWidth = 164, buttonHeight = 46;

struct Layout {
    float scale, x, y;
    Layout(int width, int height)
        : scale(std::min(width / 640.0f, height / 360.0f)),
          x((width - canvasWidth * scale) / 2),
          y((height - canvasHeight * scale) / 2) {}
};

struct Color { float r, g, b; };

void rectangle(const Layout& layout, int height, int x, int y, int w, int h, Color color) {
    const int left = static_cast<int>(layout.x + x * layout.scale);
    const int right = static_cast<int>(layout.x + (x + w) * layout.scale);
    const int top = static_cast<int>(layout.y + y * layout.scale);
    const int bottom = static_cast<int>(layout.y + (y + h) * layout.scale);
    glScissor(left, height - bottom, right - left, bottom - top);
    glClearColor(color.r, color.g, color.b, 1);
    glClear(GL_COLOR_BUFFER_BIT);
}

// Seven rows of five pixels per glyph, stored as bit masks.
const unsigned char* glyph(char c) {
    static constexpr unsigned char letters[26][7] = {
        {14,17,17,31,17,17,17},{30,17,17,30,17,17,30},{14,17,16,16,16,17,14},
        {30,17,17,17,17,17,30},{31,16,16,30,16,16,31},{31,16,16,30,16,16,16},
        {14,17,16,23,17,17,15},{17,17,17,31,17,17,17},{14,4,4,4,4,4,14},
        {7,2,2,2,18,18,12},{17,18,20,24,20,18,17},{16,16,16,16,16,16,31},
        {17,27,21,21,17,17,17},{17,25,21,19,17,17,17},{14,17,17,17,17,17,14},
        {30,17,17,30,16,16,16},{14,17,17,17,21,18,13},{30,17,17,30,20,18,17},
        {15,16,16,14,1,1,30},{31,4,4,4,4,4,4},{17,17,17,17,17,17,14},
        {17,17,17,17,17,10,4},{17,17,17,21,21,21,10},{17,17,10,4,10,17,17},
        {17,17,10,4,4,4,4},{31,1,2,4,8,16,31}
    };
    static constexpr unsigned char blank[7] = {};
    return c >= 'A' && c <= 'Z' ? letters[c - 'A'] : blank;
}

void label(const Layout& layout, int height, int y, int size, std::string_view text, Color color) {
    int x = (canvasWidth - (static_cast<int>(text.size()) * 6 - 1) * size) / 2;
    for (char c : text) {
        const auto* rows = glyph(c);
        for (int row = 0; row < 7; ++row) {
            for (int col = 0; col < 5; ++col) {
                if (rows[row] & (16 >> col))
                    rectangle(layout, height, x + col * size, y + row * size, size, size, color);
            }
        }
        x += 6 * size;
    }
}
}

bool menuButtonHovered(GLFWwindow* window) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    const Layout layout(width, height);
    if (layout.scale <= 0) return false;
    double cursorX, cursorY;
    glfwGetCursorPos(window, &cursorX, &cursorY);
    const double x = (cursorX - layout.x) / layout.scale;
    const double y = (cursorY - layout.y) / layout.scale;
    return x >= buttonX && x < buttonX + buttonWidth &&
           y >= buttonY && y < buttonY + buttonHeight;
}

void drawEndMenu(GLFWwindow* window, int width, int height, MenuScreen screen, float fade) {
    Layout layout(width, height);
    layout.y += (1.0f - fade) * 16 * layout.scale;
    const bool preview = screen == MenuScreen::SafeOpened;
    const bool won = screen == MenuScreen::Victory;
    const Color accent = preview ? Color{0.36f, 0.79f, 1.0f} :
                        won ? Color{0.25f, 0.86f, 0.61f} : Color{1.0f, 0.34f, 0.37f};
    glEnable(GL_SCISSOR_TEST);
    rectangle(layout, height, 141, 96, 358, 178, {0.04f, 0.07f, 0.11f});
    rectangle(layout, height, 141, 96, 358, 3, accent);
    label(layout, height, 120, 3, preview ? "VAULT OPENED" : won ? "LEVEL COMPLETE" : "ALARM TRIGGERED", accent);
    label(layout, height, 159, 2, preview ? "LEVEL TWO COMING SOON" : won ? "GEM EXTRACTED" : "YOU HIT A LASER", {0.83f, 0.87f, 0.92f});
    rectangle(layout, height, buttonX, buttonY, buttonWidth, buttonHeight,
              menuButtonHovered(window) && fade >= 1 ? Color{0.50f, 0.81f, 0.98f} : Color{0.31f, 0.65f, 0.87f});
    label(layout, height, 211, 2, preview ? "CONTINUE" : "PLAY AGAIN", {0.04f, 0.07f, 0.11f});
    label(layout, height, 251, 1, preview ? "RETURN TO THE EXIT" : "ENTER TO RESTART", {0.66f, 0.73f, 0.82f});
    glDisable(GL_SCISSOR_TEST);
}
