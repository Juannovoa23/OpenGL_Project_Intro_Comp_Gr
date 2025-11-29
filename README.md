# Explorador de Formas 3D – OpenGL

Proyecto en C++/OpenGL 3.3 que implementa un **visor interactivo** capaz de mostrar y transformar en tiempo real:

- Cubo  
- Esfera  
- Pirámide  
- Toro  

Incluye iluminación Phong, materiales intercambiables, rotación, escalado y cambio dinámico de color.

---

## 🚀 Características principales

- Renderizado moderno con **OpenGL Core 3.3**
- **GLFW** para la ventana y entrada de usuario
- **GLAD** para cargar funciones OpenGL
- **GLM** para matrices y vectores
- Geometría generada manualmente (sin modelos externos)
- Iluminación Phong (ambient + diffuse + specular)
- Transformaciones en tiempo real

---

## 🎮 Controles

| Tecla | Acción |
|------|---------|
| 1 | Cubo |
| 2 | Esfera |
| 3 | Pirámide |
| 4 | Toro |
| W / S | Rotar en eje X |
| A / D | Rotar en eje Y |
| Q / E | Rotar en eje Z |
| + / - | Escalar |
| C | Cambiar color |
| M | Cambiar material |
| R | Reset |

---

## 📁 Estructura del Proyecto

/src
main.cpp
Shader.cpp
Shader.h
/shaders
vertex_shader.glsl
fragment_shader.glsl
/lib
glad/
glfw/
glm/
/build
CMakeLists.txt
README.md

yaml
Copiar código

---

## 🔧 Compilación

En consola:

```bash
cmake -S . -B build -A x64
cmake --build build --config Debug
Ejecutar:

bash
Copiar código
./build/Debug/opengltriangle.exe
📜 Licencia
Este proyecto es educativo y puede ser reutilizado libremente citando al autor.

👨‍💻 Autor
juan sebastian novoa garcia
