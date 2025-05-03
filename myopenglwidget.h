#ifndef MYOPENGLWIDGET_H
#define MYOPENGLWIDGET_H

#include <QObject>

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>

class MyOpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    MyOpenGLWidget(QWidget *parent = nullptr);
    ~MyOpenGLWidget();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    QOpenGLShaderProgram *shaderProgram;
    QOpenGLBuffer vertexBuffer;
};

#endif // MYOPENGLWIDGET_H
