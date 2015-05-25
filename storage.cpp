#include "storage.h"
#include <QSettings>
#include <QFileInfo>
#include <QTextStream>
#include <QDirIterator>

static const QString TAB_FILE("tabs.txt");


storage::storage(QObject *parent) :
    QObject(parent)
{
    fetchConfigPath();
}

storage::storeReturns storage::addPlaylist(const QString &title, const QStringList &entries)
{
    if (playlistAlreadyExists(title))
        return srAlreadyExists;
    return writeEntriesToFile(playlistToPath(title), entries);
}

storage::storeReturns storage::renamePlaylist(const QString &oldTitle, const QString &newTitle)
{
    QFile file(playlistToPath(oldTitle));
    if (!file.exists())
        return srNoLongerExists;  // sneakily removed by the user.  bad user!
    if (!file.rename(playlistToPath(newTitle)))
        return srRenameFailed;  // this is probably a filesystem/permission error
    return srSuccess;
}

storage::storeReturns storage::removePlaylist(const QString &title)
{
    QFile file(playlistToPath(title));
    if (!file.exists())
        return srNoLongerExists;
    if (!file.remove())
        return srRemoveFailed;
    return srSuccess;
}

storage::storeReturns storage::importPlaylist(const QString &filePath, const QString &title, QStringList &entries)
{
    if (!entriesFromPlaylist(filePath, entries))
        return srReadFailed;
    return addPlaylist(title, entries);
}

storage::storeReturns storage::exportPlaylist(const QString &filePath, const QStringList &entries)
{
    return writeEntriesToFile(filePath, entries);
}

storage::storeReturns storage::updatePlaylist(const QString &title, const QStringList &entries)
{
    return writeEntriesToFile(playlistToPath(title), entries);
}

void storage::enumPlaylists()
{
    // We start with two lists: whats on the disk and the tab order from last
    // time.  So we merge the two, and load whatever playlists we can find.
    QStringList allLists;
    QStringList savedLists = readTabs();
    foreach (const QString &s, savedLists) {
        allLists.append(s + ".m3u");
    }
    QStringList storedLists = QDir(configPath).entryList(QStringList() << "*.m3u");
    allLists.append(storedLists);
    allLists.removeDuplicates();

    QDir configDir(configPath);
    QFileInfo info;
    QStringList entries;
    foreach (const QString &s, allLists) {
        info.setFile(configDir.filePath(s));
        if (entriesFromPlaylist(info.absoluteFilePath(), entries))
            emit playlistFound(info.completeBaseName(), entries);
    }
    emit finishedEnumerating();
}

void storage::saveTabs(const QStringList &tabs)
{
    writeTabs(tabs);
}

void storage::fetchConfigPath()
{
    QSettings::setDefaultFormat(QSettings::IniFormat);
    configPath = QFileInfo(QSettings().fileName()).absolutePath() + "/";
    QDir().mkpath(configPath);
}

bool storage::entriesFromPlaylist(const QString &filePath, QStringList &entries)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;
    QTextStream qts(&file);
    entries = entriesFromM3U(qts.readAll().split('\n'));
    return true;
}

storage::storeReturns storage::writeEntriesToFile(const QString &filePath, const QStringList &entries)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text))
        return srWriteFailed;
    file.resize(0);
    QTextStream qts(&file);
    qts << entriesToM3U(entries).join('\n');
    return srSuccess;
}

bool storage::playlistAlreadyExists(const QString &title)
{
    QFileInfo info(playlistToPath(title));
    return info.exists();
}

QStringList storage::readTabs()
{
    QFile file(QDir(configPath).absoluteFilePath(TAB_FILE));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return QStringList();
    return QTextStream(&file).readAll().split('\n');
}

void storage::writeTabs(const QStringList &tabs)
{
    QFile file(QDir(configPath).absoluteFilePath(TAB_FILE));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    QTextStream(&file) << tabs.join('\n');
}

QStringList storage::entriesToM3U(const QStringList &entries)
{
    return QStringList() << "#EXTM3U" << entries;
}

QStringList storage::entriesFromM3U(QStringList M3U)
{
    QStringList items;
    items.clear();
    foreach(QString s, M3U) {
        s = s.trimmed();
        if (s.isEmpty() || s[0] == '#')
            continue;
        if (QFileInfo(s).exists())
           items.append(s);
    }
    return items;
}

QString storage::playlistToPath(const QString &title)
{
    return QString("%1%2.m3u").arg(configPath,title);
}

