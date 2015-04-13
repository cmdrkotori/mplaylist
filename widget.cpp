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
    // Because repopulating a list is relatively expensive, (we do things very
    // simply if we at all can,) we only update our gui if we really need to.
    // Though not really, average use-cases aren't expensive unless you're
    // dropping a copious amount of files all at once.  However, now we are
    // checking if a file is valid, so this isn't so quick anymore.  If I get
    // complaints about freezes, I'll spin this into a thread.
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
    // When playback is finished, the item is removed and playback proceeds
    // on the next item, if there is one.  I don't want to store positions at
    // present, it would unduly complicate the simple storage mechanism all
    // for the purpose of storing one index into a playlist.  Playlists may
    // change when the program isn't running anyway, so don't bother.
    int index = ui->listWidget->currentRow();
    queue.removeAll(fileJustPlayed);
    repopulateList();
    emit playlistChanged(this);
    if (queue.length() > index)
        p.playFile(queue.at(index));
}

void Widget::repopulateList(bool preserveSelection)
{
    // I am not using a bunch of model-view classes just to do this "more
    // correctly" -- that would complicate the code and imo head towards
    // unreadable spaghetti territory.  Instead, we simply reload the list
    // widget whenever the queue changes.
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
    if (index < queue.length()) { // this is probably always true, except when it's not.
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
