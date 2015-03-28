#include "window.h"
#include "ui_window.h"
#include "widget.h"
#include <QInputDialog>
#include <QSettings>
#include <QFileInfo>
#include <QErrorMessage>
#include <QFileDialog>
#include <QMessageBox>

static const QString MSG_ALREADYEXISTS(QObject::tr("Playlist %1 already exists"));
static const QString MSG_STILLTHERE(QObject::tr("Playlist %1 could not be removed"));
static const QString MSG_UNWRITTEN(QObject::tr("Playlist %1 could not be written"));
static const QString MSG_UNRENAMED(QObject::tr("Playlist %1 could not be renamed"));
static const QString MSG_NONEXISTANT(QObject::tr("Playlist %1 no longer exists on the filesystem"));
static const QString MSG_UNREAD(QObject::tr("File %1 could not be read"));
static const QString MSG_UNEXPORTED(QObject::tr("Playlist %1 could not be written to %2"));

Window::Window(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Window)
{
    ui->setupUi(this);
    connect(&store, SIGNAL(playlistFound(QString,QStringList)), SLOT(storage_playlistFound(QString,QStringList)));
    connect(&store, SIGNAL(finishedEnumerating()), SLOT(storage_finishedEnumerating()));
    store.enumPlaylists();
}

Window::~Window()
{
    delete ui;
}

void Window::addTab(const QString &title, const QStringList &queue)
{
    Widget* w = new Widget();
    connect(w, SIGNAL(playlistChanged(Widget*)), SLOT(widget_playlistChanged(Widget*)));
    w->setTitle(title);
    if (!queue.empty())
        w->setQueue(queue);
    ui->tabWidget->addTab(w, title);
}

void Window::removePlaylist(int index)
{
    if (index < 0)
        return;

    Widget *w = reinterpret_cast<Widget*>(ui->tabWidget->widget(index));
    storage::storeReturns ret = store.removePlaylist(w->getTitle());
    if (ret != storage::srSuccess) {
        showFail(ret, w->getTitle());
        return;
    }
    ui->tabWidget->removeTab(index);
}

void Window::showFail(storage::storeReturns why, const QString &name, const QString &fileName)
{
    if (why == storage::srAlreadyExists)
        errorMessage(MSG_ALREADYEXISTS.arg(name));
    if (why == storage::srWriteFailed) {
        errorMessage(name.isEmpty()
                     ? MSG_UNEXPORTED.arg(name, fileName)
                     : MSG_UNWRITTEN.arg(name));
    }
    if (why == storage::srNoLongerExists)
        // ideally, we would tie a playlist to a file, and remove it when it
        // is tampered with by hooking into inotify.
        errorMessage(MSG_NONEXISTANT.arg(name));
    if (why == storage::srRenameFailed)
        errorMessage(MSG_UNRENAMED.arg(name));
    if (why == storage::srRemoveFailed)
        errorMessage(MSG_STILLTHERE.arg(name));
    if (why == storage::srReadFailed)
        errorMessage(MSG_UNREAD.arg(fileName));

}

void Window::errorMessage(const QString &message)
{
    QMessageBox::warning(this, tr("Something bad happened"), message);
}

void Window::storage_playlistFound(const QString &name, const QStringList &entries)
{
    addTab(name, entries);
}

void Window::storage_finishedEnumerating()
{
    if (ui->tabWidget->count() == 0) {
        on_addPlaylist_clicked();
    }
}

void Window::widget_playlistChanged(Widget *widget)
{
    storage::storeReturns ret = store.updatePlaylist(widget->getTitle(), widget->getQueue());
    if (ret != storage::srSuccess)
        showFail(ret, widget->getTitle());
}

void Window::on_addPlaylist_clicked()
{
    QString name = tr("empty playlist");
    QStringList queue;
    storage::storeReturns ret = store.addPlaylist(name, queue);
    if (ret != storage::srSuccess) {
        showFail(ret, name);
        return;
    }
    addTab(name, queue);
}

void Window::on_tabWidget_tabBarDoubleClicked(int index)
{
    if (index < 0) {
        on_addPlaylist_clicked();
    }

    QString oldText = ui->tabWidget->tabText(index);
    QString newText;
    bool ok;
    newText = QInputDialog::getText(this, tr("Rename playlist"), tr("Title"),
                                 QLineEdit::Normal, oldText , &ok);
    if (!ok || newText.isEmpty())
        return;
    storage::storeReturns ret = store.renamePlaylist(oldText, newText);
    if (ret == storage::srSuccess) {
        ui->tabWidget->setTabText(index, newText);
        reinterpret_cast<Widget*>(ui->tabWidget->currentWidget())->setTitle(newText);
        return;
    }
}

void Window::on_removePlaylist_clicked()
{
    removePlaylist(ui->tabWidget->currentIndex());
}

void Window::on_importPlaylist_clicked()
{
    bool ok;
    QString title = QInputDialog::getText(this, tr("New playlist"), tr("Title"),
                                          QLineEdit::Normal, QString(), &ok);
    if (!ok || title.isEmpty())
        return;

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    QDir::homePath(),
                                                    tr("Playlists (*.m3u)"),
                                                    0, QFileDialog::HideNameFilterDetails);
    if (fileName.isEmpty())
        return;

    QStringList entries;
    storage::storeReturns ret = store.importPlaylist(fileName, title, entries);
    if (ret != storage::srSuccess) {
        showFail(ret, title, fileName);
        return;
    }
    addTab(title, entries);
}

void Window::on_exportPlaylist_clicked()
{
    int index = ui->tabWidget->currentIndex();
    if (index < 0)
        return;
    Widget *w = reinterpret_cast<Widget*>(ui->tabWidget->currentWidget());
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                                                    QDir::homePath(),
                                                    tr("Playlist (*.m3u)"),
                                                    0, QFileDialog::HideNameFilterDetails);
    if (fileName.isEmpty())
        return;

    storage::storeReturns ret = store.exportPlaylist(fileName, w->getQueue());
    showFail(ret, w->getTitle(), fileName);
}

void Window::on_buttonBox_rejected()
{
    close();
}

void Window::on_tabWidget_tabCloseRequested(int index)
{
    removePlaylist(index);
}
