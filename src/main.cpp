//src/main.cpp
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <sstream>
#include "Shader.h"


// ---------------------------------------------------
// Estructura para una forma (VAO + número de vértices)
// ---------------------------------------------------
struct Shape {
    GLuint VAO;
    GLsizei vertexCount;
};

// Variables globales de transformación
float rotX = 0.0f;
float rotY = 0.0f;
float rotZ = 0.0f;
float scaleFactor = 1.0f;

// Índice de forma actual: 0 = cubo, 1 = esfera, 2 = pirámide, 3 = toro
int currentShapeIndex = 0;

// Colores predefinidos
std::vector<glm::vec3> colors = {
    glm::vec3(1.0f, 1.0f, 1.0f),
    glm::vec3(1.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 1.0f),
    glm::vec3(0.0f, 1.0f, 0.0f),
    glm::vec3(1.0f, 1.0f, 0.0f)
};
int currentColorIndex = 0;

// Materiales simples (specular + shininess)
struct Material {
    glm::vec3 specular;
    float shininess;
};

std::vector<Material> materials = {
    { glm::vec3(0.3f), 8.0f  }, // mate
    { glm::vec3(0.7f), 32.0f }, // brillante
    { glm::vec3(1.0f), 64.0f }, // metálico
    { glm::vec3(0.5f), 4.0f  }  // más suave
};
int currentMaterialIndex = 0;

// Prototipos
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);


// Utilitarios
std::string loadTextFile(const std::string& path);

Shape createShapeFromVertices(const std::vector<float>& data);
Shape createCube();
Shape createPyramid();
Shape createSphere(int sectorCount, int stackCount);
Shape createTorus(int numMajor, int numMinor, float majorRadius, float minorRadius);

int main()
{
    // -------------------------------------------
    // 1. Inicialización de GLFW
    // -------------------------------------------
    if (!glfwInit())
    {
        std::cerr << "Error: no se pudo inicializar GLFW\n";
        return -1;
    }

    // Configurar versión de OpenGL: 3.3 Core
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Crear ventana
    GLFWwindow* window = glfwCreateWindow(800, 600, "Explorador de Formas - OpenGL", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Error: no se pudo crear la ventana\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // -------------------------------------------
    // 2. Inicializar GLAD
    // -------------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Error: no se pudo inicializar GLAD\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    // -------------------------------------------
    
    // -------------------------------------------
    // 3. Crear programa de shaders usando la clase Shader
    std::string vsCode = loadTextFile("src/shaders/vertex_shader.glsl");
    std::string fsCode = loadTextFile("src/shaders/fragment_shader.glsl");



    Shader shader(vsCode, fsCode);
    GLuint shaderProgram = shader.GetId();

    if (shaderProgram == 0)
    {
        std::cerr << "Error: no se pudo crear el programa de shaders\n";
        return -1;
    }

    // -------------------------------------------
    // 4. Crear las formas (cubo, esfera, pirámide, toro)
    // -------------------------------------------
    Shape cube = createCube();
    Shape sphere = createSphere(24, 24);           // resolución baja, minimalista
    Shape pyramid = createPyramid();
    Shape torus = createTorus(32, 16, 1.0f, 0.3f);

    std::vector<Shape> shapes = { cube, sphere, pyramid, torus };

    // -------------------------------------------
    // 5. Configuración de la luz y cámara
    // -------------------------------------------
    glm::vec3 lightPos(2.0f, 2.0f, 2.0f);

    // Bucle principal
    while (!glfwWindowShouldClose(window))
    {
        // 5.1 Entrada
        processInput(window);

        // 5.2 Limpiar buffers
        glClearColor(0.07f, 0.07f, 0.09f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 5.3 Activar programa de shaders
        shader.Bind();
        GLuint shaderProgram = shader.GetId();

        // 5.4 Matrices de cámara y proyección
        glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 projection = glm::perspective(glm::radians(45.0f),
            800.0f / 600.0f,
            0.1f,
            100.0f);

        // 5.5 Matriz modelo (transformaciones de la figura)
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(scaleFactor));
        model = glm::rotate(model, glm::radians(rotX), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotZ), glm::vec3(0.0f, 0.0f, 1.0f));

        // 5.6 Enviar matrices a los shaders
        GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // 5.7 Parámetros de la luz
        glUniform3fv(glGetUniformLocation(shaderProgram, "lightPos"), 1, glm::value_ptr(lightPos));
        glm::vec3 cameraPos(0.0f, 0.0f, 5.0f);
        glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(cameraPos));

        // Intensidad básica de luz
        glm::vec3 lightAmbient(0.2f);
        glm::vec3 lightDiffuse(0.7f);
        glm::vec3 lightSpecular(1.0f);
        glUniform3fv(glGetUniformLocation(shaderProgram, "lightAmbient"), 1, glm::value_ptr(lightAmbient));
        glUniform3fv(glGetUniformLocation(shaderProgram, "lightDiffuse"), 1, glm::value_ptr(lightDiffuse));
        glUniform3fv(glGetUniformLocation(shaderProgram, "lightSpecular"), 1, glm::value_ptr(lightSpecular));

        // 5.8 Color y material actuales
        glm::vec3 objectColor = colors[currentColorIndex];
        Material mat = materials[currentMaterialIndex];

        glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, glm::value_ptr(objectColor));
        glUniform3fv(glGetUniformLocation(shaderProgram, "materialSpecular"), 1, glm::value_ptr(mat.specular));
        glUniform1f(glGetUniformLocation(shaderProgram, "materialShininess"), mat.shininess);

        // 5.9 Dibujar la forma actual
        Shape currentShape = shapes[currentShapeIndex];
        glBindVertexArray(currentShape.VAO);
        glDrawArrays(GL_TRIANGLES, 0, currentShape.vertexCount);
        glBindVertexArray(0);

        // Intercambiar buffers y procesar eventos
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Limpieza
    for (auto& s : shapes) {
        glDeleteVertexArrays(1, &s.VAO);
    }
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}

// ---------------------------------------------------
// Callbacks y entrada de teclado
// ---------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Cambiar de forma: 1 = cubo, 2 = esfera, 3 = pirámide, 4 = toro
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) currentShapeIndex = 0;
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) currentShapeIndex = 1;
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) currentShapeIndex = 2;
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) currentShapeIndex = 3;

    // Rotación
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) rotX += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) rotX -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) rotY += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) rotY -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) rotZ += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) rotZ -= 1.0f;

    // Escala
    if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS)
        scaleFactor += 0.01f;
    if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS)
        scaleFactor -= 0.01f;

    if (scaleFactor < 0.1f) scaleFactor = 0.1f;
    if (scaleFactor > 3.0f) scaleFactor = 3.0f;

    // Cambiar color
    static bool cPressed = false;
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && !cPressed) {
        cPressed = true;
        currentColorIndex = (currentColorIndex + 1) % colors.size();
    }
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE) {
        cPressed = false;
    }

    // Cambiar material
    static bool mPressed = false;
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS && !mPressed) {
        mPressed = true;
        currentMaterialIndex = (currentMaterialIndex + 1) % materials.size();
    }
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_RELEASE) {
        mPressed = false;
    }

    // Reset
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        rotX = rotY = rotZ = 0.0f;
        scaleFactor = 1.0f;
        currentColorIndex = 0;
        currentMaterialIndex = 0;
    }
}

// ---------------------------------------------------
// Utilitarios de shaders
// ---------------------------------------------------
std::string loadTextFile(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "No se pudo abrir el archivo: " << path << "\n";
        return "";
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}



// ---------------------------------------------------
// Crear VAO a partir de un vector de vértices
// Formato: [posx, posy, posz, nx, ny, nz, ...]
// ---------------------------------------------------
Shape createShapeFromVertices(const std::vector<float>& data)
{
    Shape s{};
    s.vertexCount = static_cast<GLsizei>(data.size() / 6); // 3 pos + 3 normal

    GLuint VBO;
    glGenVertexArrays(1, &s.VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(s.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);

    // Posiciones
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normales
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return s;
}

// ---------------------------------------------------
// Cubo (centrado en el origen)
// ---------------------------------------------------
Shape createCube()
{
    // 36 vértices (12 triángulos), cada uno pos + normal
    float vertices[] = {
        // Posición          // Normal
        // Cara frontal
        -1.0f, -1.0f,  1.0f,   0.0f,  0.0f,  1.0f,
         1.0f, -1.0f,  1.0f,   0.0f,  0.0f,  1.0f,
         1.0f,  1.0f,  1.0f,   0.0f,  0.0f,  1.0f,

         1.0f,  1.0f,  1.0f,   0.0f,  0.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,   0.0f,  0.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,   0.0f,  0.0f,  1.0f,

        // Cara trasera
        -1.0f, -1.0f, -1.0f,   0.0f,  0.0f, -1.0f,
         1.0f,  1.0f, -1.0f,   0.0f,  0.0f, -1.0f,
         1.0f, -1.0f, -1.0f,   0.0f,  0.0f, -1.0f,

         1.0f,  1.0f, -1.0f,   0.0f,  0.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,   0.0f,  0.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,   0.0f,  0.0f, -1.0f,

        // Cara izquierda
        -1.0f,  1.0f,  1.0f,  -1.0f,  0.0f,  0.0f,
        -1.0f,  1.0f, -1.0f,  -1.0f,  0.0f,  0.0f,
        -1.0f, -1.0f, -1.0f,  -1.0f,  0.0f,  0.0f,

        -1.0f, -1.0f, -1.0f,  -1.0f,  0.0f,  0.0f,
        -1.0f, -1.0f,  1.0f,  -1.0f,  0.0f,  0.0f,
        -1.0f,  1.0f,  1.0f,  -1.0f,  0.0f,  0.0f,

        // Cara derecha
         1.0f,  1.0f,  1.0f,   1.0f,  0.0f,  0.0f,
         1.0f, -1.0f, -1.0f,   1.0f,  0.0f,  0.0f,
         1.0f,  1.0f, -1.0f,   1.0f,  0.0f,  0.0f,

         1.0f, -1.0f, -1.0f,   1.0f,  0.0f,  0.0f,
         1.0f,  1.0f,  1.0f,   1.0f,  0.0f,  0.0f,
         1.0f, -1.0f,  1.0f,   1.0f,  0.0f,  0.0f,

         // Cara superior
         -1.0f,  1.0f, -1.0f,   0.0f,  1.0f,  0.0f,
          1.0f,  1.0f,  1.0f,   0.0f,  1.0f,  0.0f,
          1.0f,  1.0f, -1.0f,   0.0f,  1.0f,  0.0f,

          1.0f,  1.0f,  1.0f,   0.0f,  1.0f,  0.0f,
         -1.0f,  1.0f, -1.0f,   0.0f,  1.0f,  0.0f,
         -1.0f,  1.0f,  1.0f,   0.0f,  1.0f,  0.0f,

         // Cara inferior
         -1.0f, -1.0f, -1.0f,   0.0f, -1.0f,  0.0f,
          1.0f, -1.0f, -1.0f,   0.0f, -1.0f,  0.0f,
          1.0f, -1.0f,  1.0f,   0.0f, -1.0f,  0.0f,

          1.0f, -1.0f,  1.0f,   0.0f, -1.0f,  0.0f,
         -1.0f, -1.0f,  1.0f,   0.0f, -1.0f,  0.0f,
         -1.0f, -1.0f, -1.0f,   0.0f, -1.0f,  0.0f
    };

    std::vector<float> data(vertices, vertices + sizeof(vertices) / sizeof(float));
    return createShapeFromVertices(data);
}

// ---------------------------------------------------
// Pirámide (base cuadrada)
// ---------------------------------------------------
Shape createPyramid()
{
    // Pirámide centrada, altura 2, base de 2x2
    std::vector<float> data;

    glm::vec3 top(0.0f, 1.0f, 0.0f);
    glm::vec3 bl(-1.0f, -1.0f, 1.0f); // bottom-left front
    glm::vec3 br(1.0f, -1.0f, 1.0f); // bottom-right front
    glm::vec3 brb(1.0f, -1.0f, -1.0f); // bottom-right back
    glm::vec3 blb(-1.0f, -1.0f, -1.0f); // bottom-left back

    auto addTriangle = [&](glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) {
        glm::vec3 u = p2 - p1;
        glm::vec3 v = p3 - p1;
        glm::vec3 n = glm::normalize(glm::cross(u, v));
        // p1
        data.push_back(p1.x); data.push_back(p1.y); data.push_back(p1.z);
        data.push_back(n.x);  data.push_back(n.y);  data.push_back(n.z);
        // p2
        data.push_back(p2.x); data.push_back(p2.y); data.push_back(p2.z);
        data.push_back(n.x);  data.push_back(n.y);  data.push_back(n.z);
        // p3
        data.push_back(p3.x); data.push_back(p3.y); data.push_back(p3.z);
        data.push_back(n.x);  data.push_back(n.y);  data.push_back(n.z);
        };

    // Lados
    addTriangle(top, bl, br);    // frente
    addTriangle(top, br, brb);   // derecha
    addTriangle(top, brb, blb);  // atrás
    addTriangle(top, blb, bl);   // izquierda

    // Base (dos triángulos)
    glm::vec3 nBase(0.0f, -1.0f, 0.0f);

    auto addBaseTri = [&](glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) {
        data.push_back(p1.x); data.push_back(p1.y); data.push_back(p1.z);
        data.push_back(nBase.x); data.push_back(nBase.y); data.push_back(nBase.z);

        data.push_back(p2.x); data.push_back(p2.y); data.push_back(p2.z);
        data.push_back(nBase.x); data.push_back(nBase.y); data.push_back(nBase.z);

        data.push_back(p3.x); data.push_back(p3.y); data.push_back(p3.z);
        data.push_back(nBase.x); data.push_back(nBase.y); data.push_back(nBase.z);
        };

    addBaseTri(bl, br, brb);
    addBaseTri(brb, blb, bl);

    return createShapeFromVertices(data);
}

// ---------------------------------------------------
// Esfera generada por sectores y stacks
// ---------------------------------------------------
Shape createSphere(int sectorCount, int stackCount)
{
    std::vector<float> data;

    float radius = 1.0f;
    float pi = 3.14159265f;
    float twoPi = 2.0f * pi;

    for (int i = 0; i < stackCount; ++i)
    {
        float stackAngle1 = pi / 2 - (float)i * (pi / stackCount);
        float stackAngle2 = pi / 2 - (float)(i + 1) * (pi / stackCount);

        float y1 = radius * sinf(stackAngle1);
        float r1 = radius * cosf(stackAngle1);

        float y2 = radius * sinf(stackAngle2);
        float r2 = radius * cosf(stackAngle2);

        for (int j = 0; j < sectorCount; ++j)
        {
            float sectorAngle1 = j * (twoPi / sectorCount);
            float sectorAngle2 = (j + 1) * (twoPi / sectorCount);

            float x1 = r1 * cosf(sectorAngle1);
            float z1 = r1 * sinf(sectorAngle1);

            float x2 = r1 * cosf(sectorAngle2);
            float z2 = r1 * sinf(sectorAngle2);

            float x3 = r2 * cosf(sectorAngle1);
            float z3 = r2 * sinf(sectorAngle1);

            float x4 = r2 * cosf(sectorAngle2);
            float z4 = r2 * sinf(sectorAngle2);

            glm::vec3 p1(x1, y1, z1);
            glm::vec3 p2(x2, y1, z2);
            glm::vec3 p3(x3, y2, z3);
            glm::vec3 p4(x4, y2, z4);

            auto addVertex = [&](glm::vec3 p) {
                glm::vec3 n = glm::normalize(p);
                data.push_back(p.x); data.push_back(p.y); data.push_back(p.z);
                data.push_back(n.x); data.push_back(n.y); data.push_back(n.z);
                };

            // triángulo 1
            addVertex(p1);
            addVertex(p2);
            addVertex(p3);

            // triángulo 2
            addVertex(p2);
            addVertex(p4);
            addVertex(p3);
        }
    }

    return createShapeFromVertices(data);
}

// ---------------------------------------------------
// Toro (donut) paramétrico
// ---------------------------------------------------
Shape createTorus(int numMajor, int numMinor, float majorRadius, float minorRadius)
{
    std::vector<float> data;

    float twoPi = 2.0f * 3.14159265f;

    for (int i = 0; i < numMajor; ++i)
    {
        float a0 = i * twoPi / numMajor;
        float a1 = (i + 1) * twoPi / numMajor;

        float x0 = cosf(a0);
        float y0 = sinf(a0);
        float x1 = cosf(a1);
        float y1 = sinf(a1);

        for (int j = 0; j < numMinor; ++j)
        {
            float b0 = j * twoPi / numMinor;
            float b1 = (j + 1) * twoPi / numMinor;

            float c0 = cosf(b0);
            float r0 = minorRadius * c0 + majorRadius;
            float z0 = minorRadius * sinf(b0);

            float c1 = cosf(b1);
            float r1 = minorRadius * c1 + majorRadius;
            float z1 = minorRadius * sinf(b1);

            glm::vec3 p1(r0 * x0, r0 * y0, z0);
            glm::vec3 p2(r0 * x1, r0 * y1, z0);
            glm::vec3 p3(r1 * x0, r1 * y0, z1);
            glm::vec3 p4(r1 * x1, r1 * y1, z1);

            auto addVertex = [&](glm::vec3 p, glm::vec3 centerRing) {
                glm::vec3 n = glm::normalize(p - centerRing);
                data.push_back(p.x); data.push_back(p.y); data.push_back(p.z);
                data.push_back(n.x); data.push_back(n.y); data.push_back(n.z);
                };

            glm::vec3 center1(majorRadius * x0, majorRadius * y0, 0.0f);
            glm::vec3 center2(majorRadius * x1, majorRadius * y1, 0.0f);

            // triángulo 1
            addVertex(p1, center1);
            addVertex(p2, center2);
            addVertex(p3, center1);

            // triángulo 2
            addVertex(p2, center2);
            addVertex(p4, center2);
            addVertex(p3, center1);
        }
    }

    return createShapeFromVertices(data);
}
