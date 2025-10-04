#include "starrating.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDir>

StarRating::StarRating(QObject* parent) : QObject(parent) {
    saveTimer = new QTimer(this);
    saveTimer->setSingleShot(true);
    connect(saveTimer, &QTimer::timeout, this, [this]() {
        this->save();
    });
}

void StarRating::setRating(const QString& filePath, int stars) {
    ratings[filePath] = stars;
    scheduleSave();
}

int StarRating::getRating(const QString& filePath) const {
    return ratings.value(filePath, 0);
}

bool StarRating::load() {
    QFile f(storagePath());
    if (!f.open(QIODevice::ReadOnly)) return false;

    auto doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isObject()) return false;

    auto obj = doc.object();
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        ratings[it.key()] = it.value().toInt();
    }
    return true;
}

bool StarRating::save() const {
    QJsonObject obj;
    for (auto it = ratings.begin(); it != ratings.end(); ++it) {
        obj[it.key()] = it.value();
    }
    QFile f(storagePath());
    if (!f.open(QIODevice::WriteOnly)) return false;
    f.write(QJsonDocument(obj).toJson());
    return true;
}

QString StarRating::storagePath() const {
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (configDir.isEmpty()) {
        configDir = QDir::homePath();  // fallback
    }
    QDir().mkpath(configDir);  // 確保目錄存在
    return configDir + "/star_ratings.json";
}

void StarRating::scheduleSave() {
    // 如果已經在倒數，就重設 timer
    saveTimer->start(2000); // 2 秒內沒新事件才真正寫檔
}
