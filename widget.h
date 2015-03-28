#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QDropEvent>
#include "player.h"

/* This class keeps track of its own player and tracks a single playlist.  We
 * use an event-based approach to process playback.  Instead of marking files
 * as 'read', we remove them from the list when they are fully played.
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
    void player_playbackFinished(const QString &fileJustPlayed);
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
    player p;
    QString title;
    QStringList queue;

    void repopulateList(bool preserveSelection = true);
};

#endif // WIDGET_H
