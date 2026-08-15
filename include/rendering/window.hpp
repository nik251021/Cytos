#pragma once
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Window {
public:
    Window(int width, int height, const std::string& title);
    ~Window();

    bool shouldClose() const;
    void swapBuffers();
    void pollEvents();

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    GLFWwindow* getNativeWindow() const { return m_window; }

private:
    GLFWwindow* m_window;
    int m_width, m_height;
};