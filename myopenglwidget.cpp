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
        // Positions
        -0.5f, -0.5f, -0.5f, // Vertex 0
         0.5f, -0.5f, -0.5f, // Vertex 1
         0.5f,  0.5f, -0.5f, // Vertex 2
        -0.5f,  0.5f, -0.5f, // Vertex 3
        -0.5f, -0.5f,  0.5f, // Vertex 4
         0.5f, -0.5f,  0.5f, // Vertex 5
         0.5f,  0.5f,  0.5f, // Vertex 6
        -0.5f,  0.5f,  0.5f  // Vertex 7
    };

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
        uniform mat4 view;
        uniform mat4 projection;
        void main() {
            gl_Position = projection * view * vec4(position, 1.0);
        }
    )"); // vec3 для 3d

    if (!vertexShader->log().isEmpty()) {
        qDebug() << "Vertex Shader Error:" << vertexShader->log();
    }

    // Create and compile the fragment shader
    QOpenGLShader *fragmentShader = new QOpenGLShader(QOpenGLShader::Fragment);
    fragmentShader->compileSourceCode(R"(
        #version 330 core
        out vec4 color;
        void main() {
            color = vec4(1.0, 0.5, 0.0, 1.0); // Red color
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
        glm::vec3(0.0f, 0.0f, 3.0f), // Camera position
        glm::vec3(0.0f, 0.0f, 0.0f), // Look at the origin
        glm::vec3(1.0f, 1.0f, 0.0f)  // Up vector
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
    // Enable the vertex attribute array for the position
    shaderProgram->enableAttributeArray(vertexLocation);
    // Specify the format of the vertex data
    shaderProgram->setAttributeBuffer(vertexLocation, GL_FLOAT, 0, 3);

    // Define the indices for the cube's faces
    GLuint indices[] = {
        // Front face
        0, 1, 2, 2, 3, 0,
        // Back face
        4, 5, 6, 6, 7, 4,
        // Left face
        0, 3, 7, 7, 4, 0,
        // Right face
        1, 5, 6, 6, 2, 1,
        // Top face
        3, 2, 6, 6, 7, 3,
        // Bottom face
        0, 1, 5, 5, 4, 0
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
    vertexBuffer.release(); // Release the vertex buffer
    shaderProgram->release(); // Release the shader program

}
