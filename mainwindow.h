#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define APP_NAME "A Great Chance"
#define APP_VERSION "1.0"

#define FILE_POOL "pool.csv"
#define KEY_NULL "#NULL"
#define KEY_EMPTY "<Empty>"
#define KEY_BEGIN "----"

#include <QMainWindow>
#include <QTimer>
#include <QList>
#include <QLabel>
#include <QTime>

struct Person{
    QString name;
    int sex;
};

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void loadPool();
    void updatePool();

    void fire();
    void show_pool();
    void reset();

private:
    Ui::MainWindow *ui;
    QTime time;

    int cd;
    int state;
    bool repeat;
    int count;
    QString name;

    QList<Person> pool;
    QList<QString> currentPool;
    QList<QString> history;

    QLabel lb_total_count;
    QLabel lb_cur_count;
    QLabel lb_remained_count;
    QLabel lb_last_name;

signals:
    void fired(QString name);
};
#endif // MAINWINDOW_H
