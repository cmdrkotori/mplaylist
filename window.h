#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QString>
#include "storage.h"
#include "widget.h"

/* Because each playlist widget mostly manages it own playlist, the main
 * window ends up as a communicator between them and the storage backend.
 * It is the responsiblility of the main window to ensure that the tabs
 * properly reflect the playlists that are stored.  This is mostly done in
 * a state-based, flow-based fashion without much error correction and
 * data preservation.  So if the user deletes the config directory while
 * the program is running, it's their own stupid fault that they lost
 * their data.
 */

namespace Ui {
class Window;
}

class Window : public QWidget
{
    Q_OBJECT

public:
    explicit Window(QWidget *parent = 0);
    ~Window();

private:
    Ui::Window *ui;
    storage store;
    QString configPath;

    void addTab(const QString& title, const QStringList &queue = QStringList());
    void removePlaylist(int index);
    void saveTabOrder();
    void showFail(storage::storeReturns why, const QString &name, const QString &fileName = 0);
    void errorMessage(const QString &message);


private slots:
    void storage_playlistFound(const QString &name, const QStringList& entries);
    void storage_finishedEnumerating();
    void widget_playlistChanged(Widget *widget);

    void on_addPlaylist_clicked();
    void on_tabWidget_tabBarDoubleClicked(int index);
    void on_removePlaylist_clicked();
    void on_importPlaylist_clicked();
    void on_exportPlaylist_clicked();
    void on_buttonBox_rejected();
    void on_tabWidget_tabCloseRequested(int index);
    void tabWidget_tabBar_moved();

    void on_renameButton_clicked();
};

#endif // WINDOW_H
