#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class World; // Forward declaration

/// @brief Класс, управляющий камерой и состоянием игрока.
class Camera {
public:
    /**
     * @brief Конструктор камеры.
     * @param position Начальная позиция игрока.
     */
    Camera(glm::vec3 position);

    /**
     * @brief Обрабатывает движение мыши для изменения направления взгляда.
     * @param xoffset Смещение мыши по оси X.
     * @param yoffset Смещение мыши по оси Y.
     */
    void processMouseMovement(float xoffset, float yoffset);

    /**
     * @brief Обрабатывает нажатия клавиш для перемещения игрока.
     * @param direction Направление движения (вперед, назад, влево, вправо).
     * @param deltaTime Время, прошедшее с последнего кадра.
     * @param world Ссылка на игровой мир для проверки коллизий.
     */
    void processKeyboard(glm::vec3 velocity, float deltaTime, const World& world);
    
    /**
     * @brief Возвращает матрицу вида (view matrix).
     * @return Матрица вида.
     */
    [[nodiscard]] glm::mat4 getViewMatrix() const;

    // Геттеры для различных свойств
    [[nodiscard]] glm::vec3 getPosition() const { return m_PlayerPosition; }
    [[nodiscard]] glm::vec3 getFront() const { return m_Front; }
    [[nodiscard]] glm::vec3 getCameraPosition() const;

private:
    /**
     * @brief Обновляет векторы камеры на основе углов Эйлера (yaw и pitch).
     */
    void updateCameraVectors();
    
    /**
     * @brief Проверяет коллизию игрока с миром.
     * @param pos Позиция для проверки.
     * @param world Игровой мир.
     * @return true, если есть коллизия.
     */
    bool checkCollision(glm::vec3 pos, const World& world) const;


public:
    // Настройки камеры и игрока
    float Fov = 45.0f;
    const glm::vec3 PlayerSize = {0.6f, 1.8f, 0.6f};
    const float PlayerHeight = 1.6f;
    const float MouseSensitivity = 0.1f;
    const float MovementSpeed = 5.0f;

private:
    // Состояние камеры/игрока
    glm::vec3 m_PlayerPosition;
    glm::vec3 m_Front;
    glm::vec3 m_Up;
    float m_Yaw;
    float m_Pitch;
};
