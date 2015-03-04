#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QDropEvent>
#include <QProcess>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

protected:
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);
private slots:
    void on_toolButton_4_clicked();
    void on_process_read_output();
    void on_process_finished(int exitCode, QProcess::ExitStatus exitStatus);
    void on_listWidget_doubleClicked(const QModelIndex &index);
    void on_moveUpButton_clicked();

    void on_moveDownButton_clicked();

    void on_removeButton_clicked();

    void on_stopButton_clicked();

    void on_playButton_clicked();

private:
    int exitState;
    Ui::Widget *ui;
    QProcess* qp;
    QString playingFile;
    QStringList queue;

    void playFile(QString fileName);
    void removeItemAndPlayNext();
    void repopulateList();
};

#endif // WIDGET_H
