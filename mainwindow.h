#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QProcess>
#include <QFile>
#include <QTime>
#include <QDebug>
#include <QCheckBox>
#include <QStringList>
#include <QStringListModel>
#include <QClipboard>
#include <QFont>
#include <QTimer>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
      QStringListModel* model = new QStringListModel();

private slots:
    void on_pushButtonConnect_clicked();
    void on_pushButtonSend_clicked();
    void on_ListViewFoundFiles_clicked(const QModelIndex &index);
    void on_pushButtonClear_clicked();
    void on_pushButtonCopyClipboard_clicked();
    void on_lineEditSendToServer_returnPressed();
    void updateTimer();
    void clearOut();
    void createLabelsForStatusbar();

private:
    Ui::MainWindow *ui;
    QTcpSocket *socket;
    QByteArray Data;
    void SendToServer(QString str);
    quint16 nextBlockSize;
    QTimer *timer;
    int timeLeft;
    static const int maximumNumberlabel = 1;
    QLabel* Label[maximumNumberlabel];

public slots:
     void slotReadyReadSocket();
 };
#endif // MAINWINDOW_H
