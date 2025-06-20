cmake_minimum_required(VERSION 3.16)
project(LampCraft)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)

# Включаем FetchContent для загрузки зависимостей
include(FetchContent)

# --- Зависимости ---

# GLFW
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.3.8 # Используем стабильный тег вместо latest
)

# GLM
FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 0.9.9.8 # Используем стабильный тег вместо master
)

# glad
FetchContent_Declare(
        glad
        GIT_REPOSITORY https://github.com/Dav1dde/glad.git
        GIT_TAG v0.1.36 # Используем стабильный тег вместо master
)

# stb (stb_image, stb_truetype)
FetchContent_Declare(
        stb
        GIT_REPOSITORY https://github.com/nothings/stb.git
        GIT_TAG master
)

FetchContent_MakeAvailable(glfw glm glad stb)

# Указываем, где искать заголовочные файлы stb
include_directories(${stb_SOURCE_DIR})

# --- Исполняемый файл ---

# Добавляем исполняемый файл и все его исходники
add_executable(LampCraft
        src/main.cpp
        src/Application.cpp
        src/Camera.cpp
        src/Shader.cpp
        src/World.cpp
        include/Block.h
        include/Camera.h
        include/Shader.h
        include/World.h
        include/Application.h
        src/Camera.cpp
        src/Shader.cpp
        src/World.cpp
        src/Application.cpp
)

# Указываем директорию с нашими заголовочными файлами
target_include_directories(LampCraft PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${glad_SOURCE_DIR}/include
)

# Линкуем библиотеки
target_link_libraries(LampCraft PRIVATE glfw glad glm)

# --- Копирование ресурсов ---
# Эта команда скопирует папки assets и shaders в папку, где будет лежать .exe
# Это позволяет использовать относительные пути "assets/font.otf" в коде
file(COPY assets shaders DESTINATION ${CMAKE_BINARY_DIR})
