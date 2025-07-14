#ifndef DELIVERYMANAGER_H
#define DELIVERYMANAGER_H

#include <QVector>
#include <QString>
#include "datastructures.h"

class DeliveryManager
{
public:
    DeliveryManager();

    // 设置数据
    void setMap(const MapGraph& map) { this->map = map; }
    void setPackages(const QVector<Package>& packages) { this->packages = packages; }
    void setVehicles(const QVector<Vehicle>& vehicles) { this->vehicles = vehicles; }

    // 检查数据是否完备
    bool hasMap() const { return !map.adjacencyList.isEmpty(); }
    bool hasPackages() const { return !packages.isEmpty(); }
    bool hasVehicles() const { return !vehicles.isEmpty(); }
    bool hasData() const { return hasMap() && hasPackages() && hasVehicles(); }

    // 任务三：计算配送成本和未按时送达包裹数
    void calculateTask3(double& totalCost, int& latePackages);

    // 任务四：计算最短退货路线和时间
    double calculateTask4(const QStringList& returnLocations, QString& path);

    // 任务五：计算最小化最大边距离的路径
    double calculateTask5(int start, int end);

    // 扩展任务一：多车配送优化
    void calculateExtendTask1(double& totalCost, int& latePackages);

private:
    MapGraph map;               // 地图数据
    QVector<Package> packages;  // 包裹数据
    QVector<Vehicle> vehicles;  // 小车数据

    // 辅助函数：Dijkstra算法求最短路径
    double dijkstra(int start, int end, QVector<int>& pathNodes);

    // 辅助函数：获取地点编号
    int getLocationIndex(const QString& locationName);

    // 辅助函数：计算路径成本
    double calculatePathCost(const QVector<int>& path, double totalWeight);

    // 辅助函数：最小生成树(Kruskal算法)用于任务五
    double kruskalMinMaxEdge(int start, int end);
};

#endif // DELIVERYMANAGER_H