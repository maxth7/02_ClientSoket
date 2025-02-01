#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    ,socket(new QTcpSocket(this))
    ,timer(new QTimer(this)), timeLeft(0) {

    ui->setupUi(this);

    createLabelsForStatusbar();

    ui->groupBoxErr->setStyleSheet("QGroupBox { title : padding-top: 0px; }");
    ui->groupBoxListFiles->setStyleSheet("QGroupBox {title : padding-top: 0px; }");
    ui->groupBoxContents->setStyleSheet("QGroupBox  {title : padding-top: 0px; }");

    connect(timer, &QTimer::timeout, this, &MainWindow::updateTimer);

    QAction*  choosingTheme   = new QAction(tr("&Setting"), this);
     ui->menubar->addAction(choosingTheme);

    ui->menubar->setStyleSheet(
        "QMenuBar {"
        "   background-color: #FFDEAD;"
        "   color: black;"
        "   font-size: 16px;"
        "   font-weight: bold;"
        "}"
        "QMenuBar::item {"
        "   spacing: 3px;"
        "   padding: 5px;"
        "}"
        "QMenuBar::item:selected {"
        "   background-color: #2E2E2E;"
        "   border: 2px solid #FF0000;"
        "}"
        );
     setWindowIcon(QIcon(":/images/icon/socket_icon.png"));
     this->setStyleSheet("background-color: lightgray;");

     QString textErrs = "QTextBrowser { background-color: white; color: red; }";
     ui->textBrowserErrors->setStyleSheet(textErrs);

     QString textContets="QTextBrowser { background-color: white; color: blue; }";
     ui->textBrowserContent->setStyleSheet(textContets);

     ui->ListViewFoundFiles->setStyleSheet("QListView { background-color:white; }"
                                           "QListView::item { color: green; }"
                                           "QListView::item:selected { background-color: darkblue; color: red; }");

     ui->pushButtonConnect->setStyleSheet("QPushButton { background-color: white; color: green; }");


    socket= new QTcpSocket(this);

    connect(socket,&QTcpSocket::readyRead,this,&MainWindow::slotReadyReadSocket);
    connect(socket,&QTcpSocket::disconnected,socket,&MainWindow::deleteLater);

    nextBlockSize=0;

    ui->pushButtonClear->setFixedSize(25, ui->pushButtonClear->height());
    ui->pushButtonSend->setEnabled(false);
    ui->pushButtonSend->setStyleSheet("QPushButton { background-color: white; color: gray; }");

    ui->pushButtonConnect->setStyleSheet("QPushButton { background-color: white; color: green ; }");
    ui->pushButtonCopyClipboard->setEnabled(false);

    ui->lineEditSendToServer->setPlaceholderText("Enter it here...");

    ui->labelMessageFromServer->setStyleSheet("color: green;background-color: lightgray ;");
    ui->labelMessageFromServer->setAlignment(Qt::AlignCenter);
}

void MainWindow::createLabelsForStatusbar(){

    ui->statusbar->setStyleSheet("border: 2px solid black");

    QFont font;
    font.setPointSize(12);
    font.setBold(true);

    for(int i=0;i<maximumNumberlabel;++i){

        Label[i] = new QLabel("Waiting for the connection", this);
        Label[i]->setAlignment(Qt::AlignLeft);
        Label[i]->setFont(font);
        Label[i]->setStyleSheet("background-color: #FF8C00; color: #2E2E2E;");
        ui->statusbar->addWidget(Label[i]);
        Label[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    }
}

void MainWindow::on_ListViewFoundFiles_clicked(const QModelIndex &index){
      if (index.isValid()) {
         QString fileName = index.data(Qt::DisplayRole).toString();

         ui->lineEditSendToServer->setText(fileName);
         ui->textBrowserErrors->clear();
         SendToServer(fileName);
    }
}

MainWindow::~MainWindow(){
    delete ui;

    socket->disconnectFromHost();
    socket->close();
    delete socket;
}

void MainWindow::on_pushButtonConnect_clicked(){
    clearOut();
    //QObject::disconnect(socket, &QTcpSocket::connected, nullptr, nullptr);

    timeLeft = 0; // Установите начальное значение времени
    ui->pushButtonConnect->setText(QString::number(timeLeft)); // Отобразите начальное значение на кнопке
    ui->pushButtonConnect->setEnabled(false);
    timer->start(1000); // Запустите таймер с интервалом 1 секунда


    //socket = new QTcpSocket();

    //socket->connectToHost("127.0.0.1",4444);
    // Проверяем состояние сокета
    if (socket->state() == QTcpSocket::UnconnectedState) {
        socket->connectToHost("127.0.0.1", 4444);
    } else {
        qDebug() << "Сокет не готов к подключению.";

    }

    // Обработка сигналов
    connect(socket, &QTcpSocket::connected, [this]() {

        qDebug() << "Лямбда connected Сервер запущен и подключение успешно!";
        timer->stop(); // Остановите таймер, если время вышло
        ui->labelMessageFromServer->setStyleSheet("QLabel { color: green; }");
        // ui->labelMessageFromServer->setText("The server is running and the connection is successful!");
        Label[0]->setText("The server is running and the connection is successful!");

        ui->pushButtonSend->setEnabled(true);
        ui->pushButtonSend->setStyleSheet("QPushButton { background-color: white; color: green; }");
        ui->pushButtonConnect->setEnabled(false);
        ui->pushButtonConnect->setStyleSheet("QPushButton { background-color: white; color: gray ; }");

    });


    connect(socket, &QTcpSocket::errorOccurred, [this]() {

        ui->labelMessageFromServer->setStyleSheet("QLabel { color: red; }");
        ui->labelMessageFromServer->setText("Connection error:" + socket->errorString());
        ui->pushButtonSend->setEnabled(false);
        ui->pushButtonSend->setStyleSheet("QPushButton { background-color: white; color: gray; }");

        timer->stop();

        ui->pushButtonConnect->setStyleSheet("QPushButton { color: red; }");

        ui->pushButtonConnect->setText("Time's up!");
        Label[0]->setText("No connection to the server!");

        QObject::connect(socket, &QTcpSocket::disconnected, [this]() {

            socket->deleteLater();
            socket = new QTcpSocket();
            ui->pushButtonConnect->setText("Connect");
            setWindowTitle("Client without a handle");
            connect(socket,&QTcpSocket::readyRead,this,&MainWindow::slotReadyReadSocket);
            connect(socket,&QTcpSocket::disconnected,socket,&MainWindow::deleteLater);

        });


        QTimer::singleShot(3000, [&]() {

            ui->pushButtonConnect->setText("Try to connect again");
            ui->pushButtonConnect->setStyleSheet("QPushButton { background-color: white; color: green ; }");
            ui->labelMessageFromServer->setText("");
            ui->pushButtonConnect->setEnabled(true);
            ui->pushButtonSend->setEnabled(false);
            ui->pushButtonSend->setStyleSheet("QPushButton { background-color: white; color: gray; }");

        });

    });

}

void MainWindow::updateTimer() {
    if (timeLeft < 10) {
        timeLeft++;
        ui->pushButtonConnect->setText(QString::number(timeLeft));
    } else {
         timer->stop();
         ui->pushButtonConnect->setText("Time's up!");

        QTimer::singleShot(3000, [&]() {
         ui->pushButtonConnect->setStyleSheet("QPushButton { color: green; }");
         ui->pushButtonConnect->setText("Try to connect again");
         ui->labelMessageFromServer->setText("");
         ui->pushButtonConnect->setEnabled(true);

        });
    }
}

void MainWindow::clearOut(){
    if(ui->checkClearContent->isChecked()){
        ui->textBrowserContent->clear();
        ui->pushButtonCopyClipboard->setEnabled(false);
        ui->textBrowserContent->update();
    }

    if(ui->checkBoxClearListFiles->isChecked()){
        model->setStringList(QStringList());
    }
    if(ui->checkBoxErr->isChecked()){
        ui->textBrowserErrors->clear();
        ui->textBrowserErrors->update();
    }
}

void MainWindow::SendToServer(QString str){

     Data.clear();
     QDataStream out(&Data, QIODevice::WriteOnly);
     out.setVersion(QDataStream::Qt_6_8);
     QString strToserver= str;
     out<<quint16(0)<<strToserver;
     out.device()->seek(0);
     out<<quint16(Data.size()-sizeof(quint16));

     socket->write(Data);

}

void MainWindow::slotReadyReadSocket(){

    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_6_8);

    if (socket->state() != QAbstractSocket::ConnectedState) {
        ui->labelMessageFromServer->setStyleSheet("QLabel { color: red; }");
        ui->labelMessageFromServer->setText("Socket is not connected:"
                                            + socket->errorString());

        return;
    }

    if (in.status() == QDataStream::Ok) {
        while (true) {
            if (nextBlockSize == 0) {

                if (static_cast<quint64>(socket->bytesAvailable()) < sizeof(quint16)) {
                    break;
                }
                in >> nextBlockSize;
            }

            if (socket->bytesAvailable() < nextBlockSize) {
                break;
            }

            QString str;
            in >> str;
            nextBlockSize = 0;

            QChar firstChar=str.at(0);
            char receivedSymbol = firstChar.toLatin1();

            switch(receivedSymbol){
                case '!':{

                     ui->labelMessageFromServer->setText("Greetings from the server!");
                     Label[0]->setText("The server is running and the connection is successful!");
                     int lenDescr=str.length()-1;
                     QString Descr="Client received socket: "+
                                     str.right(lenDescr);

                     setWindowTitle(Descr);

                     timer->stop();

                     ui->pushButtonConnect->setText("Connect");
                     ui->lineEditSendToServer->setEnabled(true);
                    break;
                }
                case ',':{
                     QString strWithout1Comma=str.right(str.length()-1);

                     QStringList stringList = strWithout1Comma.split(",");

                     model->setStringList(stringList);

                     ui->ListViewFoundFiles->setModel(model);

                     if (model) {
                         int itemCount = model->rowCount();
                         ui->labelAllFiles->setText("Total found: "+QString::number(itemCount));
                     } else {
                        //qDebug() << "Модель не установлена." ;
                    }
                    break;
                }
                case '*':{
                    if(ui->checkClearContent->isChecked()){
                        ui->textBrowserContent->clear();
                    }
                    ui->textBrowserContent->append(str.right(str.length()-1));

                    ui->pushButtonCopyClipboard->setEnabled(true);
                    ui->pushButtonCopyClipboard->setStyleSheet("QPushButton { background-color: white; color: purple; }");

            break;
                }
                case '?':{

                    ui->textBrowserErrors->clear();
                    ui->textBrowserErrors->append(str.right(str.length()-1));

                break;
                }
                default: {
                    ui->textBrowserErrors->clear();
                    ui->textBrowserErrors->append("Unknown symbol="+str);
                break;
                }
            }
        }
    } else {
        ui->labelMessageFromServer->setText("From Server DataStream error!");
    }
}

void MainWindow::on_pushButtonSend_clicked(){
   clearOut();

   ui->pushButtonCopyClipboard->setEnabled(false);
   ui->pushButtonCopyClipboard->setStyleSheet("QPushButton { background-color: white; color: black; }");
   SendToServer(ui->lineEditSendToServer->text().trimmed());
}


void MainWindow::on_pushButtonClear_clicked(){
    ui->lineEditSendToServer->clear();
}

void MainWindow::on_pushButtonCopyClipboard_clicked(){

    ui->textBrowserContent->selectAll();
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(ui->textBrowserContent->textCursor().selectedText());
}

void MainWindow::on_lineEditSendToServer_returnPressed(){
       SendToServer(ui->lineEditSendToServer->text().trimmed());
}


