#include "Application.h"
#include <algorithm>
#include <iostream>
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include <glm/gtc/type_ptr.hpp>
#include <stb/stb_truetype.h>

// --- Помощники для геометрии ---
namespace {
// Функции для создания граней куба
void addFace(std::vector<float> &v, const glm::vec3 &v1, const glm::vec3 &v2,
             const glm::vec3 &v3, const glm::vec3 &v4) {
  v.insert(v.end(),
           {v1.x, v1.y, v1.z, 0.0f, 0.0f, v2.x, v2.y, v2.z, 0.0f, 1.0f,
            v3.x, v3.y, v3.z, 1.0f, 1.0f, v1.x, v1.y, v1.z, 0.0f, 0.0f,
            v3.x, v3.y, v3.z, 1.0f, 1.0f, v4.x, v4.y, v4.z, 1.0f, 0.0f});
}
void addTop(std::vector<float> &v, float i, float j, float k) {
  addFace(v, {i, j + 1, k}, {i + 1, j + 1, k}, {i + 1, j + 1, k + 1},
          {i, j + 1, k + 1});
}
void addBottom(std::vector<float> &v, float i, float j, float k) {
  addFace(v, {i, j, k + 1}, {i + 1, j, k + 1}, {i + 1, j, k}, {i, j, k});
}
void addLeft(std::vector<float> &v, float i, float j, float k) {
  addFace(v, {i, j, k}, {i, j + 1, k}, {i, j + 1, k + 1}, {i, j, k + 1});
}
void addRight(std::vector<float> &v, float i, float j, float k) {
  addFace(v, {i + 1, j, k + 1}, {i + 1, j + 1, k + 1}, {i + 1, j + 1, k},
          {i + 1, j, k});
}
void addFront(std::vector<float> &v, float i, float j, float k) {
  addFace(v, {i + 1, j, k}, {i + 1, j + 1, k}, {i, j + 1, k}, {i, j, k});
}
void addBack(std::vector<float> &v, float i, float j, float k) {
  addFace(v, {i, j, k + 1}, {i, j + 1, k + 1}, {i + 1, j + 1, k + 1},
          {i + 1, j, k + 1});
}

// Функция для создания каркаса (wireframe) куба
void addWire(std::vector<float> &v, float i, float j, float k) {
  glm::vec3 pts[8] = {{i, j, k},
                      {i + 1, j, k},
                      {i + 1, j + 1, k},
                      {i, j + 1, k},
                      {i, j, k + 1},
                      {i + 1, j, k + 1},
                      {i + 1, j + 1, k + 1},
                      {i, j + 1, k + 1}};
  int e[24] = {0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6,
               6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7};
  for (int idx : e) {
    v.push_back(pts[idx].x);
    v.push_back(pts[idx].y);
    v.push_back(pts[idx].z);
  }
}
} // namespace

Application::Application()
    : m_Window(nullptr),
      m_Camera({WORLD_SIZE / 2.0f + 0.5f, 1.5f, WORLD_SIZE / 2.0f + 0.5f}),
      m_WorldShader(nullptr), m_WireframeShader(nullptr), m_UiShader(nullptr),
      m_TextShader(nullptr), m_TextVAO(0), m_TextVBO(0), m_FontTexture(0),
      m_CharData{}, m_BlockTextures{}, m_VAO_world(0), m_VBO_world(0),
      m_VAO_held(0), m_VBO_held(0), m_VAO_menu_bg(0), m_VBO_menu_bg(0),
      m_VAO_crosshair(0), m_VBO_crosshair(0), m_VAO_wireframe(0),
      m_VBO_wireframe(0) {}

Application::~Application() { cleanup(); }

void Application::run() {
  init();
  while (!glfwWindowShouldClose(m_Window)) {
    auto currentFrame = static_cast<float>(glfwGetTime());
    m_DeltaTime = currentFrame - m_LastFrame;
    m_LastFrame = currentFrame;

    processInput();
    update(m_DeltaTime);
    render();

    glfwSwapBuffers(m_Window);
    glfwPollEvents();
  }
}

// --- ИНИЦИАЛИЗАЦИЯ ---

void Application::init() {
  initWindow();
  initOpenGL();
  initShaders();
  initTextures();
  initText();
  initGeometry();
  m_World.generateInitialWorld();
  updateHeldBlock(BlockType::Bricks);
}

void Application::initWindow() {
  if (!glfwInit()) {
    throw std::runtime_error("Failed to initialize GLFW");
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  m_Window = glfwCreateWindow(m_WindowWidth, m_WindowHeight, "LampCraft",
                              nullptr, nullptr);
  if (!m_Window) {
    glfwTerminate();
    throw std::runtime_error("Failed to create GLFW window");
  }
  glfwMakeContextCurrent(m_Window);

  // Сохраняем указатель на этот объект, чтобы использовать в статических
  // колбэках
  glfwSetWindowUserPointer(m_Window, this);

  // Устанавливаем колбэки
  glfwSetFramebufferSizeCallback(m_Window, framebuffer_size_callback);
  glfwSetCursorPosCallback(m_Window, mouse_callback);
  glfwSetScrollCallback(m_Window, scroll_callback);
  glfwSetKeyCallback(m_Window, key_callback);

  glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Application::initOpenGL() const {
  if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
    throw std::runtime_error("Failed to initialize GLAD");
  }
  glViewport(0, 0, m_WindowWidth, m_WindowHeight);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glLineWidth(2.0f);
  glClearColor(m_FogColor.r, m_FogColor.g, m_FogColor.b, 1.0f);
}

void Application::initShaders() {
  m_WorldShader = new Shader("shaders/world.vert", "shaders/world.frag");
  m_WireframeShader =
      new Shader("shaders/wireframe.vert", "shaders/wireframe.frag");
  m_UiShader = new Shader("shaders/ui.vert", "shaders/ui.frag");
  m_TextShader = new Shader("shaders/text.vert", "shaders/text.frag");
}

void Application::initTextures() {
  stbi_set_flip_vertically_on_load(true);
  glGenTextures(NUM_BLOCK_TYPES, m_BlockTextures);

  int i = 0;
  for (const auto &pair : BLOCK_TEXTURES) {
    int width, height, nrChannels;
    if (unsigned char *data = stbi_load(pair.second, &width, &height,
                                        &nrChannels, STBI_rgb_alpha)) {
      glBindTexture(GL_TEXTURE_2D, m_BlockTextures[i]);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                   GL_UNSIGNED_BYTE, data);
      glGenerateMipmap(GL_TEXTURE_2D);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                      GL_NEAREST_MIPMAP_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      stbi_image_free(data);
      i++;
    } else {
      std::cerr << "Failed to load texture: " << pair.second << std::endl;
    }
  }
}

void Application::initText() {
  FILE *fontFile = fopen("assets/font.otf", "rb");
  if (!fontFile) {
    std::cerr << "Failed to load font assets/font.otf" << std::endl;
    return;
  }
  fseek(fontFile, 0, SEEK_END);
  long size = ftell(fontFile);
  fseek(fontFile, 0, SEEK_SET);
  auto *ttf_buffer = new unsigned char[size];
  fread(ttf_buffer, 1, size, fontFile);
  fclose(fontFile);

  unsigned char temp_bitmap[512 * 512];
  stbtt_BakeFontBitmap(ttf_buffer, 0, 48.0, temp_bitmap, 512, 512, 32, 96,
                       m_CharData);
  delete[] ttf_buffer;

  glGenTextures(1, &m_FontTexture);
  glBindTexture(GL_TEXTURE_2D, m_FontTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 512, 512, 0, GL_RED, GL_UNSIGNED_BYTE,
               temp_bitmap);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glGenVertexArrays(1, &m_TextVAO);
  glGenBuffers(1, &m_TextVBO);
  glBindVertexArray(m_TextVAO);
  glBindBuffer(GL_ARRAY_BUFFER, m_TextVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr,
               GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void Application::initGeometry() {
  // Мир
  glGenVertexArrays(1, &m_VAO_world);
  glGenBuffers(1, &m_VBO_world);
  glBindVertexArray(m_VAO_world);
  glBindBuffer(GL_ARRAY_BUFFER, m_VBO_world);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        static_cast<void *>(nullptr));
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        reinterpret_cast<void *>(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // Фон меню
  glGenVertexArrays(1, &m_VAO_menu_bg);
  glGenBuffers(1, &m_VBO_menu_bg);
  glBindVertexArray(m_VAO_menu_bg);
  glBindBuffer(GL_ARRAY_BUFFER, m_VBO_menu_bg);
  float menuBgVertices[] = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
                            0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f};
  glBufferData(GL_ARRAY_BUFFER, sizeof(menuBgVertices), menuBgVertices,
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
                        static_cast<void *>(nullptr));
  glEnableVertexAttribArray(0);

  // Прицел
  glGenVertexArrays(1, &m_VAO_crosshair);
  glGenBuffers(1, &m_VBO_crosshair);
  glBindVertexArray(m_VAO_crosshair);
  glBindBuffer(GL_ARRAY_BUFFER, m_VBO_crosshair);
  float crosshairVerts[] = {-15.0f, 0.0f,   15.0f, 0.0f,
                            0.0f,   -15.0f, 0.0f,  15.0f};
  glBufferData(GL_ARRAY_BUFFER, sizeof(crosshairVerts), crosshairVerts,
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
                        static_cast<void *>(nullptr));
  glEnableVertexAttribArray(0);

  // Блок в руке
  glGenVertexArrays(1, &m_VAO_held);
  glGenBuffers(1, &m_VBO_held);
  std::vector<float> heldVerts;
  addTop(heldVerts, -0.5f, -0.5f, -0.5f);
  addBottom(heldVerts, -0.5f, -0.5f, -0.5f);
  addLeft(heldVerts, -0.5f, -0.5f, -0.5f);
  addRight(heldVerts, -0.5f, -0.5f, -0.5f);
  addFront(heldVerts, -0.5f, -0.5f, -0.5f);
  addBack(heldVerts, -0.5f, -0.5f, -0.5f);
  glBindVertexArray(m_VAO_held);
  glBindBuffer(GL_ARRAY_BUFFER, m_VBO_held);
  glBufferData(GL_ARRAY_BUFFER, heldVerts.size() * sizeof(float),
               heldVerts.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        static_cast<void *>(nullptr));
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        reinterpret_cast<void *>(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // Рамка выделения
  glGenVertexArrays(1, &m_VAO_wireframe);
  glGenBuffers(1, &m_VBO_wireframe);
  glBindVertexArray(m_VAO_wireframe);
  glBindBuffer(GL_ARRAY_BUFFER, m_VBO_wireframe);
  glBufferData(GL_ARRAY_BUFFER, 24 * 3 * sizeof(float), nullptr,
               GL_DYNAMIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                        static_cast<void *>(nullptr));
  glEnableVertexAttribArray(0);

  glBindVertexArray(0);
}

// --- ИГРОВОЙ ЦИКЛ ---

void Application::processInput() {
  if (glfwGetKey(m_Window, GLFW_KEY_Q) == GLFW_PRESS && m_InMenu) {
    glfwSetWindowShouldClose(m_Window, true);
  }

  if (!m_InMenu) {
    glm::vec3 velocity(0.0f);
    glm::vec3 moveDir = glm::normalize(
        glm::vec3(m_Camera.getFront().x, 0.0f, m_Camera.getFront().z));

    if (glfwGetKey(m_Window, GLFW_KEY_W) == GLFW_PRESS)
      velocity += moveDir;
    if (glfwGetKey(m_Window, GLFW_KEY_S) == GLFW_PRESS)
      velocity -= moveDir;
    if (glfwGetKey(m_Window, GLFW_KEY_A) == GLFW_PRESS)
      velocity -=
          glm::normalize(glm::cross(m_Camera.getFront(), glm::vec3(0, 1, 0)));
    if (glfwGetKey(m_Window, GLFW_KEY_D) == GLFW_PRESS)
      velocity +=
          glm::normalize(glm::cross(m_Camera.getFront(), glm::vec3(0, 1, 0)));
    if (glfwGetKey(m_Window, GLFW_KEY_SPACE) == GLFW_PRESS)
      velocity.y += 1.0f;
    if (glfwGetKey(m_Window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
      velocity.y -= 1.0f;

    m_Camera.processKeyboard(velocity, m_DeltaTime, m_World);

    // Edge-detect left click
    bool right =
        glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    if (right && !m_LeftWasPressed) {
      if (auto hit = m_World.raycast(m_Camera.getCameraPosition(),
                                     m_Camera.getFront(), m_RenderDistance)) {
        glm::ivec3 pos = hit->block + hit->normal;
        glm::vec3 pMin = {m_Camera.getPosition().x - m_Camera.PlayerSize.x / 2,
                          m_Camera.getPosition().y,
                          m_Camera.getPosition().z - m_Camera.PlayerSize.z / 2};
        glm::vec3 pMax = {m_Camera.getPosition().x + m_Camera.PlayerSize.x / 2,
                          m_Camera.getPosition().y + m_Camera.PlayerSize.y,
                          m_Camera.getPosition().z + m_Camera.PlayerSize.z / 2};
        glm::vec3 bMin = {static_cast<float>(pos.x), static_cast<float>(pos.y),
                          static_cast<float>(pos.z)};
        glm::vec3 bMax = {static_cast<float>(pos.x + 1),
                          static_cast<float>(pos.y + 1),
                          static_cast<float>(pos.z + 1)};
        if (!(pMax.x > bMin.x && pMin.x < bMax.x && pMax.y > bMin.y &&
              pMin.y < bMax.y && pMax.z > bMin.z && pMin.z < bMax.z)) {
          m_World.setBlock(pos.x, pos.y, pos.z, m_CurrentHeldBlock);
        }
      }
    }
    m_LeftWasPressed = right;

    // Edge-detect right click
    bool left =
        glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    if (left && !m_RightWasPressed) {
      if (auto hit = m_World.raycast(m_Camera.getCameraPosition(),
                                     m_Camera.getFront(), m_RenderDistance)) {
        m_World.setBlock(hit->block.x, hit->block.y, hit->block.z,
                         BlockType::Air);
      }
    }
    m_RightWasPressed = left;
  }
}

void Application::update(float deltaTime) {
  if (m_BlockNameTimer > 0) {
    m_BlockNameTimer -= deltaTime;
    m_BlockNameAlpha = std::min(1.0f, m_BlockNameTimer);
  } else {
    m_BlockNameAlpha = 0.0f;
  }
}

void Application::render() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);

  int width, height;
  glfwGetWindowSize(m_Window, &width, &height);
  if (height == 0)
    height = 1;

  glm::mat4 projection =
      glm::perspective(glm::radians(m_Camera.Fov),
                       static_cast<float>(width) / height, 0.1f, 300.0f);
  glm::mat4 view = m_Camera.getViewMatrix();

  renderWorld(projection, view);

  if (!m_InMenu) {
    renderWireframe(projection, view);
    renderHUD(width, height);
  } else {
    renderMenu(width, height);
  }
}

// --- РЕНДЕР-ФУНКЦИИ ---

void Application::renderWorld(const glm::mat4 &projection,
                              const glm::mat4 &view) const {
  auto meshes =
      m_World.buildMesh(m_Camera.getCameraPosition(), m_RenderDistance);

  m_WorldShader->use();
  m_WorldShader->setMat4("projection", projection);
  m_WorldShader->setMat4("view", view);
  m_WorldShader->setMat4("model", glm::mat4(1.0f));
  m_WorldShader->setVec3("fogColor", m_FogColor);
  m_WorldShader->setFloat("fogStart", m_RenderDistance * 0.7f);
  m_WorldShader->setFloat("fogEnd", m_RenderDistance);
  m_WorldShader->setBool("enableFog", true);
  m_WorldShader->setInt("texSampler", 0);

  glActiveTexture(GL_TEXTURE0);
  glBindVertexArray(m_VAO_world);
  glBindBuffer(GL_ARRAY_BUFFER, m_VBO_world);

  for (const auto &pair : meshes) {
    BlockType type = pair.first;
    const auto &vertices = pair.second;
    if (vertices.empty())
      continue;

    int textureIndex = static_cast<int>(type) - 1;
    if (textureIndex >= 0 && textureIndex < NUM_BLOCK_TYPES) {
      glBindTexture(GL_TEXTURE_2D, m_BlockTextures[textureIndex]);
      glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
                   vertices.data(), GL_DYNAMIC_DRAW);
      glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size() / 5));
    }
  }
}

void Application::renderWireframe(const glm::mat4 &projection,
                                  const glm::mat4 &view) const {
  if (auto hit = m_World.raycast(m_Camera.getCameraPosition(),
                                 m_Camera.getFront(), m_RenderDistance)) {
    std::vector<float> wire_verts;
    addWire(wire_verts, hit->block.x, hit->block.y, hit->block.z);

    m_WireframeShader->use();
    m_WireframeShader->setMat4("MVP", projection * view * glm::mat4(1.0f));

    glBindVertexArray(m_VAO_wireframe);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO_wireframe);
    glBufferSubData(GL_ARRAY_BUFFER, 0, wire_verts.size() * sizeof(float),
                    wire_verts.data());
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(wire_verts.size() / 3));
  }
}

void Application::renderHUD(int width, int height) {
  glClear(GL_DEPTH_BUFFER_BIT);

  // --- Блок в руке ---
  m_WorldShader->use();
  m_WorldShader->setBool("enableFog", false);
  glm::mat4 proj =
      glm::perspective(glm::radians(m_Camera.Fov),
                       static_cast<float>(width) / height, 0.1f, 100.0f);
  glm::mat4 heldModel = glm::translate(glm::mat4(1.0f), {0.6f, -0.5f, -1.0f});
  heldModel = glm::scale(heldModel, glm::vec3(0.3f));
  heldModel = glm::rotate(heldModel, glm::radians(25.0f), {1, 0, 0});
  heldModel = glm::rotate(heldModel,
                          glm::radians(-35.0f) +
                              static_cast<float>(glfwGetTime()) * 0.5f,
                          {0, 1, 0});
  m_WorldShader->setMat4("projection", proj);
  m_WorldShader->setMat4("view", glm::mat4(1.0f));
  m_WorldShader->setMat4("model", heldModel);
  int ti = static_cast<int>(m_CurrentHeldBlock) - 1;
  if (0 <= ti && ti < NUM_BLOCK_TYPES)
    glBindTexture(GL_TEXTURE_2D, m_BlockTextures[ti]);
  glBindVertexArray(m_VAO_held);
  glDrawArrays(GL_TRIANGLES, 0, 36);

  // --- 2D HUD ---
  glDisable(GL_DEPTH_TEST);

  // Название блока
  if (m_BlockNameAlpha > 0.0f) {
    renderText(m_CurrentBlockName, 25.0f, height - 25.0f, 1.0f,
               {1.0f, 1.0f, 1.0f}, m_BlockNameAlpha);
  }

  // Прицел
  m_UiShader->use();
  glm::mat4 ortho = glm::ortho(0.0f, static_cast<float>(width), 0.0f,
                               static_cast<float>(height));
  m_UiShader->setMat4("projection", ortho);
  m_UiShader->setVec4("color", {1.0f, 1.0f, 1.0f, 1.0f});
  glm::mat4 model =
      glm::translate(glm::mat4(1.0f), {width / 2.0f, height / 2.0f, 0.0f});
  model = glm::scale(model, glm::vec3(crosshairScale, crosshairScale, 1.0f));
  m_UiShader->setMat4("model", model);
  glBindVertexArray(m_VAO_crosshair);
  glDrawArrays(GL_LINES, 0, 4);
  glBindVertexArray(0);
}

void Application::renderMenu(int width, int height) const {
  glDisable(GL_DEPTH_TEST);

  // Полупрозрачный фон
  m_UiShader->use();
  glm::mat4 ortho_proj = glm::ortho(0.0f, static_cast<float>(width), 0.0f,
                                    static_cast<float>(height));
  m_UiShader->setMat4("projection", ortho_proj);
  m_UiShader->setVec4("color", {0.1f, 0.1f, 0.1f, 0.7f});
  glm::mat4 bg_model =
      glm::scale(glm::mat4(1.0f), glm::vec3(width, height, 1.0f));
  m_UiShader->setMat4("model", bg_model);
  glBindVertexArray(m_VAO_menu_bg);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  // Мигающее "PAUSE" в правом нижнем углу
  float blinkAlpha =
      (sin(static_cast<float>(glfwGetTime()) * 4.0f) + 1.0f) / 2.0f;
  renderText("PAUSE",
             width - 80.0f -
                 80.0f,      // x = справа − отступ − прибл. ширина текста
             height - 35.0f, // y = height−25, 25 от низа
             1.0f, glm::vec3(1.0f), blinkAlpha);

  // Остальные подсказки (центр экрана) оставляем без изменений
  std::string hints = R"(
        LAMPCRAFT v1.0.0

        [WASD]/[arrows] to move
        [scroll]/[0-9] to pick a block

        [ESC] to resume
        [F11] to fullscreen
        [Q] to quit
    )";
  renderText(hints, width / 2.0f - 400.0f, height * 0.4f - 150, 1.0f,
             glm::vec3(1.0f), 1.0f);
}

void Application::renderText(const std::string &text, float x, float y,
                             const float scale, glm::vec3 color,
                             float alpha) const {
  int w, h;
  glfwGetWindowSize(m_Window, &w, &h);

  m_TextShader->use();
  m_TextShader->setVec3("textColor", color);
  m_TextShader->setFloat("textAlpha", alpha);
  m_TextShader->setMat4("projection", glm::ortho(0.0f, static_cast<float>(w),
                                                 static_cast<float>(h), 0.0f));
  m_TextShader->setInt("text", 0);

  glActiveTexture(GL_TEXTURE0);
  glBindVertexArray(m_TextVAO);
  glBindTexture(GL_TEXTURE_2D, m_FontTexture);

  float startX = x;
  for (const char *p = text.c_str(); *p; ++p) {
    if (*p == '\n') {
      y += 48.0f * scale;
      x = startX;
      continue;
    }
    stbtt_aligned_quad q;
    stbtt_GetBakedQuad(m_CharData, 512, 512, *p - 32, &x, &y, &q, 1);
    float verts[6][4] = {{q.x0, q.y0, q.s0, q.t0}, {q.x1, q.y0, q.s1, q.t0},
                         {q.x1, q.y1, q.s1, q.t1}, {q.x0, q.y0, q.s0, q.t0},
                         {q.x1, q.y1, q.s1, q.t1}, {q.x0, q.y1, q.s0, q.t1}};
    glBindBuffer(GL_ARRAY_BUFFER, m_TextVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
    glDrawArrays(GL_TRIANGLES, 0, 6);
  }
  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

// --- УТИЛИТЫ И КОЛБЭКИ ---

void Application::cleanup() const {
  delete m_WorldShader;
  delete m_WireframeShader;
  delete m_UiShader;
  delete m_TextShader;

  glDeleteVertexArrays(1, &m_VAO_world);
  glDeleteBuffers(1, &m_VBO_world);
  glDeleteVertexArrays(1, &m_VAO_menu_bg);
  glDeleteBuffers(1, &m_VBO_menu_bg);
  glDeleteVertexArrays(1, &m_VAO_held);
  glDeleteBuffers(1, &m_VBO_held);
  glDeleteVertexArrays(1, &m_VAO_crosshair);
  glDeleteBuffers(1, &m_VBO_crosshair);
  glDeleteVertexArrays(1, &m_VAO_wireframe);
  glDeleteBuffers(1, &m_VBO_wireframe);
  glDeleteVertexArrays(1, &m_TextVAO);
  glDeleteBuffers(1, &m_TextVBO);

  glDeleteTextures(NUM_BLOCK_TYPES, m_BlockTextures);
  glDeleteTextures(1, &m_FontTexture);

  glfwTerminate();
}

void Application::updateHeldBlock(BlockType newType) {
  if (newType != m_CurrentHeldBlock) {
    m_CurrentHeldBlock = newType;
    m_CurrentBlockName = BLOCK_NAMES.at(m_CurrentHeldBlock);
    m_BlockNameTimer = 3.0f;
    m_BlockNameAlpha = 1.0f;
  }
}

void Application::framebuffer_size_callback(GLFWwindow *window, int width,
                                            int height) {
  glViewport(0, 0, width, height);
  if (auto *app =
          static_cast<Application *>(glfwGetWindowUserPointer(window))) {
    app->m_WindowWidth = width;
    app->m_WindowHeight = height;
  }
}

void Application::mouse_callback(GLFWwindow *window, double xpos, double ypos) {
  auto *app = static_cast<Application *>(glfwGetWindowUserPointer(window));
  if (app && !app->m_InMenu) {
    if (app->m_FirstMouse) {
      app->m_LastX = static_cast<float>(xpos);
      app->m_LastY = static_cast<float>(ypos);
      app->m_FirstMouse = false;
    }
    float xoffset = static_cast<float>(xpos) - app->m_LastX;
    float yoffset = app->m_LastY - static_cast<float>(ypos);
    app->m_LastX = static_cast<float>(xpos);
    app->m_LastY = static_cast<float>(ypos);
    app->m_Camera.processMouseMovement(xoffset, yoffset);
  }
}

void Application::scroll_callback(GLFWwindow *window, double xoffset,
                                  double yoffset) {
  auto app = static_cast<Application *>(glfwGetWindowUserPointer(window));
  if (app && !app->m_InMenu) {
    int current = static_cast<int>(app->m_CurrentHeldBlock);
    int numBlockTypes = static_cast<int>(BlockType::Wood);
    if (yoffset < 0) { // Scroll down
      current = (current % numBlockTypes) + 1;
    } else { // Scroll up
      current--;
      if (current < 1)
        current = numBlockTypes;
    }
    app->updateHeldBlock(static_cast<BlockType>(current));
  }
}

void Application::key_callback(GLFWwindow *window, int key, int scancode,
                               int action, int mods) {
  auto app = static_cast<Application *>(glfwGetWindowUserPointer(window));
  if (!app)
    return;

  if (action == GLFW_PRESS) {
    // Выход из меню/игры
    if (key == GLFW_KEY_ESCAPE) {
      app->m_InMenu = !app->m_InMenu;
      if (app->m_InMenu) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
      } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        app->m_FirstMouse = true;
      }
    }
    // Полноэкранный режим
    if (key == GLFW_KEY_F11) {
      app->m_IsFullscreen = !app->m_IsFullscreen;
      GLFWmonitor *monitor = glfwGetPrimaryMonitor();
      const GLFWvidmode *mode = glfwGetVideoMode(monitor);
      if (app->m_IsFullscreen) {
        glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height,
                             mode->refreshRate);
            } else {
                glfwSetWindowMonitor(window, nullptr, 100, 100, 800, 600, 0);
            }
        }
        // Выбор блока по цифрам
        if (key >= GLFW_KEY_1 && key <= GLFW_KEY_9) {
            app->updateHeldBlock(static_cast<BlockType>(key - GLFW_KEY_1 + 1));
        }
        if (key == GLFW_KEY_0) {
            app->updateHeldBlock(BlockType::Wood); // '0' соответствует 10-му блоку
        }
    }
}
