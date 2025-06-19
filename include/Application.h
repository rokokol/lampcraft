#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Shader.h"
#include "Camera.h"
#include "World.h"
#include <string>

#include <stb/stb_truetype.h>

/// @brief Главный класс приложения, управляющий окном, игровым циклом и рендерингом.
class Application {
public:
    Application();
    ~Application();

    /**
     * @brief Запускает главный игровой цикл.
     */
    void run();

private:
    // --- Фазы игрового цикла ---
    void init();
    void processInput();
    void update(float deltaTime);
    void render();
    void cleanup()const;

    // --- Инициализация подсистем ---
    void initWindow();
    void initOpenGL()const;
    void initShaders();
    void initTextures();
    void initText();
    void initGeometry();

    // --- Рендеринг ---
    void renderWorld(const glm::mat4& projection, const glm::mat4& view)const;
    void renderWireframe(const glm::mat4& projection, const glm::mat4& view)const;
    void renderHUD(int width, int height);
    void renderMenu(int width, int height)const;
    void renderText(const std::string& text, float x, float y, float scale, glm::vec3 color, float alpha)const;

    // --- Утилиты ---
    void updateHeldBlock(BlockType newType);

    // --- Статические колбэки для GLFW ---
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

private:
    bool m_LeftWasPressed = false;
    bool m_RightWasPressed = false;
    GLFWwindow* m_Window;
    int m_WindowWidth = 800;
    int m_WindowHeight = 600;
    const float crosshairScale = 0.5f;

    // Основные объекты
    Camera m_Camera;
    World m_World;
    
    // Шейдеры
    Shader* m_WorldShader;
    Shader* m_WireframeShader;
    Shader* m_UiShader;
    Shader* m_TextShader;

    // Состояние игры
    bool m_InMenu = false;
    bool m_IsFullscreen = false;
    BlockType m_CurrentHeldBlock = BlockType::Bricks;

    // Время
    float m_DeltaTime = 0.0f;
    float m_LastFrame = 0.0f;

    // Ввод
    bool m_FirstMouse = true;
    float m_LastX = 400;
    float m_LastY = 300;
    
    // Текст и HUD
    std::string m_CurrentBlockName;
    float m_BlockNameAlpha = 0.0f;
    float m_BlockNameTimer = 0.0f;
    GLuint m_TextVAO, m_TextVBO, m_FontTexture;
    stbtt_bakedchar m_CharData[96];

    // Ресурсы OpenGL
    GLuint m_BlockTextures[NUM_BLOCK_TYPES];
    GLuint m_VAO_world, m_VBO_world;
    GLuint m_VAO_held, m_VBO_held;
    GLuint m_VAO_menu_bg, m_VBO_menu_bg;
    GLuint m_VAO_crosshair, m_VBO_crosshair;
    GLuint m_VAO_wireframe, m_VBO_wireframe;
    
    // Настройки рендера
    const float m_RenderDistance = 16.0f;
    const glm::vec3 m_FogColor = {0.529f, 0.808f, 0.922f};
};
