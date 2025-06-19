#include "World.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// --- Функции-помощники для создания геометрии ---
namespace {
    void addFace(std::vector<float>& vertices, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, const glm::vec3& v4) {
        vertices.insert(vertices.end(), {
            v1.x, v1.y, v1.z, 0.0f, 0.0f,
            v2.x, v2.y, v2.z, 0.0f, 1.0f,
            v3.x, v3.y, v3.z, 1.0f, 1.0f,
            v1.x, v1.y, v1.z, 0.0f, 0.0f,
            v3.x, v3.y, v3.z, 1.0f, 1.0f,
            v4.x, v4.y, v4.z, 1.0f, 0.0f
        });
    }
    void addTop(std::vector<float>& v, float i, float j, float k) { addFace(v, {i, j + 1, k}, {i + 1, j + 1, k}, {i + 1, j + 1, k + 1}, {i, j + 1, k + 1}); }
    void addBottom(std::vector<float>& v, float i, float j, float k) { addFace(v, {i, j, k + 1}, {i + 1, j, k + 1}, {i + 1, j, k}, {i, j, k}); }
    void addLeft(std::vector<float>& v, float i, float j, float k) { addFace(v, {i, j, k}, {i, j + 1, k}, {i, j + 1, k + 1}, {i, j, k + 1}); }
    void addRight(std::vector<float>& v, float i, float j, float k) { addFace(v, {i + 1, j, k + 1}, {i + 1, j + 1, k + 1}, {i + 1, j + 1, k}, {i + 1, j, k}); }
    void addFront(std::vector<float>& v, float i, float j, float k) { addFace(v, {i + 1, j, k}, {i + 1, j + 1, k}, {i, j + 1, k}, {i, j, k}); }
    void addBack(std::vector<float>& v, float i, float j, float k) { addFace(v, {i, j, k + 1}, {i, j + 1, k + 1}, {i + 1, j + 1, k + 1}, {i + 1, j, k + 1}); }
}

World::World() : m_Blocks(WORLD_SIZE * WORLD_SIZE * WORLD_SIZE, BlockType::Air) {}

void World::generateInitialWorld() {
    for (int i = 0; i < WORLD_SIZE; i++) {
        for (int k = 0; k < WORLD_SIZE; k++) {
            setBlock(i, 0, k, BlockType::Grass);
        }
    }
}

BlockType World::getBlock(int x, int y, int z) const {
    if (x < 0 || x >= WORLD_SIZE || y < 0 || y >= WORLD_SIZE || z < 0 || z >= WORLD_SIZE) {
        return BlockType::Air; // Считаем, что за пределами мира - воздух
    }
    return m_Blocks[x * WORLD_SIZE * WORLD_SIZE + y * WORLD_SIZE + z];
}

void World::setBlock(int x, int y, int z, BlockType type) {
    if (x >= 0 && x < WORLD_SIZE && y >= 0 && y < WORLD_SIZE && z >= 0 && z < WORLD_SIZE) {
        m_Blocks[x * WORLD_SIZE * WORLD_SIZE + y * WORLD_SIZE + z] = type;
    }
}

std::map<BlockType, std::vector<float>> World::buildMesh(const glm::vec3& cameraPos, float renderDistance) const {
    std::map<BlockType, std::vector<float>> meshes;

    for (int i = 0; i < WORLD_SIZE; i++) {
        for (int j = 0; j < WORLD_SIZE; j++) {
            for (int k = 0; k < WORLD_SIZE; k++) {
                BlockType currentType = getBlock(i, j, k);
                if (currentType == BlockType::Air) continue;

                // Отсечение по расстоянию (frustum culling)
                if (glm::distance(glm::vec3(i, j, k), cameraPos) > renderDistance + 2.0f) continue;

                //  Захватываем this для вызова getBlock
                auto isNeighborTransparent = [&](int x, int y, int z) {
                    return isBlockTransparent(this->getBlock(x,y,z));
                };

                // Добавляем грань, только если соседний блок прозрачный
                if (isNeighborTransparent(i, j + 1, k)) addTop(meshes[currentType], i, j, k);
                if (isNeighborTransparent(i, j - 1, k)) addBottom(meshes[currentType], i, j, k);
                if (isNeighborTransparent(i - 1, j, k)) addLeft(meshes[currentType], i, j, k);
                if (isNeighborTransparent(i + 1, j, k)) addRight(meshes[currentType], i, j, k);
                if (isNeighborTransparent(i, j, k - 1)) addFront(meshes[currentType], i, j, k);
                if (isNeighborTransparent(i, j, k + 1)) addBack(meshes[currentType], i, j, k);
            }
        }
    }
    return meshes;
}

std::optional<RaycastHit> World::raycast(const glm::vec3& startPos, const glm::vec3& direction, float maxDist) const {
    glm::ivec3 last_air_block = glm::floor(startPos);

    for (float t = 0.0f; t < maxDist; t += 0.02f) {
        glm::vec3 p = startPos + direction * t;
        glm::ivec3 current_block = glm::floor(p);

        if (getBlock(current_block.x, current_block.y, current_block.z) != BlockType::Air) {
            glm::ivec3 normal = last_air_block - current_block;
            return RaycastHit{current_block, normal, t};
        }
        last_air_block = current_block;
    }
    return std::nullopt;
}
