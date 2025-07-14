#include "deliverymanager.h"
#include <queue>
#include <algorithm>
#include <limits>
#include <QDebug>
#include <QTime>

DeliveryManager::DeliveryManager()
{
}

void DeliveryManager::calculateTask3(double& totalCost, int& latePackages)
{
    totalCost = 0;
    latePackages = 0;

    if (!hasData()) return;

    // 简化实现：假设每辆车负责一个区域的包裹
    // 实际应用中应实现更复杂的路径规划算法
    for (const Vehicle& vehicle : vehicles) {
        // 为每辆车分配包裹（简化：按目的地分组）
        QMap<QString, QVector<Package>> destPackages;
        for (const Package& pkg : packages) {
            destPackages[pkg.destination].append(pkg);
        }

        // 为每个目的地计算路径
        for (auto it = destPackages.begin(); it != destPackages.end(); ++it) {
            const QString& dest = it.key();
            const QVector<Package>& destPkgs = it.value();

            int destIndex = getLocationIndex(dest);
            if (destIndex == -1) continue;

            // 计算从驿站到目的地的最短路径
            QVector<int> pathNodes;
            double distance = dijkstra(map.stationIndex, destIndex, pathNodes);
            if (distance <= 0) continue;

            // 计算包裹总重量
            double totalWeight = 0;
            for (const Package& pkg : destPkgs) {
                totalWeight += pkg.weight;
            }

            // 计算配送成本：趟数×路程段×本段路程长度×此段路程上的包裹总重
            // 简化：假设一趟配送一个目的地
            double cost = 1 * pathNodes.size() * distance * totalWeight;
            totalCost += cost;

            // 计算配送时间并检查是否按时送达
            double time = distance / vehicle.speed;
            QTime deliveryTime = packages[0].arrivalTime.addSecs(static_cast<int>(time * 60));
            for (const Package& pkg : destPkgs) {
                if (deliveryTime > pkg.deadline) {
                    latePackages++;
                }
            }
        }
    }
}

double DeliveryManager::calculateTask4(const QStringList& returnLocations, QString& path)
{
    path.clear();
    if (!hasMap() || returnLocations.isEmpty()) return 0;

    // 收集所有退货地点的编号
    QVector<int> locations;
    for (const QString& locName : returnLocations) {
        int index = getLocationIndex(locName);
        if (index != -1) {
            locations.append(index);
        }
    }

    // 如果没有有效的地点，返回0
    if (locations.isEmpty()) return 0;

    // 简化实现：使用TSP近似算法（最近邻点）
    QVector<int> route;
    int current = map.stationIndex;
    QVector<bool> visited(locations.size(), false);
    int visitedCount = 0;

    // 添加起点
    route.append(current);

    // 构建路径
    while (visitedCount < locations.size()) {
        double minDist = std::numeric_limits<double>::max();
        int nextIndex = -1;
        int nextLoc = -1;

        // 找到最近的未访问地点
        for (int i = 0; i < locations.size(); ++i) {
            if (!visited[i]) {
                QVector<int> tempPath;
                double dist = dijkstra(current, locations[i], tempPath);
                if (dist < minDist) {
                    minDist = dist;
                    nextIndex = i;
                    nextLoc = locations[i];
                }
            }
        }

        if (nextIndex == -1) break;

        visited[nextIndex] = true;
        visitedCount++;
        current = nextLoc;
        route.append(current);
    }

    // 返回驿站
    route.append(map.stationIndex);

    // 计算总距离和时间
    double totalDistance = 0;
    for (int i = 0; i < route.size() - 1; ++i) {
        QVector<int> tempPath;
        totalDistance += dijkstra(route[i], route[i+1], tempPath);
    }

    // 计算时间（假设使用第一辆车的速度）
    double speed = vehicles.isEmpty() ? 100 : vehicles[0].speed;
    double time = totalDistance / speed;

    // 构建路径字符串
    for (int i = 0; i < route.size(); ++i) {
        if (i > 0) path += " -> ";
        path += map.locationNames.value(route[i], QString::number(route[i]));
    }

    return time;
}

double DeliveryManager::calculateTask5(int start, int end)
{
    if (!hasMap() || start < 1 || end < 1 || start >= map.adjacencyList.size() || end >= map.adjacencyList.size()) {
        return -1;
    }

    // 使用Kruskal算法找到最小生成树，然后找到路径中最大边
    return kruskalMinMaxEdge(start, end);
}

void DeliveryManager::calculateExtendTask1(double& totalCost, int& latePackages)
{
    totalCost = 0;
    latePackages = 0;

    if (!hasData() || vehicles.size() < 2) {
        // 如果车辆不足2辆，使用任务三的算法
        calculateTask3(totalCost, latePackages);
        return;
    }

    // 简化实现：将包裹平均分配给两辆车
    QVector<Package> vehicle1Pkgs, vehicle2Pkgs;
    for (int i = 0; i < packages.size(); ++i) {
        if (i % 2 == 0) {
            vehicle1Pkgs.append(packages[i]);
        } else {
            vehicle2Pkgs.append(packages[i]);
        }
    }

    // 为每辆车计算成本
    double cost1 = 0, cost2 = 0;
    int late1 = 0, late2 = 0;

    QVector<Package> originalPackages = packages;

    packages = vehicle1Pkgs;
    calculateTask3(cost1, late1);

    packages = vehicle2Pkgs;
    calculateTask3(cost2, late2);

    // 恢复原始包裹数据
    packages = originalPackages;

    totalCost = cost1 + cost2;
    latePackages = late1 + late2;
}

double DeliveryManager::dijkstra(int start, int end, QVector<int>& pathNodes)
{
    pathNodes.clear();
    if (start < 1 || end < 1 || start >= map.adjacencyList.size() || end >= map.adjacencyList.size()) {
        return -1;
    }

    // 距离数组和前驱节点数组
    QVector<double> dist(map.adjacencyList.size(), std::numeric_limits<double>::max());
    QVector<int> prev(map.adjacencyList.size(), -1);
    QVector<bool> visited(map.adjacencyList.size(), false);

    // 优先队列：(距离, 节点)
    std::priority_queue<std::pair<double, int>, std::vector<std::pair<double, int>>, std::greater<std::pair<double, int>>> pq;

    dist[start] = 0;
    pq.push({0, start});

    while (!pq.empty()) {
        int u = pq.top().second;
        pq.pop();

        if (visited[u]) continue;
        if (u == end) break;

        visited[u] = true;

        for (const Road& road : map.adjacencyList[u]) {
            int v = road.to;
            double w = road.length;

            if (dist[v] > dist[u] + w) {
                dist[v] = dist[u] + w;
                prev[v] = u;
                pq.push({dist[v], v});
            }
        }
    }

    // 重建路径
    if (prev[end] == -1 && start != end) {
        return -1; // 无路径
    }

    for (int v = end; v != -1; v = prev[v]) {
        pathNodes.push_back(v);
    }
    std::reverse(pathNodes.begin(), pathNodes.end());

    return dist[end];
}

int DeliveryManager::getLocationIndex(const QString& locationName)
{
    for (auto it = map.locationNames.begin(); it != map.locationNames.end(); ++it) {
        if (it.value() == locationName) {
            return it.key();
        }
    }
    return -1;
}

double DeliveryManager::calculatePathCost(const QVector<int>& path, double totalWeight)
{
    if (path.size() < 2) return 0;

    double totalCost = 0;
    for (int i = 0; i < path.size() - 1; ++i) {
        QVector<int> segmentPath;
        double distance = dijkstra(path[i], path[i+1], segmentPath);
        totalCost += distance * totalWeight * segmentPath.size();
    }

    return totalCost;
}

double DeliveryManager::kruskalMinMaxEdge(int start, int end)
{
    // 边结构体
    struct Edge {
        int u, v;
        double weight;
        Edge(int u_, int v_, double w_) : u(u_), v(v_), weight(w_) {}
        bool operator<(const Edge& other) const { return weight < other.weight; }
    };

    // 收集所有边
    QVector<Edge> edges;
    for (int u = 1; u < map.adjacencyList.size(); ++u) {
        for (const Road& road : map.adjacencyList[u]) {
            if (u < road.to) { // 避免重复
                edges.emplace_back(u, road.to, road.length);
            }
        }
    }

    // 排序边
    std::sort(edges.begin(), edges.end());

    // 并查集
    QVector<int> parent(map.adjacencyList.size());
    for (int i = 0; i < parent.size(); ++i) parent[i] = i;

    auto find = [&](auto&& self, int x) -> int {
        if (parent[x] != x) parent[x] = self(self, parent[x]);
        return parent[x];
    };

    auto unite = [&](int x, int y) {
        x = find(find, x);
        y = find(find, y);
        if (x != y) parent[y] = x;
    };

    // Kruskal算法构建最小生成树，直到start和end连通
    for (const Edge& e : edges) {
        unite(e.u, e.v);
        if (find(find, start) == find(find, end)) {
            return e.weight;
        }
    }

    return -1; // 不连通（题目保证连通，所以不会执行到这里）
}