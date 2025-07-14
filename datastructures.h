#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H

#include <QString>
#include <QVector>
#include <QMap>
#include <QTime>

// 包裹信息结构体
struct Package {
    int id;                 // 包裹编号
    double weight;          // 包裹重量(kg)
    QString destination;    // 目的地名称
    QTime arrivalTime;      // 到达驿站时间
    QTime deadline;         // 最晚送达时间
    bool deliveredOnTime;   // 是否按时送达
};

// 快递小车信息结构体
struct Vehicle {
    int id;                 // 小车编号
    double speed;           // 速度(m/min)
    double weight;          // 自重(kg)
    double maxLoad;         // 最大载重(kg)
};

// 道路信息结构体
struct Road {
    int to;                 // 目标地点编号
    double length;          // 道路长度(m)
};

// 地图数据结构 - 使用邻接表存储图
struct MapGraph {
    QMap<int, QString> locationNames;  // 地点编号到名称的映射
    QVector<QVector<Road>> adjacencyList; // 邻接表存储道路信息
    int stationIndex;       // 菜鸟驿站地点编号(默认为1号地点)
};

#endif // DATASTRUCTURES_H