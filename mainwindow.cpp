#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <math.h>
#include <qvector.h>

///////////////////////////////////////////////
#define RX_BUFFER_SIZE 128
char rx_buffer[RX_BUFFER_SIZE] = {0};
long int n = 0;
char ans[30] = {0};
unsigned char ansPtr = 0;
int rx_wr_index = 0;

unsigned char chek = 0;
unsigned char chekSum(char *str);
void moveX_(int steps);
void moveY_(int steps);
void moveZ_(int steps);

long int posX = 0;
long int posY = 0;
long int posZ = 0;
long int posR = 0;


//////////////////////////////////////////////


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);             // Стандартный вызов настройки GUI (по умолчанию)
    this->setMouseTracking(true);


    motPosX = 0;                   // Инициализируем позиции моторов в 0
    motPosY = 0;
    motPosZ = 0;
    motPosR = 0;



    if(DEBUG_MK == 0)
    {
     ui->pushButton->setEnabled(0);
     ui->pushButton_2->setEnabled(0);
     ui->pushButton_3->setEnabled(0);
     ui->pushButton_5->setEnabled(0);
     ui->pushButton_6->setEnabled(0);
     ui->pushButton_7->setEnabled(0);
     ui->pushButton_8->setEnabled(0);
    }

    connect(ui->pushButton_9, SIGNAL(clicked(bool)), this, SLOT(goToHome()));

    ui->lineEdit_4->setText(QString::number(motPosX));  // Выводим позиции моторов
    ui->lineEdit_5->setText(QString::number(motPosY));
    ui->lineEdit_6->setText(QString::number(motPosZ));


    ui->lineEdit_4->setReadOnly(1);                     // Поля которые показывают позиции можно только читать
    ui->lineEdit_5->setReadOnly(1);
    ui->lineEdit_6->setReadOnly(1);

    steppingProcess = 0;                                // Флаг запуска шагов по интерполяции
    pointsIndex = 0;                                    // Индекс массива точек интерполяции

    if(DEBUG_MK)
     serialInit();                                       // Инициализация порта


}

MainWindow::~MainWindow()
{
    SerialPort->disconnect();                          // При выходе из программы закрываем порт
    delete ui;
}

void MainWindow::serialInit()                          // Конфигурация порта
{

    SerialPort = new QSerialPort(this);
    SerialPort->setPortName(ui->lineEdit_7->text());
    //SerialPort->setBaudRate(QSerialPort::Baud9600);
    SerialPort->setBaudRate(QSerialPort::Baud115200);
    SerialPort->setDataBits(QSerialPort::Data8);
    SerialPort->setParity(QSerialPort::NoParity);
    SerialPort->setStopBits(QSerialPort::OneStop);
    SerialPort->setFlowControl(QSerialPort::NoFlowControl);

    if (SerialPort->open(QIODevice::ReadWrite))
    {
       qDebug() << "Serial connected";
       connect(SerialPort, SIGNAL(readyRead()), this, SLOT(SerialPortRead()));
    }

}

void MainWindow::SerialPortRead()
{
  QByteArray data = SerialPort->readAll();
  fromPort += QString(data);



  if(fromPort.back() == '\n')
   {
      //qDebug() << "from AVR - "<< fromPort;
      //qDebug() << "control sum - " << QString::number(checkSum(fromPort.split('#').first().toLatin1().constData()));
      if(checkSum(fromPort.split('#').first().toLatin1().constData()) != fromPort.split('#').back().toInt())          //  Сверяем контрольные суммы
      {
          qDebug() << "check sum error";
          qDebug() << "from port - "<< fromPort;
          qDebug() << "control sum - " << QString::number(checkSum(fromPort.split('#').first().toLatin1().constData()));
      }


      if(fromPort.contains('X') && fromPort.contains('Y') && fromPort.contains('Z'))
      {
          motPosX = fromPort.split('X').last().split('Y').first().toInt();
          motPosY = fromPort.split('X').last().split('Y').last().split('Z').first().toInt();
          motPosZ = fromPort.split('X').last().split('Y').last().split('Z').last().split('#').first().toInt();
      }
      else if (fromPort.contains('R'))
      {
          motPosR = fromPort.split('R').last().split('#').first().toInt();
      }
      ui->lineEdit_4->setText(QString::number(motPosX));
      ui->lineEdit_5->setText(QString::number(motPosY));
      ui->lineEdit_6->setText(QString::number(motPosZ));
      ui->lineEdit_13->setText(QString::number(motPosR));

      if(steppingProcess == 1)
      {
         pointsIndex++;
         if(pointsIndex == points.size())
         {
             steppingProcess = 0;
             pointsIndex = 0;
             points.clear();
         }
         else
         goToXY2(points.at(pointsIndex).first, points.at(pointsIndex).second);
      }

      update();
   }

  if(fromPort.split('\n').last().size())
   fromPort = fromPort.split('\n').last();
  else
   fromPort.clear();

}

void MainWindow::on_pushButton_4_clicked()
{
    serialInit();
    if(SerialPort->isOpen())
    {
        ui->pushButton_4->setEnabled(0);
        ui->pushButton->setEnabled(1);
        ui->pushButton_2->setEnabled(1);
        ui->pushButton_3->setEnabled(1);
        ui->pushButton_5->setEnabled(1);
        ui->pushButton_6->setEnabled(1);
        ui->pushButton_7->setEnabled(1);
        ui->pushButton_8->setEnabled(1);
    }
}


void MainWindow::on_pushButton_clicked()
{
   goMotX(ui->lineEdit->text().toInt());
}


void MainWindow::on_pushButton_2_clicked()
{
   goMotY(ui->lineEdit_2->text().toInt());
}


void MainWindow::on_pushButton_3_clicked()
{
   goMotZ(ui->lineEdit_3->text().toInt());
}

void MainWindow::goMotX(int steps)
{
   steps = -steps;

    if(SerialPort->isOpen())
     SerialPort->write(getToMotStr(motX, steps).toLatin1().constData());

    if(DEBUG_MK)
    testMegaParser(getToMotStr(motX, steps));  //!
}

void MainWindow::goMotY(int steps)
{
    steps = -steps;

    if(SerialPort->isOpen())
     SerialPort->write(getToMotStr(motY, steps).toLatin1().constData());

    if(DEBUG_MK)
    testMegaParser(getToMotStr(motY, steps));  //!
}

void MainWindow::goMotZ(int steps)
{
    if(SerialPort->isOpen())
     SerialPort->write(getToMotStr(motZ, steps).toLatin1().constData());

    if(DEBUG_MK)
    testMegaParser(getToMotStr(motZ, steps));  //!
}

QString MainWindow::getToMotStr(int mot, int steps)   // Функция для формирования строки запроса к МК для 1ого мотора
{
    int x = mot == motX ? steps : 0;
    int y = mot == motY ? steps : 0;
    int z = mot == motZ ? steps : 0;
    QString str;

    QString chk;
    str = "X";
    str += QString::number(x);
    str += "Y";
    str += QString::number(y);
    str += "Z";
    str += QString::number(z);
    chk = QString::number(checkSum(str.toLatin1().constData()));
    str += "#";
    str += chk;
    str += '\n';
    return str;

}


void MainWindow::goToX(int pos)
{
    goMotX(pos - motPosX);
}

void MainWindow::goToY(int pos)
{
   goMotY(pos - motPosY);
}

void MainWindow::goToZ(int pos)
{
   goMotZ(pos - motPosZ);
}



void MainWindow::goToXY2(int x, int y)    // Функция для формирования строки запроса к МК для 2ух моторов (X/Y)
{

    QString chk;
    QString toPort;
    toPort = "X";
    toPort += QString::number(x);
    toPort += "Y";
    toPort += QString::number(y);
    toPort += "Z";
    toPort += QString::number(0);
    chk = QString::number(checkSum(toPort.toLatin1().constData()));
    toPort += "#";
    toPort += chk;
    toPort += '\n';

    if(SerialPort->isOpen())
     SerialPort->write(toPort.toLatin1().constData());

    if(DEBUG_MK)
     testMegaParser(toPort); //!

}

void MainWindow::drawCircle(int x, int y, int a, int b, QVector<QPair<int, int>> *points_circle)
{
    double h = a;
    double w = b;
    double psi = 0;
    double phi = 0;
    int pix = 0;
    int piy = 0;
    for (int i = 0; i<=360; i++)
    {
     psi = i*3.1415/180.0;
     phi = atan2(h*sin(psi), w*cos(psi));
     pix = (float)((h*cos(phi))+x);
     piy = (float)((w*sin(phi))+y);
     points_circle->push_back(QPair<int, int>{pix, piy});
    }
}

void MainWindow::drawRect(int x, int y, int a, int b, QVector<QPair<int, int> > *points_r)
{
    int pix = x;
    int piy = y;
    points_r->push_back(QPair<int, int>{pix, piy});
    pix = a+x;
    points_r->push_back(QPair<int, int>{pix, piy});
    piy = b+y;
    points_r->push_back(QPair<int, int>{pix, piy});
    pix = x;
    points_r->push_back(QPair<int, int>{pix, piy});
    piy = y;
    points_r->push_back(QPair<int, int>{pix, piy});
}

void MainWindow::drawLine(int x1, int y1, int x2, int y2, QVector<QPair<int, int> > points_l)
{
    int pix = x1;
    int piy = y1;
    points_l.push_back(QPair<int, int>{pix, piy});
    pix = x2+x1;
    piy = y2+y1;
    points_l.push_back(QPair<int, int>{pix, piy});
}


unsigned char MainWindow::checkSum(const char *str)  // Функция проверки контрольных сумм
{

 unsigned char check = 0;
 while(*str != 0 && *str != '#')
  check ^= *str++;

 //if(*str == 0)
 //    qDebug() << "0";

  return check;

}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
   //qDebug()<<event->x();

}
/////////my
void MainWindow::mousePressEvent(QMouseEvent *event)
{
  if(event->button() == Qt::LeftButton){

      mouse_pos.setX(event->x() - SHEET_POS_X);
      mouse_pos.setY(event->y() - SHEET_POS_Y);
      if(mouse_pos.x() > SHEET_SIZE_X) mouse_pos.setX(-1);
      if(mouse_pos.y() > SHEET_SIZE_Y) mouse_pos.setY(-1);
      qDebug()<<mouse_pos;
      if(mouse_pos.x() >= 0 && mouse_pos.y() >=0)
      ui->label_26->setText("x- " + QString::number(mouse_pos.x()) + ", y-" + QString::number(mouse_pos.y()));

  }
}
/////////my



void MainWindow::on_pushButton_7_clicked()     // Функция обработчик нажатия кнопки GUI goToX
{
    goToX(ui->lineEdit_8->text().toInt());     // Берем строку из поля ввода, переводим ее в число и суем в функцию...
}


void MainWindow::on_pushButton_6_clicked()     // Функция обработчик нажатия кнопки GUI goToY
{
    goToY(ui->lineEdit_9->text().toInt());
}


void MainWindow::on_pushButton_5_clicked()     // Функция обработчик нажатия кнопки GUI goToZ
{
    goToZ(ui->lineEdit_10->text().toInt());
}



void MainWindow::paintEvent(QPaintEvent *) // Функция отрисовки положения осей
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::black);

    painter.drawRect
    (
        SHEET_POS_X,
        SHEET_POS_Y,
        SHEET_SIZE_X,
        SHEET_SIZE_Y
    );

    painter.setPen(Qt::red);

    painter.drawLine
    (
        SHEET_POS_X + motPosX*STEP_SIZE_MM*DEVIDER_COOF_X,
        SHEET_POS_Y,
        SHEET_POS_X + motPosX*STEP_SIZE_MM*DEVIDER_COOF_X,
        SHEET_POS_Y + SHEET_SIZE_Y
    );

    painter.drawLine
    (
        SHEET_POS_X,
        SHEET_POS_Y + motPosY*STEP_SIZE_MM*DEVIDER_COOF_Y,
        SHEET_POS_X + SHEET_SIZE_X,
        SHEET_POS_Y + motPosY*STEP_SIZE_MM*DEVIDER_COOF_Y
    );

    if(points_figures.size())
     for(QPair<int, int> &p : points_figures)
        painter.drawPoint(p.first, p.second);

}


void MainWindow::on_pushButton_8_clicked()    // Функция обработчик нажатия кнопки GUI goToXY
{
    steppingProcess = 1;
    coordinats(motPosX, motPosY,  ui->lineEdit_11->text().toInt(), ui->lineEdit_12->text().toInt());
}

void MainWindow::on_btnDrawCircle_pressed()
{

    drawCircle(ui->leCircleX->text().toInt(), ui->leCircleY->text().toInt(),
               ui->leCircleA->text().toInt(), ui->leCircleB->text().toInt(),
               &points_figures);

     qDebug() << points_figures.size();

    for (int i=0; i < points_figures.size(); i++)
    {
       steppingProcess = 1;
       coordinats(motPosX, motPosY, points_figures.at(i).first, points_figures.at(i).second);
    }
}

void MainWindow::on_btnDrawRect_pressed()
{

    drawRect(ui->leRectX->text().toInt(), ui->leRectY->text().toInt(),
             ui->leRectL1->text().toInt(), ui->leRectL2->text().toInt(),
             &points_figures);

    //qDebug() << points_figures;


    for (int i=0; i < points_figures.size(); i++)
    {
       steppingProcess = 1;
       coordinats(motPosX, motPosY, points_figures.at(i).first, points_figures.at(i).second);
    }

}

void MainWindow::on_btnDrawLine_pressed()
{
    steppingProcess = 1;
    drawLine(ui->leLineX1->text().toInt(), ui->leLineY1->text().toInt(),
             ui->leLineX2->text().toInt(), ui->leLineY2->text().toInt(),
             points_figures);
    for (int i=0; i < 2; i++)
    {
        coordinats(motPosX, motPosY, points_figures.at(i).first, points_figures.at(i).second);
    }
}

void MainWindow::testMegaParser(QString str)   // Эта часть эмулирует МК, т.е. это парсер который будет в ардуине
{


    qDebug() << "from PC - " << str;

    char *p = NULL;
    char *data = &rx_buffer[0];
    char numBuf[10] = {0};
    char *num = &numBuf[0];
    ansPtr = 0;
    memset(ans, 0, 30);


    rx_wr_index = str.size();
    memcpy(rx_buffer, str.toLatin1().constData(), str.size());
    //qDebug() << "rs_buffer 0 - " << QString(rx_buffer);

    rx_buffer[rx_wr_index-1] = 0;
    p = strchr(rx_buffer, '#');

    if(p == NULL)
    {
        qDebug() << "p - NULL";
        memset(rx_buffer, 0, RX_BUFFER_SIZE);
        rx_wr_index = 0;
        return;
    }
    p++;

     if(atoi(p) == checkSum(&rx_buffer[0]))
      {

         data++;
         while((*data >= '0' && *data <= '9') || *data == '-')
          *num++ = *data++;
         n = atol(numBuf);
         memset(numBuf,0, 10);
         num = &numBuf[0];
         moveX_(n);


         data++;
         while((*data >= '0' && *data <= '9') || *data == '-')
          *num++ = *data++;
         n = atol(numBuf);
         memset(numBuf,0, 10);
         num = &numBuf[0];
         moveY_(n);


         data++;
         while((*data >= '0' && *data <= '9') || *data == '-')
          *num++ = *data++;
         n = atol(numBuf);
         memset(numBuf,0, 10);
         num = &numBuf[0];
         moveZ_(n);


        ansPtr = sprintf(ans, "X%ldY%ldZ%ld", posX, posY, posZ);
        sprintf(&ans[ansPtr], "#%u\n", checkSum(ans));
        //printf("%s", ans);
        //qDebug() << QString(ans);
        QTimer::singleShot(0,[=]{testParser(QString(ans));});

      }

     else
     {
      printf("bad\n");
      qDebug() << "bad";
     }


     memset(rx_buffer, 0, RX_BUFFER_SIZE);
     rx_wr_index = 0;

}


void MainWindow::testParser(QString data)   // Парсер который принемает строку от МК
{
  fromPort = data;

  qDebug() << "from AVR virtual - "<< fromPort;

  if(fromPort.back() == '\n')                // Смотрим если есть завершающий символ то парсим (значит строка пришла полностью)
   {
      //qDebug() << "control sum - " << QString::number(checkSum(fromPort.split('#').first().toLatin1().constData()));
      if(checkSum(fromPort.split('#').first().toLatin1().constData()) != fromPort.split('#').back().toInt())          //  Сверяем контрольные суммы
      {
          // попадаем сюда если контрольный суммы не совподают (потом будем как-то на это реагировать, пока просто показываем пользователю что что-то не так)
          qDebug() << "check sum error";
          qDebug() << "from port - "<< fromPort;
          qDebug() << "control sum - " << QString::number(checkSum(fromPort.split('#').first().toLatin1().constData()));
      }


       motPosX = fromPort.split('X').last().split('Y').first().toInt();                                        // Вычленяем из пришедшей стролки позицию мотора X
       motPosY = fromPort.split('X').last().split('Y').last().split('Z').first().toInt();                      // Вычленяем из пришедшей стролки позицию мотора Y
       motPosZ = fromPort.split('X').last().split('Y').last().split('Z').last().split('#').first().toInt();    // Вычленяем из пришедшей стролки позицию мотора Z

      ui->lineEdit_4->setText(QString::number(motPosX));                                                       // Отображаем эти позиции
      ui->lineEdit_5->setText(QString::number(motPosY));
      ui->lineEdit_6->setText(QString::number(motPosZ));

      if(steppingProcess == 1)                                                                                 // Флаг : если 1 - то в режим интерполяции запущен
      {
         pointsIndex++;
         if(pointsIndex == points.size())
         {
             steppingProcess = 0;
             pointsIndex = 0;
             points.clear();
         }
         else
         goToXY2(points.at(pointsIndex).first, points.at(pointsIndex).second);
      }

      update();
   }

  if(fromPort.split('\n').last().size())         // Если после завершающего символа что-то  есть, значит Мы приняли часть новой строки, которую нужно сохранить.
   fromPort = fromPort.split('\n').last();
  else
   fromPort.clear();                             // Если нет, то отчищаем строку которую распарсили

}


void moveX_(int steps){ posX += steps; }       // Функции эмулируют движение моторов в МК (это часть кода из codeVision)
void moveY_(int steps){ posY += steps; }
void moveZ_(int steps){ posZ += steps; }



void MainWindow::coordinats(long int x1, long int y1, long int x2, long int y2)  // Фукция интерполяции : принемает текущее положение и куда нужно прийти
{
//qDebug()<<"o_x - "<<o_x<<"  o_y - "<<o_y;
 if(x1 == x2 && y1 == y2)
   return;

 long int x = 0;
 long int y = 0;
 bool X_or_Y;              // Если больше Х флаг установлен, если У, то сброшен.
 bool remainder = 1;       // если есть остаток то 1
 double a_tmp;
 double a = 0;             // целое от деления
 double b = 0;             // кооф приращения
 double c = 0;             // остаток от деления
 int derection_ox = 1;
 int derection_oy = 1;
 double c_tmp = 0;


 if(x2 > x1)
 {
     x = x2 - x1;
     derection_ox = 1;
 }

 if(x2 < x1)
 {
     x = x1 - x2;
     derection_ox = -1;
 }


 if(y2 > y1)
 {
     y = y2 - y1;
     derection_oy = 1;
 }

 if(y2 < y1)
 {
     y = y1 - y2;
     derection_oy = -1;
 }


 if (x && y)
 {

   if (x > y)
   {
    c = fmod(double(x)/double(y), double(1.0));
    a = double(x)/double(y)-c;
    X_or_Y = 0;
   }

   if (x < y)
   {
    c = fmod(double(y)/double(x), double(1.0));
    a = double(y)/double(x)-c;
    X_or_Y = 1;
   }

 if(c != 0) a++;
 if(c == 0) remainder = 0;
 else       remainder = 1;

 if (x==y)
 {
     a = y/x;
     remainder = 0;
 }

 if (X_or_Y) a_tmp = x;
 else        a_tmp = y;


  while(a_tmp)
  {
    if(X_or_Y)
     points.append(QPair<int, int>{1 * derection_ox, (a-b) * derection_oy});
    else
     points.append(QPair<int, int>{(a-b) * derection_ox, 1 * derection_oy});


   if(remainder)
   {
    c_tmp += c;
    if (c_tmp<1) b=1;
    if (c_tmp>=1)
     {
        b=0;
        c_tmp = c_tmp - 1.0;
     }
    }
   a_tmp--;

  }

 }

  else
   {

     if(x)
      while(x--)
       points.append(QPair<int, int>{1 * derection_ox, 0});

     if(y)
      while(y--)
         points.append(QPair<int, int>{0, 1 * derection_oy});

   }


  goToXY2(points.first().first, points.first().second);

}


void MainWindow::goToHome()
{

    qDebug() << "GO HOME";

     QString chk;
     QString str = "H";
     chk = QString::number(checkSum(str.toLatin1().constData()));
     str += "#";
     str += chk;
     str += '\n';

     if(SerialPort->isOpen())
      SerialPort->write(str.toLatin1().constData());

     if(DEBUG_MK)
     {
         motPosX = motPosY = motPosZ = 0;
         ui->lineEdit_4->setText(QString::number(motPosX));
         ui->lineEdit_5->setText(QString::number(motPosY));
         ui->lineEdit_6->setText(QString::number(motPosZ));
         update();
     }


}

void MainWindow::on_pushButton_10_clicked()  //ROT R BUT
{
  int pos = ui->leLineR->text().toInt();
  QString chk;
  QString toPort;
  toPort = "R";
  toPort += QString::number(pos);
  chk = QString::number(checkSum(toPort.toLatin1().constData()));
  toPort += "#";
  toPort += chk;
  toPort += '\n';

  if(SerialPort->isOpen())
   SerialPort->write(toPort.toLatin1().constData());

  if(DEBUG_MK)
   testMegaParser(toPort); //!

}

//my
void MainWindow::on_pushButton_11_clicked()//GoToPos мышка

{
    int pixels_x, pixels_y;

    if(mouse_pos.x() >= 0 && mouse_pos.y() >=0){
    //pixels = mouse_pos.x()*((((REAL_SIZE_X_MM/LEN_ON_REVOLUTION_MM)*STEPS_ON_REVOLUTION_MM)/REAL_SIZE_X_MM)*DEVIDER_COOF_X);
    pixels_x = (mouse_pos.x() / DEVIDER_COOF_X) * (STEPS_ON_REVOLUTION_MM / LEN_ON_REVOLUTION_MM);
    pixels_y = (mouse_pos.y() / DEVIDER_COOF_Y) * (STEPS_ON_REVOLUTION_MM / LEN_ON_REVOLUTION_MM);

        qDebug() << pixels_x;
        qDebug() << pixels_y;

        QString chk;//контрольная сумма
        QString toPort;// строка, следующа в мк

        //Формирования строки вида X Координата Y Координата Z Координата # chk /n
        toPort = "X";
        toPort += QString::number(pixels_x - posX);
        toPort += "Y";
        toPort += QString::number(pixels_y - posY);
        toPort += "Z";
        toPort += QString::number(0);
        chk = QString::number(checkSum(toPort.toLatin1().constData()));
        toPort += "#";
        toPort += chk;
        toPort += '\n';

        if(SerialPort->isOpen())//если порт открыт
         SerialPort->write(toPort.toLatin1().constData());

        if(DEBUG_MK)//  для отладки
         testMegaParser(toPort); //!

 }
}
//my
