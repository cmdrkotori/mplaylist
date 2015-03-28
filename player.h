#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <QProcess>

/* The advantage of spinning off mpv-specific functions to a seperate module
 * is that you can use different players so long as they support the same
 * operations.  Of course, this class would become virtual, with the mpv-
 * specific code ending up in a subclass.
 *
 * You could even anticipate switching between different classes depending
 * upon the file type (e.g. feh, gwenview, mpd, paplay, etc.), or even
 * configuring mplaylist to use an encoder instead of a player.  In which case
 * this might be extended to a sort of host class for these use-cases.
 *
 * TODO: Instead of executing mpv each time, hook up to libmpv and run stuff
 * in a window/widget for the same effect.  It's not as simple a spawning a
 * process, but it is the Right(tm) thing to do.
 *
 * WHY: Currently we use an state-based approach to program exit; it's messy
 * and depends upon arbitrary console output much the same way slave mode did.
 */

class player : public QObject
{
    Q_OBJECT
public:
    explicit player(QObject *parent = 0);
    ~player();

    void checkFiles(QStringList &list);
    bool checkFile(QString fileName);

    void playFile(QString fileName);
    void stopFile();

    void kill();

private:
    QProcess *qp;
    int exitState;
    QString playingFile;

signals:
    void playbackFinished(const QString &fileJustPlayed);
    void playbackHalted(const QString &fileJustPlayed);
    void playbackQuit(const QString &fileJustPlayed);
    void playbackBadFile(const QString &fileNotPlayed);
    void playbackNonstart(const QString &fileNotPlayed);

public slots:

private slots:
    void process_read_output();
    void process_finished(int exitCode, QProcess::ExitStatus exitStatus);

};

#endif // PLAYER_H
