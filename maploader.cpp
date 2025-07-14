#include "maploader.h"
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QDebug>

MapLoader::MapLoader()
{
}

MapGraph MapLoader::loadMap(const QString& filePath)
{
    MapGraph map;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法打开地图文件: " << filePath;
        return map;
    }

    QTextStream in(&file);
    QString line = in.readLine();
    if (line.isEmpty()) return map;

    // 解析地点数量和道路数量
    QStringList firstLine = line.split(QRegExp("\s+"), Qt::SkipEmptyParts);
    if (firstLine.size() < 2) return map;

    int n = firstLine[0].toInt(); // 地点数量
    int m = firstLine[1].toInt(); // 道路数量

    // 初始化邻接表 (地点编号从1开始)
    map.adjacencyList.resize(n + 1);
    map.stationIndex = 1; // 默认1号地点为菜鸟驿站

    // 读取地点名称
    for (int i = 1; i <= n; ++i) {
        line = in.readLine().trimmed();
        if (line.isEmpty()) continue;
        map.locationNames[i] = line;
    }

    // 读取道路信息
    for (int i = 0; i < m; ++i) {
        line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList roadData = line.split(QRegExp("\s+"), Qt::SkipEmptyParts);
        if (roadData.size() < 3) continue;

        int u = roadData[0].toInt();
        int v = roadData[1].toInt();
        double w = roadData[2].toDouble();

        // 添加双向道路
        map.adjacencyList[u].append({v, w});
        map.adjacencyList[v].append({u, w});
    }

    file.close();
    return map;
}

QVector<Package> MapLoader::loadPackages(const QString& filePath)
{
    QVector<Package> packages;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法打开包裹文件: " << filePath;
        return packages;
    }

    QTextStream in(&file);
    QString line = in.readLine();
    if (line.isEmpty()) return packages;

    int numCar = line.toInt(); // 快递小车数量(任务三数据格式)
    // 跳过小车数据行
    for (int i = 0; i < numCar; ++i) {
        in.readLine();
    }

    // 读取包裹数量
    line = in.readLine();
    if (line.isEmpty()) return packages;
    int k = line.toInt();

    // 读取包裹信息
    for (int i = 0; i < k; ++i) {
        line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList pkgData = line.split(QRegExp("\s+"), Qt::SkipEmptyParts);
        if (pkgData.size() < 5) continue;

        Package pkg;
        pkg.id = pkgData[0].toInt();
        pkg.weight = pkgData[1].toDouble();
        pkg.destination = pkgData[2];
        pkg.arrivalTime = parseTime(pkgData[3]);
        pkg.deadline = parseTime(pkgData[4]);
        pkg.deliveredOnTime = false; // 初始化为未送达

        packages.append(pkg);
    }

    file.close();
    return packages;
}

QVector<Vehicle> MapLoader::loadVehicles(const QString& filePath)
{
    QVector<Vehicle> vehicles;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法打开小车文件: " << filePath;
        return vehicles;
    }

    QTextStream in(&file);
    QString line = in.readLine();
    if (line.isEmpty()) return vehicles;

    int numCar = line.toInt(); // 快递小车数量

    // 读取每个小车信息
    for (int i = 0; i < numCar; ++i) {
        line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList vehData = line.split(QRegExp("\s+"), Qt::SkipEmptyParts);
        if (vehData.size() < 3) continue;

        Vehicle veh;
        veh.id = i + 1; // 小车编号从1开始
        veh.speed = vehData[0].toDouble();
        veh.weight = vehData[1].toDouble();
        veh.maxLoad = vehData[2].toDouble();

        vehicles.append(veh);
    }

    file.close();
    return vehicles;
}

QTime MapLoader::parseTime(const QString& timeStr)
{
    // 支持多种时间格式: 整数分钟数或HH:MM格式
    if (timeStr.contains(":")) {
        return QTime::fromString(timeStr, "HH:mm");
    } else {
        // 假设是分钟数
        int minutes = timeStr.toInt();
        return QTime(0, 0).addSecs(minutes * 60);
    }
}