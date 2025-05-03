#include "myopenglwidget.h"
#include <QOpenGLShader>
#include <QDebug>

MyOpenGLWidget::MyOpenGLWidget(QWidget *parent)
    : QOpenGLWidget(parent), vertexBuffer(QOpenGLBuffer::VertexBuffer)
{
}

MyOpenGLWidget::~MyOpenGLWidget()
{
    makeCurrent();
    vertexBuffer.destroy();
    delete shaderProgram;
}

void MyOpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();

    glClearColor(0.1f, 0.1f, 0.4f, 1.0f);

    // Define the vertices of the triangle
    GLfloat vertices[] = {
        // Позиции          // Цвета
        // Передняя грань
         0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, // Vertex 0 (красный)
        -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, // Vertex 1 (зеленый)
        -0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // Vertex 2 (синий)
         0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f, // Vertex 3 (желтый)

        // Задняя грань
         0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 1.0f, // Vertex 4 (пурпурный)
        -0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 1.0f, // Vertex 5 (циановый)
        -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, // Vertex 6 (белый)
         0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f  // Vertex 7 (серый)

        // Вы можете добавить дополнительные цвета для других граней, если хотите
    };
//    GLfloat vertices[] = {
//        // Позиции          // Цвета
//        -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, // Vertex 0 (красный)
//         0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, // Vertex 1 (зеленый)
//         0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // Vertex 2 (синий)
//        -0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f, // Vertex 3 (желтый)
//        -0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 1.0f, // Vertex 4 (пурпурный)
//         0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 1.0f, // Vertex 5 (циановый)
//         0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, // Vertex 6 (белый)
//        -0.5f,  0.5f,  0.5f, 0.5f, 0.5f, 0.5f  // Vertex 7 (серый)
//    };

    // Create and bind the vertex buffer
    // Create and bind the vertex buffer
    if (!vertexBuffer.create()) {
        qDebug() << "Failed to create vertex buffer.";
        return;
    }
    vertexBuffer.bind();
    vertexBuffer.allocate(vertices, sizeof(vertices));

    // Create and compile the vertex shader
    QOpenGLShader *vertexShader = new QOpenGLShader(QOpenGLShader::Vertex);
    vertexShader->compileSourceCode(R"(
        #version 330 core
        layout(location = 0) in vec3 position;
        layout(location = 1) in vec3 color; // Цвет
        out vec3 fragColor; // Передаем цвет во фрагментный шейдер
        uniform mat4 view;
        uniform mat4 projection;
        void main() {
            gl_Position = projection * view * vec4(position, 1.0);
            fragColor = color; // Передаем цвет
        }
    )"); // vec3 для 3d

    if (!vertexShader->log().isEmpty()) {
        qDebug() << "Vertex Shader Error:" << vertexShader->log();
    }

    // Create and compile the fragment shader
    QOpenGLShader *fragmentShader = new QOpenGLShader(QOpenGLShader::Fragment);
    fragmentShader->compileSourceCode(R"(
        #version 330 core
        in vec3 fragColor; // Получаем цвет из вершинного шейдера
        out vec4 color;
        void main() {
            color = vec4(fragColor, 1.0); // Устанавливаем цвет
        }
    )");

    if (!fragmentShader->log().isEmpty()) {
        qDebug() << "Fragment Shader Error:" << fragmentShader->log();
    }

    // Create the shader program and link the shaders
    shaderProgram = new QOpenGLShaderProgram();
    shaderProgram->addShader(vertexShader);
    shaderProgram->addShader(fragmentShader);
    shaderProgram->link();

    // Set up the view matrix
    view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 4.0f), // Camera position
        glm::vec3(0.0f, 0.0f, 0.0f), // Look at the origin
        glm::vec3(0.0f, 1.0f, 0.0f)  // Up vector
    );

    // Set up the projection matrix
    projection = glm::perspective(
        glm::radians(45.0f), // Field of view in degrees
        (float)width() / (float)height(), // Aspect ratio
        0.1f, // Near plane
        100.0f // Far plane
    );

    if (!shaderProgram->log().isEmpty()) {
        qDebug() << "Shader Program Error:" << shaderProgram->log();
    }

    qDebug() << "Vertex Buffer ID:" << vertexBuffer.bufferId();
    qDebug() << "OpenGL Version:" << (const char*)glGetString(GL_VERSION);
    qDebug() << "GLSL Version:" << (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

    // Clean up
    vertexShader->deleteLater();
    fragmentShader->deleteLater();
}

void MyOpenGLWidget::resizeGL(int w, int h)
{
    if (h == 0) h = 1; // Prevent division by zero
    glViewport(0, 0, w, h); // Set the viewport to cover the new window

    // Update the projection matrix on resize
    projection = glm::perspective(
        glm::radians(45.0f), // Field of view in degrees
        (float)w / (float)h, // Aspect ratio
        0.1f, // Near plane
        100.0f // Far plane
    );
}

void MyOpenGLWidget::paintGL()
{
    qDebug() << "launch";
    glClear(GL_COLOR_BUFFER_BIT); // Clear the color buffer
    if (shaderProgram) {
        shaderProgram->bind(); // Bind the shader program

        // Pass the view matrix to the shader
        int viewLoc = shaderProgram->uniformLocation("view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        // Pass the projection matrix to the shader
        int projLoc = shaderProgram->uniformLocation("projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    } else {
        qDebug() << "Shader program is not valid!";
        return;
    }
    vertexBuffer.bind();   // Bind the vertex buffer

    int vertexLocation = shaderProgram->attributeLocation("position");
    int colorLocation = shaderProgram->attributeLocation("color"); // Получаем индекс для цвета

    // Включаем массив атрибутов для позиции
    shaderProgram->enableAttributeArray(vertexLocation);
    shaderProgram->setAttributeBuffer(vertexLocation, GL_FLOAT, 0, 3); // Позиции

    // Включаем массив атрибутов для цвета
    shaderProgram->enableAttributeArray(colorLocation);
    shaderProgram->setAttributeBuffer(colorLocation, GL_FLOAT, sizeof(GLfloat) * 3, 3); // Цвета

    // Define the indices for the cube's faces
    GLuint indices[] = {
        1, 0, 3, //Front face
        3, 2, 1, //Front face
//        5, 4, 7, //Back face
//        7, 6, 5, //Back face
        1, 2, 6, //Left face
        6, 5, 1, //Left face
//        0, 3, 7, //Right face
//        7, 4, 0, //Right face
//        1, 5, 4, //Top face
//        4, 0, 1, //Top face
//        2, 6, 7, //Bottom face
//        7, 3, 2  //Bottom face
    };

    // Create an index buffer
    QOpenGLBuffer indexBuffer(QOpenGLBuffer::IndexBuffer);
    if (!indexBuffer.create()) {
        qDebug() << "Failed to create index buffer.";
        return;
    }
    indexBuffer.bind();
    indexBuffer.allocate(indices, sizeof(indices));

    // Draw the cube using the index buffer
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    // Clean up
    indexBuffer.release();
    shaderProgram->disableAttributeArray(vertexLocation);
    shaderProgram->disableAttributeArray(colorLocation);
    vertexBuffer.release(); // Release the vertex buffer
    shaderProgram->release(); // Release the shader program

}
