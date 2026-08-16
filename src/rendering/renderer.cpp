#include "rendering/renderer.hpp"
#include <glad/glad.h>

Renderer::Renderer() : m_maxInstances(10000) {
    m_shader = std::make_unique<Shader>("data/shaders/cell.vert", "data/shaders/cell.frag");
    // Загружаем шейдер постпроцессинга (пути подставь свои)
    m_postShader = std::make_unique<Shader>("data/shaders/microscope.vert", "data/shaders/microscope.frag");

    float vertices[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f,  1.0f
    };
    unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };

    m_vao = std::make_unique<VertexArray>();
    m_vbo = std::make_unique<VertexBuffer>(vertices, sizeof(vertices));
    m_ebo = std::make_unique<IndexBuffer>(indices, 6);

    m_vao->addBuffer(*m_vbo);
    m_vao->setIndexBuffer(*m_ebo);

    m_instanceBuffer = std::make_unique<InstanceBuffer>(nullptr, m_maxInstances * sizeof(InstanceData));
    m_vao->addInstanceBuffer(*m_instanceBuffer);

    // --- НАСТРОЙКА FBO ДЛЯ ПОСТПРОЦЕССИНГА ---
    // Создаем FBO
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // Создаем текстуру цвета
    glGenTextures(1, &m_textureColorBuffer);
    glBindTexture(GL_TEXTURE_2D, m_textureColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1000, 1000, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr); // Задай начальное разрешение окна
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // Важно для искажений линзы!
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureColorBuffer, 0);

    // Создаем Renderbuffer для буфера глубины и трафарета
    glGenRenderbuffers(1, &m_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1000, 1000);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        // Обработка ошибки FBO, если нужно

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // --- НАСТРОЙКА КВАДРАТА ДЛЯ ПОСТПРОЦЕССИНГА ---
    float quadVertices[] = { // Позиции     // TexCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    glGenVertexArrays(1, &m_quadVAO);
    glGenBuffers(1, &m_quadVBO);
    glBindVertexArray(m_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glad_glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glad_glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);
}

void Renderer::beginScene(const Camera& camera) {
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    
    glClearColor(0.05f, 0.05f, 0.06f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    m_shader->use();
    m_shader->setMat4("u_ViewProj", camera.getViewProjection());
}

void Renderer::drawCells(const std::vector<InstanceData>& cells) {
    if (cells.empty()) return;

    if (cells.size() > m_maxInstances) {
        m_maxInstances = cells.size() * 2;
        m_instanceBuffer = std::make_unique<InstanceBuffer>(nullptr, m_maxInstances * sizeof(InstanceData));
        m_vao->addInstanceBuffer(*m_instanceBuffer);
    }

    m_instanceBuffer->updateData(cells.data(), cells.size() * sizeof(InstanceData));

    m_vao->bind();
    glDrawElementsInstanced(GL_TRIANGLES, m_ebo->getCount(), GL_UNSIGNED_INT, nullptr, cells.size());
}

void Renderer::drawRect(glm::vec2 pos, glm::vec2 size, glm::vec4 color) {
    InstanceData frame(pos + size * 0.5f, size.x * 0.5f, -99.0f, color);

    m_instanceBuffer->updateData(&frame, sizeof(InstanceData));
    
    m_vao->bind();
    glDrawArrays(GL_LINE_LOOP, 0, 4); 
}

void Renderer::endScene() {
    // 2. Возвращаем отрисовку на экран по умолчанию
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glad_glDisable(GL_DEPTH_TEST); // Для постпроцессинга тест глубины не нужен
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // 3. Рисуем полноэкранный квадрат с шейдером микроскопа
    m_postShader->use();
    m_postShader->setVec2("u_resolution", m_screenResolution); // Передай текущий размер экрана
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_textureColorBuffer);
    m_postShader->setInt("screenTexture", 0);

    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}