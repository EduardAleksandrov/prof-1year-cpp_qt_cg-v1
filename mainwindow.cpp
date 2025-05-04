#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    openglWidget = new MyOpenGLWidget(this);
    setCentralWidget(openglWidget);

    this->statusBar()->hide(); // нижняя белая планка
}

MainWindow::~MainWindow()
{
    delete ui;
}

