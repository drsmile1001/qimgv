#pragma once
#include <QObject>
#include <QHash>
#include <QString>
#include <QTimer>

class StarRating : public QObject {
    Q_OBJECT
public:
    explicit StarRating(QObject* parent = nullptr);

    void setRating(const QString& filePath, int stars);
    int getRating(const QString& filePath) const;

    bool load();
    bool save() const;

private:
    QString storagePath() const;
    QHash<QString, int> ratings;
    mutable QTimer* saveTimer;
    void scheduleSave();
};
