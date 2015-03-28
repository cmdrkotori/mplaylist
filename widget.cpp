#include "widget.h"
#include "ui_widget.h"
#include "player.h"
#include <qdrag.h>
#include <qmimedata.h>
#include <QDebug>
#include <QProcess>
#include <QFileDialog>
#include <QFileInfo>


Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget),
    p()
{
    ui->setupUi(this);
    connect(&p, SIGNAL(playbackFinished(QString)), SLOT(player_playbackFinished(QString)));
}

Widget::~Widget()
{
    delete ui;
}

void Widget::setQueue(const QStringList &queue)
{
    p.stopFile();
    this->queue = queue;
    repopulateList(false);
}

QStringList Widget::getQueue()
{
    return queue;
}

void Widget::setTitle(const QString &title)
{
    this->title = title;
}

QString Widget::getTitle()
{
    return title;
}

void Widget::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls()) {
        e->acceptProposedAction();
    }
}

void Widget::dropEvent(QDropEvent *e)
{
    bool changed = false;
    foreach (const QUrl &url, e->mimeData()->urls()) {
        const QString &fileName = url.toLocalFile();
        if (p.checkFile(fileName)) {
            queue.append(fileName);
            changed = true;
        }
    }
    if (changed) {
        repopulateList();
        emit playlistChanged(this);
    }
}

void Widget::player_playbackFinished(const QString &fileJustPlayed)
{
    int index = ui->listWidget->currentRow();
    queue.removeAll(fileJustPlayed);
    repopulateList();
    emit playlistChanged(this);
    if (queue.length() > index)
        p.playFile(queue.at(index));
}

void Widget::repopulateList(bool preserveSelection)
{
    int index;
    if (preserveSelection)
        index = ui->listWidget->currentRow();
    ui->listWidget->clear();
    foreach (QString q, queue) {
        ui->listWidget->addItem(QFileInfo(q).completeBaseName());
    }
    ui->listWidget->setCurrentRow(preserveSelection ? index : 0);
}

void Widget::on_listWidget_doubleClicked(const QModelIndex &index)
{
    if (queue.length() > index.row())
        p.playFile(queue.at(index.row()));
}

void Widget::on_moveUpButton_clicked()
{
    int index = ui->listWidget->currentRow();
    if (index > 0) {
        queue.move(index, index - 1);
        repopulateList();
        ui->listWidget->setCurrentRow(index - 1);
        emit playlistChanged(this);
    }
}

void Widget::on_moveDownButton_clicked()
{
    int index = ui->listWidget->currentRow();
    if (index < queue.length() - 1) {
        queue.move(index, index + 1);
        repopulateList();
        ui->listWidget->setCurrentRow(index + 1);
        emit playlistChanged(this);
    }
}

void Widget::on_removeButton_clicked()
{
    int index = ui->listWidget->currentRow();
    if (index < queue.length()) { // this is probably always true...
        queue.removeAt(index);
        repopulateList();
        emit playlistChanged(this);
    }
}

void Widget::on_stopButton_clicked()
{
    p.kill();
}

void Widget::on_playButton_clicked()
{
    int index = ui->listWidget->currentRow();
    if (index >= 0 && index < queue.length())
        p.playFile(queue.at(index));
}

void Widget::on_browseButton_clicked()
{
    QStringList files = QFileDialog::getOpenFileNames(this);
    p.checkFiles(files);
    if (files.isEmpty())
        return;
    queue.append(files);
    repopulateList();
    emit playlistChanged(this);
}
