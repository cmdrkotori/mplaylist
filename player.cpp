#include "player.h"
#include <QDebug>

const int QP_EXIT_NONSTARTER = 4;
const int QP_EXIT_BADFILE = 3;
const int QP_EXIT_END = 2;
const int QP_EXIT_QUIT = 1;
const int QP_EXIT_NONE = 0;

player::player(QObject *parent) :
    QObject(parent), qp(NULL)
{
}

player::~player()
{
    if (qp) {
        delete qp;
    }
}

void player::checkFiles(QStringList &list)
{
    // instead of a simple foreach loop and maintaining a seperate list for
    // returning, we shall use slightly different and more simple iterator
    // that allows us to modify the list in-place.
    QMutableStringListIterator i(list);
    while (i.hasNext())
        if (!checkFile(i.next()))
            i.remove();
}

bool player::checkFile(QString fileName)
{
    // we're using our own private process this time, because we don't want
    // to muck up the main process in case something is playing.
    QProcess check;
    check.start("mpv", QStringList() << "--no-config" << "--no-video" << "--no-audio" << fileName);
    return check.waitForFinished() && !check.readAll().contains("Failed to recognize file format.");
}

void player::playFile(QString fileName)
{
    stopFile();
    qp = new QProcess(this);
    exitState = QP_EXIT_NONE;
    connect(qp, SIGNAL(readyReadStandardOutput()), this, SLOT(process_read_output()));
    connect(qp, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(process_finished(int,QProcess::ExitStatus)));
    qp->start("mpv", QStringList() << fileName);
    playingFile = fileName;
}

void player::stopFile()
{
    if (qp) {
        qp->kill();
        qp->waitForFinished();
        delete qp;
        qp = NULL;
        playingFile.clear();
    }
}

void player::kill()
{
    if (qp) {
        qp->kill();
        playingFile.clear();
    }
}

void player::process_read_output()
{
    QString out(qp->readAllStandardOutput());
    if (out.contains("Exiting... (End of file)")) {
        exitState = QP_EXIT_END;
    }
    if (out.contains("Exiting... (Quit)")) {
        exitState = QP_EXIT_QUIT;
    }
    if (out.contains("Failed to recognize file format.")) {
        exitState = QP_EXIT_BADFILE;
    }
    if (out.contains("Exiting... (Errors when loading file)")) {
        exitState = QP_EXIT_NONSTARTER;
    }
}

void player::process_finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    (void)exitStatus;
    (void)exitCode;
    switch (exitState) {
    case 0:
        emit playbackHalted(playingFile);
        break;
    case QP_EXIT_END:
        emit playbackFinished(playingFile);
        break;
    case QP_EXIT_QUIT:
        emit playbackQuit(playingFile);
        break;
    case QP_EXIT_BADFILE:
        emit playbackBadFile(playingFile);
        break;
    case QP_EXIT_NONSTARTER:  // e.g. from an unavailable video output
        emit playbackNonstart(playingFile);
        break;
    }
}

