#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <math.h>
#include "App.h"
#include "../Common/InputParser.h"
#include "../Common/Screenshots.h"
#include "../Common/Rois.h"

using namespace std;
using namespace cv;


int main(int argc, char **argv) {
    InputParser input(argc, argv);

    auto argStrId = input.getCmdOption("-id");
    int id = stoi(argStrId);

    FileStorage configFs("../../config.json", FileStorage::READ);
    string dataPath;
    configFs["dataPath"] >> dataPath;
    configFs.release();

    App myApp;
    myApp.calibrateCamera();
    myApp.initCamera();
    myApp.initLight();
    myApp.turnLightOff();

    Mat rawLitOn, rawLitOff, undistOn, undistOff;
    vector<thread> workers;

    myApp.turnLightOn();
    rawLitOn = myApp.takeCroppedPicture(428, 13, 1100, 1067);
    workers.push_back(thread([&]() {
        myApp.undistord(rawLitOn, undistOn);
    }));

    myApp.turnLightOff();
    rawLitOff = myApp.takeCroppedPicture(428, 13, 1100, 1067);
    workers.push_back(thread([&]() {
        myApp.undistord(rawLitOff, undistOff);
    }));

    for_each(workers.begin(), workers.end(), [](thread &t) {
        t.join();
    });

    Screenshots screenshot(rawLitOn, rawLitOn, undistOn, undistOff, dataPath, id);
    screenshot.segmentation();
    screenshot.writeToFile(true);

    myApp.close();
    return 0;
}
