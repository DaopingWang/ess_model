//
// Created by Daoping Wang on 30.07.20.
//

#ifndef ESSMODEL_REVENUECALCULATOR_H
#define ESSMODEL_REVENUECALCULATOR_H

#include <vector>
#include <queue>
#include <stack>
#include <algorithm>
#include <iostream>

using namespace std;

struct RevenueInfo {
    double totalRevenue; // in cent
    int cycles;
    vector<int> chargeDates;
    vector<int> dischargeDates;
    vector<pair<int, int>> cycleTiming;
    vector<double> revenues;
    bool initialized;
};

struct UserParams {
    vector<double> *prices;
    int tCharge;
    int tDischarge;
    double minPriceDiff;
    int maxCycles;
    bool initialized;
};

class RevenueCalculator {
public:
    RevenueCalculator();
    void automataReference();
    void calculateRInfo();
    int rMaxReference(int k);
    void setUserParams(vector<double>* prices, const int tCharge, const int tDischarge, const double minPriceDiff, const int maxCycles);
    RevenueInfo getRevenueInfo();
    bool cycleVerification();

private:
    RevenueInfo rInfo;
    UserParams uParams;
    vector<double> chargePrices;
    vector<double> dischargePrices;

    void calRMaxCycleUnlimited();
    void addCycleTiming(int prevDischarge, vector<int> &chargeDates);
};


#endif //ESSMODEL_REVENUECALCULATOR_H
