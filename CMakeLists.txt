set(CMAKE_POLICY_VERSION_MINIMUM 3.12)

cmake_minimum_required(VERSION 3.20)
project(Vulkan3dRenderer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Настройки конфигураций
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Настройки выходных директорий
set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}")
set(OUTPUT_INT_DIR "${CMAKE_BINARY_DIR}/bin-int/${CMAKE_BUILD_TYPE}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})

# Включение подпроектов
add_subdirectory("Vulkan3dRenderer/vendor/glfw")
add_subdirectory("Vulkan3dRenderer/vendor/glm")

# Настройка исполняемого файла
file(GLOB_RECURSE SOURCES 
    "Vulkan3dRenderer/src/*.h"
    "Vulkan3dRenderer/src/*.cpp"
    "Vulkan3dRenderer/src/*.hpp"
)

add_executable(Vulkan3dRenderer ${SOURCES})

# Поиск Vulkan SDK
if(DEFINED ENV{VULKAN_SDK})
    set(VULKAN_SDK $ENV{VULKAN_SDK})
    message(STATUS "Using Vulkan SDK from: ${VULKAN_SDK}")
    set(VULKAN_SDK_LIBS "${VULKAN_SDK}/Lib/vulkan-1.lib"
                        "${VULKAN_SDK}/Lib/VkLayer_utils.lib")
    set(Vulkan_INCLUDE_DIR "${VULKAN_SDK}/include")
    set(Vulkan_GLSLC_EXECUTABLE "${VULKAN_SDK}/bin/glslc.exe")
else()

    find_package(Vulkan)
    set (VULKAN_SDK_LIBS Vulkan::Vulkan)
    message("VULKAN INCLUDE DIR: ${Vulkan_INCLUDE_DIR} ${Vulkan_MoltenVK_LIBRARY}")
    message("VULKAN LIB PATH: ${Vulkan_LIBRARY}")
    #message(FATAL_ERROR "VULKAN_SDK environment variable is not set")
endif()

# Директории включения
target_include_directories(Vulkan3dRenderer PRIVATE
    "Vulkan3dRenderer/src"
    "Vulkan3dRenderer/vendor/glfw/include"
    "Vulkan3dRenderer/vendor/glm"
    "${Vulkan_INCLUDE_DIR}"
    "Vulkan3dRenderer/vendor/stb_image"
    "Vulkan3dRenderer/vendor/tinyobjloader"
    "Vulkan3dRenderer/vendor/spdlog/include"
)

# Определения компилятора
target_compile_definitions(Vulkan3dRenderer PRIVATE GLFW_INCLUDE_NONE)

# Настройки линковки
target_link_libraries(Vulkan3dRenderer PRIVATE
    glfw
        PRIVATE glm
        #PRIVATE glm::glm
        ${Vulkan_LIBRARY}
        ${Vulkan_MoltenVK_LIBRARY}
        #${VULKAN_SDK_LIBS}
)

# Дополнительные настройки для Windows
if(WIN32)
    target_link_libraries(Vulkan3dRenderer PRIVATE "${VULKAN_SDK}/Lib/shaderc_shared.lib")
endif()

# Конфигурация Debug
target_compile_definitions(Vulkan3dRenderer PRIVATE $<$<CONFIG:Debug>:DEBUG_ALLOCATOR>)

# Конфигурация Release: добавляем флаг оптимизации
if(MSVC)
    target_compile_options(Vulkan3dRenderer PRIVATE $<$<CONFIG:Release>:/O2>)
else()
    target_compile_options(Vulkan3dRenderer PRIVATE $<$<CONFIG:Release>:-O2>)
endif()

message("OUTPUTDIR: ${OUTPUT_DIR}")

# Компиляция шейдеров (раскомментируйте при необходимости)
 #file(GLOB_RECURSE SHADERS "Vulkan3dRenderer/shaders/*")
 #add_custom_target(CompileShaders ALL
 #        POST_BUILD
 #    COMMAND ${Vulkan_GLSLC_EXECUTABLE} -V -o ${CMAKE_BINARY_DIR}/shaders/vert.spv Vulkan3dRenderer/shaders/shader.vert
 #    DEPENDS ${SHADERS}
 #    COMMENT "Compiling shaders..."
 #)

# Создаем отдельную цель для компиляции шейдеров
set(SHADER_SOURCE_DIR "Vulkan3dRenderer/shaders")
set(SHADER_OUTPUT_DIR "${CMAKE_BINARY_DIR}/compiled_shaders")

# Создаем выходную директорию
file(MAKE_DIRECTORY ${SHADER_OUTPUT_DIR})

# Ищем все файлы шейдеров
file(GLOB_RECURSE SHADER_FILES
        "${SHADER_SOURCE_DIR}/*.vert"
        "${SHADER_SOURCE_DIR}/*.frag"
        "${SHADER_SOURCE_DIR}/*.comp"
        "${SHADER_SOURCE_DIR}/*.tesc"
        "${SHADER_SOURCE_DIR}/*.tese"
        "${SHADER_SOURCE_DIR}/*.geom"
)

set(COMPILED_SHADERS "")

foreach(SHADER_FILE ${SHADER_FILES})
    get_filename_component(SHADER_NAME ${SHADER_FILE} NAME)
    set(OUTPUT_FILE "${SHADER_OUTPUT_DIR}/${SHADER_NAME}.spv")

    # Команда компиляции с комментарием
    add_custom_command(
            OUTPUT ${OUTPUT_FILE}
            COMMAND ${Vulkan_GLSLC_EXECUTABLE} -o ${OUTPUT_FILE} ${SHADER_FILE}
            DEPENDS ${SHADER_FILE}
            COMMENT "Компилирую шейдер: ${SHADER_NAME}"
            VERBATIM
    )

    list(APPEND COMPILED_SHADERS ${OUTPUT_FILE})
endforeach()

# Создаем цель для компиляции всех шейдеров
add_custom_target(CompileAllShaders ALL DEPENDS ${COMPILED_SHADERS})

# Копируем шейдеры после сборки основного проекта
add_custom_command(
        TARGET Vulkan3dRenderer
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:Vulkan3dRenderer>/shaders/"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${SHADER_OUTPUT_DIR}"
        "$<TARGET_FILE_DIR:Vulkan3dRenderer>/shaders/"
        COMMENT "Копирую скомпилированные шейдеры в финальную папку приложения"
        VERBATIM
)

# Убеждаемся, что шейдеры компилируются перед основным приложением
add_dependencies(Vulkan3dRenderer CompileAllShaders)

