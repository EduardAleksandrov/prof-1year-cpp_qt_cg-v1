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
        0.0f,  0.5f, 0.0f,  // Vertex 1 (X, Y) Bottom left
       -0.5f, -0.5f, 0.0f,  // Vertex 2 (X, Y) Bottom right
        0.5f, -0.5f, 0.0f   // Vertex 3 (X, Y) Top
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
        layout(location = 0) in vec2 position;
        void main() {
            gl_Position = vec4(position, 0.0, 1.0);
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
}

void MyOpenGLWidget::paintGL()
{
    qDebug() << "launch";
    glClear(GL_COLOR_BUFFER_BIT); // Clear the color buffer
    if (shaderProgram) {
        shaderProgram->bind(); // Bind the shader program
    } else {
        qDebug() << "Shader program is not valid!";
        return;
    }
    vertexBuffer.bind();   // Bind the vertex buffer

    int vertexLocation = shaderProgram->attributeLocation("position");
    // Enable the vertex attribute array for the position
    shaderProgram->enableAttributeArray(vertexLocation);
    // Specify the format of the vertex data
    shaderProgram->setAttributeBuffer(vertexLocation, GL_FLOAT, 0, 2);

    // Draw the triangle
    glDrawArrays(GL_TRIANGLES, 0, 3);

    shaderProgram->disableAttributeArray(vertexLocation);
    vertexBuffer.release(); // Release the vertex buffer
    shaderProgram->release(); // Release the shader program

}
