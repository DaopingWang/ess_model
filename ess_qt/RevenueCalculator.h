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

struct RevenueLink {
    RevenueLink* parent;
    double r;
    int type; // 1 for charge 2 for discharge
    int ind;
};

struct RevenueInfo {
    double totalRevenue;
    double validationSum;
    double filteredSum;
    vector<pair<int, int>> cycleTiming;
    vector<pair<int, int>> filteredCycleTiming;
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
    bool cycleValidation();

private:
    RevenueInfo rInfo;
    UserParams uParams;
    vector<double> chargePrices;
    vector<double> dischargePrices;

    void calRMaxCycleUnlimited();
    void addCycleTiming(int prevDischarge, vector<int> &chargeDates);
};


#endif //ESSMODEL_REVENUECALCULATOR_H
