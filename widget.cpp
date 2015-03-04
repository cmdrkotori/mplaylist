#include "widget.h"
#include "ui_widget.h"
#include <qdrag.h>
#include <qmimedata.h>
#include <QDebug>
#include <QProcess>

const int QP_EXIT_END = 2;
const int QP_EXIT_QUIT = 1;
const int QP_EXIT_NONE = 0;


Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget),
    qp(NULL)
{
    ui->setupUi(this);
}

Widget::~Widget()
{
    if (qp) {
        delete qp;
    }
    delete ui;
}

void Widget::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls()) {
        e->acceptProposedAction();
    }
}

void Widget::dropEvent(QDropEvent *e)
{
    foreach (const QUrl &url, e->mimeData()->urls()) {
        const QString &fileName = url.toLocalFile();
        queue.append(fileName);
    }
    int index = ui->listWidget->currentRow();
    ui->listWidget->clear();
    ui->listWidget->addItems(queue);
    ui->listWidget->setCurrentRow(index);
}

void Widget::on_toolButton_4_clicked()
{
    QString fileName = ui->listWidget->currentItem()->text();
    qDebug() << fileName;
    playFile(fileName);
}

void Widget::on_process_read_output()
{
    QString out(qp->readAllStandardOutput());

    if (out.contains("Exiting... (End of file)")) {
        exitState = QP_EXIT_END;
    }
    if (out.contains("Exiting... (Quit))")) {
        exitState = QP_EXIT_QUIT;
    }
}

void Widget::on_process_finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitState == 0) {
        qDebug() << "Abnormal termination";
    }
    if (exitState == QP_EXIT_END) {
        qDebug() << "End of file read";
        removeItemAndPlayNext();
    }
    if (exitState == QP_EXIT_QUIT) {
        qDebug() << "User quit";
    }
}

void Widget::playFile(QString fileName)
{
    if (qp) {
        qp->kill();
        qp->waitForFinished();
        delete qp;
        qp = NULL;
    }
    qp = new QProcess(this);
    exitState = QP_EXIT_NONE;
    connect(qp, SIGNAL(readyReadStandardOutput()), this, SLOT(on_process_read_output()));
    connect(qp, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(on_process_finished(int,QProcess::ExitStatus)));
    qp->start("mpv", QStringList() << fileName);
    playingFile = fileName;
}

void Widget::removeItemAndPlayNext()
{
    int index = ui->listWidget->currentRow();
    queue.removeAll(playingFile);
    repopulateList();

    if (queue.length() > index)
        playFile(queue.at(index));
}

void Widget::repopulateList()
{
    int index = ui->listWidget->currentRow();
    ui->listWidget->clear();
    ui->listWidget->addItems(queue);
    ui->listWidget->setCurrentRow(index);
}

void Widget::on_listWidget_doubleClicked(const QModelIndex &index)
{
    if (queue.length() > index.row())
        playFile(queue.at(index.row()));
}

void Widget::on_moveUpButton_clicked()
{
    int index = ui->listWidget->currentRow();
    if (index > 0) {
        queue.move(index, index - 1);
        repopulateList();
        ui->listWidget->setCurrentRow(index - 1);
    }
}

void Widget::on_moveDownButton_clicked()
{
    int index = ui->listWidget->currentRow();
    if (index < queue.length() - 1) {
        queue.move(index, index + 1);
        repopulateList();
        ui->listWidget->setCurrentRow(index + 1);
    }
}

void Widget::on_removeButton_clicked()
{
    int index = ui->listWidget->currentRow();
    if (index < queue.length()) { // this is probably always true...
        queue.removeAt(index);
        repopulateList();
    }
}

void Widget::on_stopButton_clicked()
{
    if (qp) {
        qp->kill();
    }
}

void Widget::on_playButton_clicked()
{
    int index = ui->listWidget->currentRow();
    if (index > 0 && queue.length() < index)
        playFile(queue.at(index));
}
