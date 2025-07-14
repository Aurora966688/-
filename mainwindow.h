#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QGraphicsScene>
#include <QFileDialog>
#include <QMessageBox>
#include "datastructures.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MapLoader;
class DeliveryManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 文件导入相关槽函数
    void on_actionImportMap_triggered();
    void on_actionImportPackages_triggered();
    void on_actionImportVehicles_triggered();

    // 任务处理相关槽函数
    void on_task3Button_clicked();
    void on_task4Button_clicked();
    void on_task5Button_clicked();
    void on_extendTask1Button_clicked();

private:
    Ui::MainWindow *ui;
    MapLoader *mapLoader;
    DeliveryManager *deliveryManager;
    QGraphicsScene *mapScene;

    // 数据显示函数
    void displayPackages(const QVector<Package>& packages);
    void displayVehicles(const QVector<Vehicle>& vehicles);
    void displayMap(const MapGraph& map);
    void displayResults(const QString& results);
};

#endif // MAINWINDOW_H