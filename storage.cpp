#include "storage.h"
#include <QSettings>
#include <QFileInfo>
#include <QTextStream>
#include <QDirIterator>

storage::storage(QObject *parent) :
    QObject(parent)
{
    fetchConfigPath();
}

storage::cfgReturns storage::addPlaylist(const QString &title, const QStringList &entries)
{
    if (playlistAlreadyExists(title))
        return crAlreadyExists;
    return writeEntriesToFile(playlistToPath(title), entries);
}

storage::cfgReturns storage::renamePlaylist(const QString &oldTitle, const QString &newTitle)
{
    QFile file(playlistToPath(oldTitle));
    if (!file.exists())
        return crNoLongerExists;  // sneakily removed
    if (!file.rename(playlistToPath(newTitle)))
        return crRenameFailed;  // this is probably a filesystem/permission error
    return crSuccess;
}

storage::cfgReturns storage::removePlaylist(const QString &title)
{
    QFile file(playlistToPath(title));
    if (!file.exists())
        return crNoLongerExists;
    if (!file.remove())
        return crRemoveFailed;
    return crSuccess;
}

storage::cfgReturns storage::importPlaylist(const QString &filePath, const QString &title, QStringList &entries)
{
    if (!entriesFromPlaylist(filePath, entries))
        return crReadFailed;
    return addPlaylist(title, entries);
}

storage::cfgReturns storage::exportPlaylist(const QString &filePath, const QStringList &entries)
{
    return writeEntriesToFile(filePath, entries);
}

storage::cfgReturns storage::updatePlaylist(const QString &title, const QStringList &entries)
{
    return writeEntriesToFile(playlistToPath(title), entries);
}

void storage::enumPlaylists()
{
    QFileInfo info;
    QDirIterator it(configPath, QDir::Files);
    QStringList entries;
    while (it.hasNext()) {
        it.next();
        info = it.fileInfo();
        if (info.completeSuffix() != "m3u")
            continue;
        if (entriesFromPlaylist(info.absoluteFilePath(), entries))
            emit playlistFound(info.baseName(), entries);
    }
    emit finishedEnumerating();
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

storage::cfgReturns storage::writeEntriesToFile(const QString &filePath, const QStringList &entries)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text))
        return crWriteFailed;
    file.resize(0);
    QTextStream qts(&file);
    qts << entriesToM3U(entries).join('\n');
    return crSuccess;
}

bool storage::playlistAlreadyExists(const QString &title)
{
    QFileInfo info(playlistToPath(title));
    return info.exists();
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

