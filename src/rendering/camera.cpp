#include "rendering/camera.hpp"
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(float width, float height) 
    : m_width(width), m_height(height), m_position(0.0f, 0.0f), m_zoom(1.0f) {
    updateProjection();
}

void Camera::updateProjection() {
    float halfW = (m_width / 2.0f) / m_zoom;
    float halfH = (m_height / 2.0f) / m_zoom;
    m_projection = glm::ortho(-halfW, halfW, -halfH, halfH, -1.0f, 1.0f);
}

glm::mat4 Camera::getViewProjection() const {
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(-m_position, 0.0f));
    return m_projection * view;
}

void Camera::setPosition(glm::vec2 pos) {
    m_position = pos;
}

void Camera::setZoom(float zoom) {
    m_zoom = zoom;
    updateProjection();
}