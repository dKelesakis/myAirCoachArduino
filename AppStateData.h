#ifndef APPSTATEDATA_H
#define APPSTATEDATA_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QJsonArray>
#include <QJsonObject>
#include <QVariant>

class CategoryData
{
public:
    CategoryData() { subcategories = new QList<QVariant>(); }
    QJsonObject serializeToJson();

    QString name;
    QList<QVariant> *subcategories;
};


class ImageData
{
public:
    QJsonObject serializeToJson();

    QString name;
    QString path;
    QString category;
    QString subCategory;
    QString gender;
    QString birthYear;
    int     size;
    bool    healthy;
    bool    withBreathingDisease;
    bool    unknownMedicalIssue;
    QString microphoneModel;
    QString stretchSensorLength;
};


class AppStateData : public QObject
{
    Q_OBJECT
public:
    explicit AppStateData(QObject *parent = 0);

    QJsonObject                 serializeToJson();
    const CategoryData*         getCategoryByName(  const QString name );
    const ImageData*            getImageDataByName( const QString name );

    QMap<int, CategoryData*>    *listCategories;
    QList<ImageData*>           *listImageData;

signals:

public slots:

};


#endif // APPSTATEDATA_H
