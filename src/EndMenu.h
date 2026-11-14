#pragma once

struct GLFWwindow;

enum class MenuScreen { Alarm, Victory, SafeOpened };

void drawEndMenu(GLFWwindow* window, int width, int height, MenuScreen screen, float fade);
bool menuButtonHovered(GLFWwindow* window);
