#ifndef MYOPENGLWIDGET_H
#define MYOPENGLWIDGET_H

#include <QObject>

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class MyOpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    MyOpenGLWidget(QWidget *parent = nullptr);
    ~MyOpenGLWidget();
    QString mat4ToString(const glm::mat4& matrix);


protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void timerEvent(QTimerEvent *event) override;

private:
    QOpenGLShaderProgram *shaderProgram;
    QOpenGLBuffer vertexBuffer;
    QOpenGLBuffer colorBuffer;

    glm::mat4 view;
    glm::mat4 projection;

    float rotationX;
    float rotationY;
    float rotationZ;

    float alphaValue;
    int indexAlfa;
};

#endif // MYOPENGLWIDGET_H
