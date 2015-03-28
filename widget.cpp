#include "widget.h"
#include "ui_widget.h"
#include <qdrag.h>
#include <qmimedata.h>
#include <QDebug>
#include <QProcess>
#include <QFileDialog>
#include <QFileInfo>

/* Instead of executing mpv each time, we COULD hook up to libmpv and run
 * stuff in a window/widget for the same effect.
 *
 * Advantage: No parsing of program output which could change at any time.
 * Disadvantage: mplaylist is not a media player, it is a playlist manager.
 * I'd have to manage another window, and write probably two module/classes to
 * do it.  On the other hand, I really want to keep this simple.
 */

const int QP_EXIT_BADFILE = 3;
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

void Widget::setQueue(const QStringList &queue)
{
    stopFile();
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
        if (checkFile(fileName)) {
            queue.append(fileName);
            changed = true;
        }
    }
    if (changed) {
        repopulateList();
        emit playlistChanged(this);
    }
}

void Widget::process_read_output()
{
    QString out(qp->readAllStandardOutput());
    if (out.contains("Exiting... (End of file)")) {
        exitState = QP_EXIT_END;
    }
    if (out.contains("Exiting... (Quit)")) {
        exitState = QP_EXIT_QUIT;
    }
    if (out.contains("Exiting... (Errors when loading file)")) {
        exitState = QP_EXIT_BADFILE;
    }
}

void Widget::process_finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    (void)exitStatus;
    (void)exitCode;
    // TODO: Replace these debug statements with informative signals.
    switch (exitState) {
    case 0:
        qDebug() << "Abnormal termination";
        break;
    case QP_EXIT_END:
        qDebug() << "End of file read";
        removeItemAndPlayNext();
        break;
    case QP_EXIT_QUIT:
        qDebug() << "User quit";
        break;
    case QP_EXIT_BADFILE:
        qDebug() << "Bad file";
        break;
    }
}

void Widget::playFile(QString fileName)
{
    stopFile();
    qp = new QProcess(this);
    exitState = QP_EXIT_NONE;
    connect(qp, SIGNAL(readyReadStandardOutput()), this, SLOT(process_read_output()));
    connect(qp, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(process_finished(int,QProcess::ExitStatus)));
    qp->start("mpv", QStringList() << fileName);
    playingFile = fileName;
}

void Widget::stopFile()
{
    if (qp) {
        qp->kill();
        qp->waitForFinished();
        delete qp;
        qp = NULL;
    }
}

void Widget::removeItemAndPlayNext()
{
    int index = ui->listWidget->currentRow();
    queue.removeAll(playingFile);
    repopulateList();
    emit playlistChanged(this);
    if (queue.length() > index)
        playFile(queue.at(index));
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

void Widget::checkFiles(QStringList &list)
{
    // instead of a simple foreach loop and maintaining a seperate list for
    // returning, we shall use slightly different and more simple iterator
    // that allows us to modify the list in-place.
    QMutableStringListIterator i(list);
    while (i.hasNext())
        if (!checkFile(i.next()))
            i.remove();
}

bool Widget::checkFile(QString fileName)
{
    // we're using our own private process this time, because we don't want
    // to muck up the main process in case something is playing.
    QProcess check;
    check.start("mpv", QStringList() << "--no-config" << "--no-video" << "--no-audio" << fileName);
    return check.waitForFinished() && !check.readAll().contains("Failed to recognize file format.");
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
    if (qp) {
        qp->kill();
    }
}

void Widget::on_playButton_clicked()
{
    int index = ui->listWidget->currentRow();
    if (index >= 0 && index < queue.length())
        playFile(queue.at(index));
}

void Widget::on_browseButton_clicked()
{
    QStringList files = QFileDialog::getOpenFileNames(this);
    checkFiles(files);
    if (files.isEmpty())
        return;
    queue.append(files);
    repopulateList();
    emit playlistChanged(this);
}
