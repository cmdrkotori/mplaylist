#ifndef STORAGE_H
#define STORAGE_H

#include <QObject>
#include <QStringList>

/* Note that our implementation of a storage backend does not try to keep a
 * in-memory copy of our playlists and sync with something like a save
 * function.  Instead, we work with the information associated with each
 * playlist widget to store our file lists.  The purpose of this class is not
 * to hold anything in memory, but to communicate with the filesystem.  One
 * could say that the filesystem is our database.  The advantage of this
 * approach is the system can literally crash and we'll still know where we
 * are up to next time.
 *
 * Our objects are simply m3u playlists stored in the application's config
 * directory.  The title of each playlist in the gui is the name of each file.
 * We do store the tab order in an text file and attempt to restore it,
 * however.
 */

class storage : public QObject
{
    Q_OBJECT
public:
    explicit storage(QObject *parent = 0);

    /* This is an absurd amount of error detection.  We could display error
     * dialogs from this class, but those belong in the ui/view classes.
     */
    enum storeReturns { srSuccess, srAlreadyExists, srNoLongerExists,
                        srWriteFailed, srReadFailed, srRenameFailed,
                        srRemoveFailed };

    storeReturns addPlaylist(const QString &title, const QStringList &entries = QStringList());
    storeReturns renamePlaylist(const QString &oldTitle, const QString &newTitle);
    storeReturns removePlaylist(const QString &title);
    // Note the use of the non-const parameter.  Instead of passing this back
    // on the stack, we modify what was passed to us.
    storeReturns importPlaylist(const QString &filePath, const QString &title, QStringList &entries);
    storeReturns exportPlaylist(const QString &filePath, const QStringList &entries);
    storeReturns updatePlaylist(const QString &title, const QStringList &entries);
    void enumPlaylists();
    void saveTabs(const QStringList &tabs);

private:
    /* Literally the only reason why this is a class and not a bunch of static
     * functions, are that we need to associate a config path with our
     * functions.  We could make a C-style storage::init() function and pass a
     * window pointer to enumPlaylists, but I find init()-like functions
     * distateful because we could forget to initalize the module; it is far
     * better to guarantee that initialization has taken place by requiring
     * instantiation.
     *
     * >W-What if I have two copies in different places?
     * What on Earth are you doing.
     */
    QString configPath;
    void fetchConfigPath();

    QStringList entriesFromPlaylist(const QString &filePath);
    QStringList entriesToM3U(const QStringList &entries);
    QStringList entriesFromM3U(QStringList);
    QString playlistToPath(const QString &title);
    bool entriesFromPlaylist(const QString &filePath, QStringList &entries);
    storeReturns writeEntriesToFile(const QString &filePath, const QStringList &entries);
    bool playlistAlreadyExists(const QString &title);

    /* Because QSettings sorts string lists upon read, we need our own storage
     * functions for this.
     */
    QStringList readTabs();
    void writeTabs(const QStringList &tabs);


signals:
    void playlistFound(const QString &name, const QStringList& entries);
    void finishedEnumerating();

public slots:

};

#endif // STORAGE_H
