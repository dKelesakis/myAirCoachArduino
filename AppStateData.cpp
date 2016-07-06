#include "AppStateData.h"

QJsonObject CategoryData::serializeToJson()
{
    QJsonArray subcatArray;
    for ( int i = 0; i < subcategories->count(); i++ )
        subcatArray.append( subcategories->at( i ).toString() );

    QJsonObject jsonObj;
    jsonObj["name"] = name;
    jsonObj["subcategories"] = subcatArray;

    return jsonObj;
}


QJsonObject ImageData::serializeToJson()
{
    QJsonObject jsonObj;
    jsonObj["path"] = path;
    jsonObj["category"] = category;
    jsonObj["subcategory"] = subCategory;
    jsonObj["gender"] = gender;
    jsonObj["size"] = size;
    jsonObj["birthYear"] = birthYear;
    jsonObj["healthy"] = healthy;
    jsonObj["withBreathingDisease"] = withBreathingDisease;
    jsonObj["unknownMedicalIssue"] = unknownMedicalIssue;
    jsonObj["microphoneModel"] = microphoneModel;
    jsonObj["stretchSensorLength"] = stretchSensorLength;

    return jsonObj;
}

QJsonObject AppStateData::serializeToJson()
{
}

AppStateData::AppStateData(QObject *parent) : QObject( parent )
{
    listCategories = new QMap<int, CategoryData*>();
    listImageData  = new QList<ImageData*>();
}

const CategoryData* AppStateData::getCategoryByName( const QString name )
{
    for ( int i = 0; i < listCategories->count(); i++ )
    {
        if ( listCategories->value( i )->name == name )
            return listCategories->value( i );
    }

    return new CategoryData();
}

const ImageData* AppStateData::getImageDataByName( const QString name )
{
    for ( int i = 0; i < listImageData->count(); i++ )
    {
        if ( listImageData->at( i )->name == name )
            return listImageData->at( i );
    }

    return new ImageData();
}
