#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QThread>
#include <QDebug>
#include <QPainter>
#include <QThread>
#include <QPair>
#include <QVector>
#include <QTimer>
#include <math.h>

//my
#include <QKeyEvent>
#include <QPoint>
//my


#define REAL_SIZE_X_MM ((int)500)
#define REAL_SIZE_Y_MM ((int)300)
#define REAL_SIZE_Z_MM ((int)100)

#define LEN_ON_REVOLUTION_MM   ((int)2)
#define STEPS_ON_REVOLUTION_MM ((int)200)
#define STEP_SIZE_MM ((double) LEN_ON_REVOLUTION_MM / STEPS_ON_REVOLUTION_MM)

#define SIZE_X_STEPS (int(REAL_SIZE_X_MM / LEN_ON_REVOLUTION_MM * STEPS_ON_REVOLUTION_MM))
#define SIZE_Y_STEPS (int(REAL_SIZE_Y_MM / LEN_ON_REVOLUTION_MM * STEPS_ON_REVOLUTION_MM))
#define SIZE_Z_STEPS (int(REAL_SIZE_Z_MM / LEN_ON_REVOLUTION_MM * STEPS_ON_REVOLUTION_MM))

#define SHEET_SIZE_X ((int)400)//размеры рамки с осями
#define SHEET_SIZE_Y ((int)270)

#define SHEET_POS_X ((int)40)
#define SHEET_POS_Y ((int)340)


#define DEVIDER_COOF_X ((double) SHEET_SIZE_X / REAL_SIZE_X_MM)
#define DEVIDER_COOF_Y ((double) SHEET_SIZE_Y / REAL_SIZE_Y_MM)



#define DEBUG_MK 1


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    typedef enum
    {
      motX,
      motY,
      motZ
    }motors;

    void serialInit();
    QSerialPort *SerialPort;
    QString fromPort;

    int motPosX;
    int motPosY;
    int motPosZ;
    int motPosR;

    bool steppingProcess;

    QVector<QPair<int, int>> points;
    QVector<QPair<int, int>> points_figures;
    int pointsIndex;


//my
    QPoint mouse_pos;
//my


    static unsigned char checkSum(const char *str);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void goMotX(int steps);
    void goMotY(int steps);
    void goMotZ(int steps);
    void goToX(int pos);
    void goToY(int pos);
    void goToZ(int pos);
    void goToXY2(int x, int y);
    void drawCircle(int x, int y, int a, int b, QVector<QPair<int, int> > *points_circle);
    void drawRect(int x, int y, int a, int b, QVector<QPair<int, int>> *points_r);
    void drawLine(int x1, int y1, int x2, int y2, QVector<QPair<int, int>> points_l);
    void testMegaParser(QString str);
    void testParser(QString data);



    QString getToMotStr(int mot, int steps);
    void coordinats(long x1, long y1, long x2, long y2);
private:
    Ui::MainWindow *ui;
    void paintEvent(QPaintEvent *);

public slots:
    void SerialPortRead();
    void goToHome();
private slots:
    void on_pushButton_4_clicked();
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_7_clicked();
    void on_pushButton_6_clicked();
    void on_pushButton_5_clicked();
    void on_pushButton_8_clicked();
    void on_btnDrawCircle_pressed();
    void on_btnDrawRect_pressed();
    void on_btnDrawLine_pressed();
    void on_pushButton_10_clicked();
    void on_pushButton_11_clicked();
};
#endif // MAINWINDOW_H
