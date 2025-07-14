#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "maploader.h"
#include "deliverymanager.h"
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QHeaderView>
#include <QTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , mapLoader(new MapLoader())
    , deliveryManager(new DeliveryManager())
    , mapScene(new QGraphicsScene(this))
{
    ui->setupUi(this);
    setWindowTitle("校园快递配送系统");

    // 初始化地图显示区域
    ui->graphicsView->setScene(mapScene);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);

    // 初始化表格控件
    ui->packagesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->vehiclesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // 设置表格列标题
    QStringList packageHeaders = {"包裹编号", "重量(kg)", "目的地", "到达时间", "最晚时间", "是否按时送达"};
    ui->packagesTable->setColumnCount(packageHeaders.size());
    ui->packagesTable->setHorizontalHeaderLabels(packageHeaders);

    QStringList vehicleHeaders = {"小车编号", "速度(m/min)", "自重(kg)", "最大载重(kg)"};
    ui->vehiclesTable->setColumnCount(vehicleHeaders.size());
    ui->vehiclesTable->setHorizontalHeaderLabels(vehicleHeaders);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete mapLoader;
    delete deliveryManager;
}

void MainWindow::on_actionImportMap_triggered()
{
    QString filePath = QFileDialog::getOpenFileName(this, "导入地图数据", ".", "文本文件 (*.txt)");
    if (filePath.isEmpty()) return;

    MapGraph map = mapLoader->loadMap(filePath);
    if (map.adjacencyList.isEmpty()) {
        QMessageBox::warning(this, "错误", "地图数据导入失败!");
        return;
    }

    deliveryManager->setMap(map);
    displayMap(map);
    QMessageBox::information(this, "成功", "地图数据导入成功!");
}

void MainWindow::on_actionImportPackages_triggered()
{
    QString filePath = QFileDialog::getOpenFileName(this, "导入包裹数据", ".", "文本文件 (*.txt)");
    if (filePath.isEmpty()) return;

    QVector<Package> packages = mapLoader->loadPackages(filePath);
    if (packages.isEmpty()) {
        QMessageBox::warning(this, "错误", "包裹数据导入失败!");
        return;
    }

    deliveryManager->setPackages(packages);
    displayPackages(packages);
    QMessageBox::information(this, "成功", "包裹数据导入成功!");
}

void MainWindow::on_actionImportVehicles_triggered()
{
    QString filePath = QFileDialog::getOpenFileName(this, "导入小车数据", ".", "文本文件 (*.txt)");
    if (filePath.isEmpty()) return;

    QVector<Vehicle> vehicles = mapLoader->loadVehicles(filePath);
    if (vehicles.isEmpty()) {
        QMessageBox::warning(this, "错误", "小车数据导入失败!");
        return;
    }

    deliveryManager->setVehicles(vehicles);
    displayVehicles(vehicles);
    QMessageBox::information(this, "成功", "小车数据导入成功!");
}

void MainWindow::displayPackages(const QVector<Package>& packages)
{
    ui->packagesTable->setRowCount(packages.size());
    for (int i = 0; i < packages.size(); ++i) {
        const Package& pkg = packages[i];
        ui->packagesTable->setItem(i, 0, new QTableWidgetItem(QString::number(pkg.id)));
        ui->packagesTable->setItem(i, 1, new QTableWidgetItem(QString::number(pkg.weight)));
        ui->packagesTable->setItem(i, 2, new QTableWidgetItem(pkg.destination));
        ui->packagesTable->setItem(i, 3, new QTableWidgetItem(pkg.arrivalTime.toString("HH:mm")));
        ui->packagesTable->setItem(i, 4, new QTableWidgetItem(pkg.deadline.toString("HH:mm")));
        ui->packagesTable->setItem(i, 5, new QTableWidgetItem(pkg.deliveredOnTime ? "是" : "否"));
    }
}

void MainWindow::displayVehicles(const QVector<Vehicle>& vehicles)
{
    ui->vehiclesTable->setRowCount(vehicles.size());
    for (int i = 0; i < vehicles.size(); ++i) {
        const Vehicle& veh = vehicles[i];
        ui->vehiclesTable->setItem(i, 0, new QTableWidgetItem(QString::number(veh.id)));
        ui->vehiclesTable->setItem(i, 1, new QTableWidgetItem(QString::number(veh.speed)));
        ui->vehiclesTable->setItem(i, 2, new QTableWidgetItem(QString::number(veh.weight)));
        ui->vehiclesTable->setItem(i, 3, new QTableWidgetItem(QString::number(veh.maxLoad)));
    }
}

void MainWindow::displayMap(const MapGraph& map)
{
    mapScene->clear();
    const int nodeRadius = 20;
    const int sceneSize = 500;
    const double scaleFactor = sceneSize / (map.adjacencyList.size() + 1);

    // 绘制节点(使用圆形布局)
    QMap<int, QPointF> nodePositions;
    for (int i = 1; i < map.adjacencyList.size(); ++i) {
        double angle = 2 * M_PI * (i - 1) / (map.adjacencyList.size() - 1);
        double x = sceneSize/2 + (sceneSize/2 - nodeRadius*2) * cos(angle);
        double y = sceneSize/2 + (sceneSize/2 - nodeRadius*2) * sin(angle);
        nodePositions[i] = QPointF(x, y);

        QGraphicsEllipseItem* node = new QGraphicsEllipseItem(x - nodeRadius, y - nodeRadius,
                                                              nodeRadius*2, nodeRadius*2);
        node->setBrush(i == map.stationIndex ? Qt::red : Qt::blue);
        mapScene->addItem(node);

        // 添加地点名称
        QGraphicsTextItem* label = new QGraphicsTextItem(map.locationNames.value(i));
        label->setPos(x - nodeRadius, y + nodeRadius);
        mapScene->addItem(label);
    }

    // 绘制道路
    for (int from = 1; from < map.adjacencyList.size(); ++from) {
        for (const Road& road : map.adjacencyList[from]) {
            int to = road.to;
            QGraphicsLineItem* line = new QGraphicsLineItem(nodePositions[from].x(), nodePositions[from].y(),
                                                            nodePositions[to].x(), nodePositions[to].y());
            mapScene->addItem(line);

            // 添加道路长度标签
            QPointF midPoint = (nodePositions[from] + nodePositions[to]) / 2;
            QGraphicsTextItem* lengthLabel = new QGraphicsTextItem(QString::number(road.length));
            lengthLabel->setPos(midPoint);
            mapScene->addItem(lengthLabel);
        }
    }
}

void MainWindow::displayResults(const QString& results)
{
    ui->resultsTextEdit->setText(results);
}

void MainWindow::on_task3Button_clicked()
{
    if (!deliveryManager->hasData()) {
        QMessageBox::warning(this, "错误", "请先导入地图、包裹和小车数据!");
        return;
    }

    double cost = 0;
    int latePackages = 0;
    deliveryManager->calculateTask3(cost, latePackages);
    displayResults(QString("任务三结果:\n总配送成本: %1\n未按时送达包裹数: %2").arg(cost).arg(latePackages));
}

void MainWindow::on_task4Button_clicked()
{
    if (!deliveryManager->hasMap()) {
        QMessageBox::warning(this, "错误", "请先导入地图数据!");
        return;
    }

    QString returnLocations = ui->returnLocationsEdit->text();
    if (returnLocations.isEmpty()) {
        QMessageBox::warning(this, "错误", "请输入退货地点!");
        return;
    }

    QString path;
    double time = deliveryManager->calculateTask4(returnLocations.split(" "), path);
    displayResults(QString("任务四结果:\n最短时间: %1 分钟\n路径: %2").arg(time).arg(path));
}

void MainWindow::on_task5Button_clicked()
{
    if (!deliveryManager->hasMap()) {
        QMessageBox::warning(this, "错误", "请先导入地图数据!");
        return;
    }

    bool ok1, ok2;
    int x = ui->startLocationSpin->text().toInt(&ok1);
    int y = ui->endLocationSpin->text().toInt(&ok2);
    if (!ok1 || !ok2) {
        QMessageBox::warning(this, "错误", "请输入有效的地点编号!");
        return;
    }

    double maxEdge = deliveryManager->calculateTask5(x, y);
    displayResults(QString("任务五结果:\n最小化最大边距离: %1").arg(maxEdge));
}

void MainWindow::on_extendTask1Button_clicked()
{
    if (!deliveryManager->hasData()) {
        QMessageBox::warning(this, "错误", "请先导入地图、包裹和小车数据!");
        return;
    }

    double cost = 0;
    int latePackages = 0;
    deliveryManager->calculateExtendTask1(cost, latePackages);
      displayResults(QString("扩展任务一结果:\n总配送成本: %1\n未按时送达包裹数: %2").arg(cost).arg(latePackages));
}

void MainWindow::on_extendTask2Button_clicked()
{
    if (!deliveryManager->hasPackages()) {
        QMessageBox::warning(this, "错误", "请先导入包裹数据!");
        return;
    }

    int maxBatch = ui->batchSizeSpin->value();
    int interval = ui->intervalSpin->value();

    double dissatisfaction = 0;
    deliveryManager->calculateExtendTask2(maxBatch, interval, dissatisfaction);
    displayResults(QString("扩展任务二结果:\n总不满意度: %1").arg(dissatisfaction));
}