#ifndef MAPLOADER_H
#define MAPLOADER_H

#include <QString>
#include "datastructures.h"

class MapLoader
{
public:
    MapLoader();

    // 加载地图数据
    MapGraph loadMap(const QString& filePath);

    // 加载包裹数据
    QVector<Package> loadPackages(const QString& filePath);

    // 加载小车数据
    QVector<Vehicle> loadVehicles(const QString& filePath);

private:
    // 解析时间字符串为QTime
    QTime parseTime(const QString& timeStr);
};

#endif // MAPLOADER_H