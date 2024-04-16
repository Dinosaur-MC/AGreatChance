#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPushButton>
#include <QRandomGenerator>
#include <QFile>
#include <QRadioButton>
#include <QDialog>
#include <QListWidget>
#include <QPropertyAnimation>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 变量初始化
    time = QTime::currentTime();
    cd = 150;
    state = 0;
    repeat = 0;
    count = 0;
    name = KEY_NULL;

    // 页面初始化
    this->setWindowTitle(QString(APP_NAME) + " - V" + APP_VERSION);
    ui->statusbar->addWidget(&lb_total_count);
    ui->statusbar->addWidget(&lb_cur_count);
    ui->statusbar->addPermanentWidget(&lb_remained_count);
    ui->statusbar->addPermanentWidget(&lb_last_name);

    QVBoxLayout *layout = new QVBoxLayout;
    QDialog *w = new QDialog(this);
    w->setLayout(layout);
    w->setFont(QFont("微软雅黑", 16));
    w->resize(w->width(), this->height());
    w->move(this->x() - w->width() - 10, this->y());
    QListWidget *list = new QListWidget;
    layout->addWidget(list);

    // 启动初始化
    loadPool();
    updatePool();

    // 连接控件、槽函数
    connect(ui->radAll, &QRadioButton::clicked, this, [=](bool checked){
        if (checked)
        {
            state = 0;
            updatePool();
        }
    });
    connect(ui->radMale, &QRadioButton::clicked, this, [=](bool checked){
        if (checked)
        {
            state = 1;
            updatePool();
        }
    });
    connect(ui->radFemale, &QRadioButton::clicked, this, [=](bool checked){
        if (checked)
        {
            state = 2;
            updatePool();
        }
    });

    connect(ui->cbEnableRepeat, &QCheckBox::stateChanged, this, [=](){
        repeat = ui->cbEnableRepeat->isChecked();
        if(repeat)
        {
            lb_cur_count.setText("池大小：" + QString::number(currentPool.count()));
            lb_remained_count.setText("");
        }
        else
            lb_remained_count.setText("剩余量：" + QString::number(currentPool.count()));
    });

    connect(ui->btnFire, &QPushButton::clicked, this, &MainWindow::fire);
    connect(ui->btnHistory, &QPushButton::clicked, w, [=](){
        if (w->isHidden())
        {
            w->show();
            w->resize(w->width(), this->height());
            w->move(this->x() - w->width() - 10, this->y());
            list->scrollToItem(list->item(list->count() - 1));
        }
        else
            w->hide();
    });
    connect(this, &MainWindow::fired, list, [=](QString name){
        qDebug() << "Result:" << name;
        if (name != KEY_NULL)
        {
            list->addItem(name);
            list->scrollToItem(list->item(list->count() - 1));
        }
    });
    connect(ui->btnPool, &QPushButton::clicked, this, &MainWindow::show_pool);
    connect(ui->btnReset, &QPushButton::clicked, this, &MainWindow::reset);
    connect(ui->btnReset, &QPushButton::clicked, list, &QListWidget::clear);
    connect(ui->btnExit, &QPushButton::clicked, this, &MainWindow::close);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::loadPool()
{
    qDebug() << "Loading pool file...";
    QFile fp(FILE_POOL);
    if(!fp.open(QIODevice::ReadOnly))
    {
       if(!fp.open(QIODevice::ReadWrite))
       {
           qDebug() << "Cannot open the pool file!";
       }
    }

    QString raw_data = fp.readAll().trimmed();
    QStringList splitted_data = raw_data.split('\n');
    if(splitted_data.count() == 0)
        return;

    for(int i = 0; i < splitted_data.count(); i++)
    {
        QStringList temp = splitted_data[i].split(',');
        Person p;
        if(temp.count() != 2)
            continue;
        p.name = temp[0].simplified();
        p.sex = temp[1].simplified().toUInt();
        pool.append(p);
    }

    fp.close();
    lb_total_count.setText("池容量：" + QString::number(pool.count()));
    qDebug() << "The pool has been loaded.";
}

void MainWindow::updatePool()
{
    qDebug() << "Refreshing...";

    currentPool.clear();
    switch (state)
    {
    case 1:
        for (int i = 0; i < pool.count(); i++)
            if (pool.at(i).sex == 0)
                currentPool.append(pool.at(i).name);
        break;
    case 2:
        for (int i = 0; i < pool.count(); i++)
            if(pool.at(i).sex == 1)
                currentPool.append(pool.at(i).name);
        break;
    default:
        for (int i = 0; i < pool.count(); i++)
            currentPool.append(pool.at(i).name);
        break;
    }
    count = currentPool.count();

    ui->Screen->setText(KEY_BEGIN);
    lb_cur_count.setText("池大小：" + QString::number(count));
    lb_remained_count.setText("");
}

void MainWindow::fire()
{
    if (time.msecsTo(QTime::currentTime()) < cd)
        return;

    qDebug() << "+ Count:" << count;
    if (count > 0)
    {
        int p = QRandomGenerator::global()->bounded(count);
        name = currentPool.at(p);
        if (!repeat)
        {
            currentPool.takeAt(p);
            count = currentPool.count();
            lb_remained_count.setText("剩余量：" + QString::number(currentPool.count()));
        }

        if (!history.isEmpty())
            lb_last_name.setText("上一个：" + history.last());
        history.append(name);

        ui->Screen->setText(name);
        QPropertyAnimation *animation = new QPropertyAnimation(ui->Screen, "pos", this);
        animation->setDuration(cd);
        animation->setStartValue(QPoint(ui->Screen->x(), ui->Screen->y() - ui->Screen->height()));
        animation->setEndValue(ui->Screen->pos());
        animation->setEasingCurve(QEasingCurve::OutInElastic);
        animation->start(QAbstractAnimation::DeleteWhenStopped);

        time = QTime::currentTime();
        emit fired(name);
    }
    else
    {
        name = KEY_NULL;
        ui->Screen->setText(KEY_EMPTY);
    }
}

void MainWindow::show_pool()
{
    QDialog w;
    w.setFont(QFont("微软雅黑", 16));
    QListWidget list(&w);
    list.setFixedSize(300,400);
    w.setFixedSize(300,400);
    w.setModal(true);
    for (int i = 0; i < currentPool.count(); i++)
        list.addItem(QString::number(i+1) + ". " + currentPool.at(i));
    w.show();
    w.exec();
}

void MainWindow::reset()
{
    updatePool();
    lb_last_name.setText("");
}

