#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QDropEvent>
#include <QProcess>

/* This class keeps track of its own mpv process and tracks a single
 * playlist.  We use a state-based approach to program exit, to determine
 * what to do next.  A way to improve the situation is to spin off the mpv
 * code to a player module.  But hey it works and literally who cares?
 */

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

    void setQueue(const QStringList& queue);
    QStringList getQueue();
    void setTitle(const QString& title);
    QString getTitle();

signals:
    void playlistChanged(Widget *widget);

protected:
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);
private slots:
    void process_read_output();
    void process_finished(int exitCode, QProcess::ExitStatus exitStatus);

    void on_listWidget_doubleClicked(const QModelIndex &index);
    void on_moveUpButton_clicked();
    void on_moveDownButton_clicked();
    void on_removeButton_clicked();
    void on_stopButton_clicked();
    void on_playButton_clicked();
    void on_browseButton_clicked();

private:
    int exitState;
    Ui::Widget *ui;
    QProcess* qp;
    QString playingFile;
    QString title;
    QStringList queue;

    void playFile(QString fileName);
    void stopFile();
    void removeItemAndPlayNext();
    void repopulateList(bool preserveSelection = true);
    void checkFiles(QStringList &list);
    bool checkFile(QString fileName);
};

#endif // WIDGET_H
