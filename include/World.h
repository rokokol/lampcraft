#pragma once
#include <glm/glm.hpp>
#include <map>
#include <optional>
#include <vector>
#include "Block.h"

/// @brief Размер мира по каждой из осей (X, Y, Z).
constexpr int WORLD_SIZE = 124;

/// @brief Результат трассировки луча (raycasting).
struct RaycastHit {
    glm::ivec3 block; ///< Координаты блока, в который попал луч.
    glm::ivec3 normal; ///< Нормаль к грани блока, в которую попал луч.
    float distance; ///< Расстояние от начала луча до точки попадания.
};


/// @brief Класс, представляющий игровой мир и управляющий блоками.
class World {
public:
    World();

    /**
     * @brief Генерирует начальный ландшафт мира.
     */
    void generateInitialWorld();

    /**
     * @brief Возвращает тип блока по заданным координатам.
     * @param x, y, z Координаты блока.
     * @return Тип блока. Возвращает Air, если координаты вне мира.
     */
    [[nodiscard]] BlockType getBlock(int x, int y, int z) const;

    /**
     * @brief Устанавливает тип блока по заданным координатам.
     * @param x, y, z Координаты блока.
     * @param type Новый тип блока.
     */
    void setBlock(int x, int y, int z, BlockType type);

    /**
     * @brief Строит меш (набор вершин) для видимых блоков мира.
     * @param cameraPos Позиция камеры для отсечения дальних блоков.
     * @param renderDistance Максимальное расстояние отрисовки.
     * @return Карта, где ключ - это тип блока, а значение - вектор вершин для этого типа.
     */
    [[nodiscard]] std::map<BlockType, std::vector<float>> buildMesh(const glm::vec3& cameraPos, float renderDistance) const;

    /**
     * @brief Выполняет трассировку луча для определения блока, на который смотрит игрок.
     * @param startPos Начальная точка луча (позиция камеры).
     * @param direction Направление луча.
     * @param maxDist Максимальная дистанция трассировки.
     * @return Структура RaycastHit, если луч попал в блок, иначе std::nullopt.
     */
    [[nodiscard]] std::optional<RaycastHit> raycast(const glm::vec3& startPos, const glm::vec3& direction, float maxDist) const;

private:
    std::vector<BlockType> m_Blocks;
};
