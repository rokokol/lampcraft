#include "Camera.h"
#include "World.h"

Camera::Camera(glm::vec3 position) :
    m_PlayerPosition(position), m_Front(glm::vec3(0.0f, 0.0f, -1.0f)), m_Up(glm::vec3(0.0f, 1.0f, 0.0f)), m_Yaw(-90.0f),
    m_Pitch(0.0f) {
    updateCameraVectors();
}

void Camera::processMouseMovement(float xoffset, float yoffset) {
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    m_Yaw += xoffset;
    m_Pitch += yoffset;

    // Ограничиваем угол Pitch, чтобы избежать "переворота" камеры
    m_Pitch = glm::clamp(m_Pitch, -89.0f, 89.0f);

    updateCameraVectors();
}

void Camera::processKeyboard(glm::vec3 velocity, float deltaTime, const World &world) {
    if (glm::length(velocity) > 0.0f) {
        velocity = glm::normalize(velocity) * MovementSpeed * deltaTime;

        glm::vec3 newPos = m_PlayerPosition;

        // Проверяем коллизии по каждой оси отдельно для "скольжения" вдоль стен
        newPos.x += velocity.x;
        if (checkCollision(newPos, world)) {
            newPos.x -= velocity.x;
        }
        newPos.y += velocity.y;
        if (checkCollision(newPos, world)) {
            newPos.y -= velocity.y;
        }
        newPos.z += velocity.z;
        if (checkCollision(newPos, world)) {
            newPos.z -= velocity.z;
        }
        m_PlayerPosition = newPos;
    }
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(getCameraPosition(), getCameraPosition() + m_Front, m_Up);
}

glm::vec3 Camera::getCameraPosition() const {
    // Камера находится на уровне глаз игрока
    return m_PlayerPosition + glm::vec3(0.0f, PlayerHeight, 0.0f);
}

void Camera::updateCameraVectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    front.y = sin(glm::radians(m_Pitch));
    front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    m_Front = glm::normalize(front);
}

bool Camera::checkCollision(glm::vec3 pos, const World &world) const {
    // Определяем AABB (ограничивающий параллелепипед) игрока
    glm::vec3 playerMin = {pos.x - PlayerSize.x / 2, pos.y, pos.z - PlayerSize.z / 2};
    glm::vec3 playerMax = {pos.x + PlayerSize.x / 2, pos.y + PlayerSize.y, pos.z + PlayerSize.z / 2};

    // Определяем, какие блоки мира пересекаются с AABB игрока
    glm::ivec3 minBlock = glm::floor(playerMin);
    glm::ivec3 maxBlock = glm::ceil(playerMax);

    // Проверяем каждый блок в этой области
    for (int x = minBlock.x; x < maxBlock.x; ++x) {
        for (int y = minBlock.y; y < maxBlock.y; ++y) {
            for (int z = minBlock.z; z < maxBlock.z; ++z) {
                if (world.getBlock(x, y, z) != BlockType::Air) {
                    // Простая AABB-AABB проверка на пересечение
                    glm::vec3 blockMin = {(float) x, (float) y, (float) z};
                    glm::vec3 blockMax = {(float) x + 1, (float) y + 1, (float) z + 1};
                    if (playerMax.x > blockMin.x && playerMin.x < blockMax.x && playerMax.y > blockMin.y && playerMin.y < blockMax.y &&
                        playerMax.z > blockMin.z && playerMin.z < blockMax.z) {
                        return true; // Найдена коллизия
                    }
                }
            }
        }
    }
    return false; // Коллизий не найдено
}
