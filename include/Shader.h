#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

/// @brief Класс для компиляции, линковки и использования шейдерных программ OpenGL.
class Shader {
public:
    /// @brief ID шейдерной программы.
    unsigned int ID;

    /**
     * @brief Конструктор, который читает и собирает шейдер из файлов.
     * @param vertexPath Путь к файлу вершинного шейдера.
     * @param fragmentPath Путь к файлу фрагментного шейдера.
     */
    Shader(const char* vertexPath, const char* fragmentPath);

    /**
     * @brief Активирует шейдерную программу.
     */
    void use() const;

    // Функции для установки uniform-переменных
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec3(const std::string &name, const glm::vec3 &value) const;
    void setVec4(const std::string &name, const glm::vec4 &value) const;
    void setMat4(const std::string &name, const glm::mat4 &mat) const;

private:
    /**
     * @brief Вспомогательная функция для проверки ошибок компиляции/линковки.
     * @param shader ID шейдера или программы.
     * @param type Тип для проверки ("PROGRAM" или "SHADER").
     */
    static void checkCompileErrors(unsigned int shader, std::string type);
};
