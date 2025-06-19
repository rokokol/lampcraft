#pragma once
#include <cstdint>
#include <string>
#include <map>

/// @brief Перечисление всех возможных типов блоков в игре.
enum class BlockType : uint8_t {
    Air = 0,
    Bricks,           // 1
    Cobblestone,      // 2
    Planes,           // 3
    Dirt,             // 4
    Glass,            // 5
    MossyCobblestone, // 6
    Glowstone,        // 7
    Grass,            // 8
    Leaves,           // 9
    Wood              // 10
};

/// @brief Количество типов блоков, имеющих текстуру (все, кроме воздуха).
constexpr int NUM_BLOCK_TYPES = 10;

/// @brief Карта, связывающая тип блока с путем к его текстуре.
const std::map<BlockType, const char*> BLOCK_TEXTURES = {
    {BlockType::Bricks,           "assets/bricks.png"},
    {BlockType::Cobblestone,      "assets/cobblestone.png"},
    {BlockType::Planes,           "assets/planes.png"},
    {BlockType::Dirt,             "assets/dirt.png"},
    {BlockType::Glass,            "assets/glass.png"},
    {BlockType::MossyCobblestone, "assets/mossy_cobblestone.png"},
    {BlockType::Glowstone,        "assets/glowstone.png"},
    {BlockType::Grass,            "assets/grass.png"},
    {BlockType::Leaves,           "assets/leaves.png"},
    {BlockType::Wood,             "assets/wood.png"}
};

/// @brief Карта, связывающая тип блока с его названием для отображения в HUD.
const std::map<BlockType, std::string> BLOCK_NAMES = {
    {BlockType::Bricks, "Bricks"},
    {BlockType::Cobblestone, "Cobblestone"},
    {BlockType::Planes, "Planes"},
    {BlockType::Dirt, "Dirt"},
    {BlockType::Glass, "Glass"},
    {BlockType::MossyCobblestone, "Mossy Cobblestone"},
    {BlockType::Glowstone, "Glowstone"},
    {BlockType::Grass, "Grass"},
    {BlockType::Leaves, "Leaves"},
    {BlockType::Wood, "Wood"}
};


/**
 * @brief Проверяет, является ли блок прозрачным.
 * Прозрачные блоки (например, стекло, листва) не скрывают за собой другие блоки.
 * @param type Тип блока для проверки.
 * @return true, если блок прозрачный, иначе false.
 */
inline bool isBlockTransparent(BlockType type) {
    return type == BlockType::Air || type == BlockType::Glass || type == BlockType::Leaves;
}
