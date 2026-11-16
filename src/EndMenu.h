#pragma once

struct GLFWwindow;

enum class MenuScreen { Alarm, Victory, NextLevel };

void drawEndMenu(GLFWwindow* window, int width, int height, MenuScreen screen, float fade);
bool menuButtonHovered(GLFWwindow* window);

void drawControls(int width, int height, int level, bool cursorFree, const char* hint);
