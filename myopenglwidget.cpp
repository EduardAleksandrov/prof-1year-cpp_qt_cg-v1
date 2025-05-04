#include "myopenglwidget.h"
#include <QOpenGLShader>
#include <QDebug>

MyOpenGLWidget::MyOpenGLWidget(QWidget *parent)
    : QOpenGLWidget(parent), vertexBuffer(QOpenGLBuffer::VertexBuffer), colorBuffer(QOpenGLBuffer::VertexBuffer),
      rotationX(0.0f), rotationY(0.0f), rotationZ(0.0f), alphaValue(0.0f), indexAlfa(0)
{
}

MyOpenGLWidget::~MyOpenGLWidget()
{
    makeCurrent();
    vertexBuffer.destroy();
    colorBuffer.destroy();
    delete shaderProgram;
}

void MyOpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();

    glClearColor(0.1f, 0.1f, 0.4f, 1.0f);

    // Включение глубинного тестирования для непрозрачности .. прозрачные объекты отрисовываются после непрозрачных
    glEnable(GL_DEPTH_TEST); // сначала отрисовывают дальнюю непрозрачную, а потом поверх нее прозрачную
    // Включение смешивания для работы фрагментного шейдера
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Настройка функции смешивания



    // Define the vertices of the triangle
    GLfloat vertices[] = {
        // Позиции
        // Передняя грань
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,

        // Задняя грань
         0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f
    };
    GLfloat colors[] = {
        // Цвета
        1.0f, 0.0f, 0.0f, // Vertex 0 (красный)
        0.0f, 1.0f, 0.0f, // Vertex 1 (зеленый)
        0.0f, 0.0f, 1.0f, // Vertex 2 (синий)
        1.0f, 1.0f, 0.0f, // Vertex 3 (желтый)
        1.0f, 0.0f, 1.0f, // Vertex 4 (пурпурный)
        0.0f, 1.0f, 1.0f, // Vertex 5 (циановый)
        1.0f, 0.0f, 1.0f, // Vertex 6 (белый)
        0.5f, 0.5f, 0.5f  // Vertex 7 (серый)
    };

    // Create and bind the vertex buffer
    if (!vertexBuffer.create()) {
        qDebug() << "Failed to create vertex buffer.";
        return;
    }
    vertexBuffer.bind();
    vertexBuffer.allocate(vertices, sizeof(vertices));

    // Create and bind the color buffer
    if (!colorBuffer.create()) {
        qDebug() << "Failed to create color buffer.";
        return;
    }
    colorBuffer.bind();
    colorBuffer.allocate(colors, sizeof(colors));

    // Create and compile the vertex shader  принимает координаты вершины и преобразует их в координаты экрана.
    QOpenGLShader *vertexShader = new QOpenGLShader(QOpenGLShader::Vertex);
    vertexShader->compileSourceCode(R"(
        #version 330 core
        layout(location = 0) in vec3 position;
        layout(location = 1) in vec3 color; // Цвет
        out vec3 fragColor; // Передаем цвет во фрагментный шейдер
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        void main() {
            gl_Position =  projection * view * model * vec4(position, 1.0);
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
        uniform float alpha;
        void main() {
            color = vec4(fragColor, alpha); // Устанавливаем цвет
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
        glm::vec3(0.0f, 1.0f, 3.0f), // Camera position
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

    // таймер для вращения
    startTimer(16); // Запускает таймер с интервалом 16 мс (примерно 60 FPS)

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
    // Очистка экрана и буфера глубины
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the color buffer
    if (shaderProgram) {
        shaderProgram->bind(); // Bind the shader program

        // Pass the view matrix to the shader
        int viewLoc = shaderProgram->uniformLocation("view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        // Pass the projection matrix to the shader
        int projLoc = shaderProgram->uniformLocation("projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Создание матрицы модели
        glm::mat4 model = glm::mat4(1.0f); // Инициализация единичной матрицы
        model = glm::rotate(model, glm::radians(rotationX), glm::vec3(1.0f, 0.0f, 0.0f)); // Вращение вокруг оси X
        model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f)); // Вращение вокруг оси Y
        model = glm::rotate(model, glm::radians(rotationZ), glm::vec3(0.0f, 0.0f, 1.0f)); // Вращение вокруг оси Z

        // Передача матрицы модели в шейдер
        int modelLoc = shaderProgram->uniformLocation("model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        // Установите значение альфа-канала
        float alfaToShader;
        if(alphaValue > 1.0f) alfaToShader = 1.0f;
        else alfaToShader = alphaValue;
        int alphaLoc = shaderProgram->uniformLocation("alpha");
        glUniform1f(alphaLoc, alfaToShader);

    } else {
        qDebug() << "Shader program is not valid!";
        return;
    }

    vertexBuffer.bind();   // Bind the vertex buffer

    int vertexLocation = shaderProgram->attributeLocation("position");

    // Включаем массив атрибутов для позиции
    shaderProgram->enableAttributeArray(vertexLocation);
    shaderProgram->setAttributeBuffer(vertexLocation, GL_FLOAT, 0, 3); // Позиции

    // Bind the color buffer
    colorBuffer.bind();
    int colorLocation = shaderProgram->attributeLocation("color");
    shaderProgram->enableAttributeArray(colorLocation);
    shaderProgram->setAttributeBuffer(colorLocation, GL_FLOAT, 0, 3); // Colors

    // Define the indices for the cube's faces
    GLuint indices[] = {
        1, 0, 3, //Front face
        3, 2, 1, //Front face
        5, 4, 7, //Back face
        7, 6, 5, //Back face
        1, 2, 6, //Left face
        6, 5, 1, //Left face
        0, 3, 7, //Right face
        7, 4, 0, //Right face
        1, 5, 4, //Top face
        4, 0, 1, //Top face
        2, 6, 7, //Bottom face
        7, 3, 2 //Bottom face

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
    if(alphaValue < 0.99f) glDisable(GL_DEPTH_TEST);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
//    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (void*)(30* sizeof(GLuint))); // Индексы для прозрачной грани
    glEnable(GL_DEPTH_TEST);

    // Clean up
    indexBuffer.release();
    shaderProgram->disableAttributeArray(vertexLocation);
    shaderProgram->disableAttributeArray(colorLocation);
    vertexBuffer.release(); // Release the vertex buffer
    colorBuffer.release(); // Release the color buffer
    shaderProgram->release(); // Release the shader program

}

void MyOpenGLWidget::timerEvent(QTimerEvent *event)
{
    rotationX += 1.0f; // Увеличьте угол вращения по оси X
    rotationY += 1.0f; // Увеличьте угол вращения по оси Y

    if(indexAlfa == 0)
    {
        alphaValue += 0.001f; // Увеличиваем альфа-канал
        if (alphaValue > 1.2f) indexAlfa = 1;
    } else {
        alphaValue -= 0.001f; // Увеличиваем альфа-канал
        if (alphaValue < 0.0f) indexAlfa = 0;
    }

    update(); // Перерисуйте виджет, вызывает paintGl
}
