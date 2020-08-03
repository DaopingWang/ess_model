//
// Created by Daoping Wang on 30.07.20.
//

#include "RevenueCalculator.h"

void
RevenueCalculator::setUserParams(vector<double> &prices, int tCharge, int tDischarge, double minPriceDiff, int maxCycles) {
    uParams.prices = prices;
    uParams.maxCycles = maxCycles;
    uParams.minPriceDiff = minPriceDiff;
    uParams.tCharge = tCharge;
    uParams.tDischarge = tDischarge;
    uParams.initialized = true;

    for (int i = 0; i < prices.size(); i++) {
        // calculate charge and discharge prices
        double c = 0, d = 0;
        for (int j = i; j < i + tCharge; j++) {
            if (i - 1 + tCharge >= prices.size()) {
                // remaining time not enough for a full charge
                break;
            }
            c += prices[j];
        }
        for (int j = i; j < i + tDischarge; j++) {
            if (i - 1 + tDischarge >= prices.size()) break;
            d += prices[j];
        }
        c /= tCharge;       // caution: divided by zero
        d /= tDischarge;
        chargePrices.push_back(c);
        dischargePrices.push_back(d);
    }
}

void RevenueCalculator::calculateRevenueInfo() {
    //if (!uParams.initialized) throw "User parameters not initialized!";
    // dynamic programming with Finite State Machine
    // https://www.youtube.com/watch?v=pkiJyNijgBw
    int n = uParams.prices.size();
    vector<double> neutral(n, INT32_MIN), charged(n, INT32_MIN);

    // number of charging states == tCharge - 1
    vector<vector<double>> charging(uParams.tCharge - 1, vector<double>(n, INT32_MIN));

    // number of discharging states == tDischarge - 1
    vector<vector<double>> discharging(uParams.tDischarge - 1, vector<double>(n, INT32_MIN));

    // starting state could be either neutral, or
    // 1. charging[0][0] if tCharge > 1
    // 2. charged[0] if tCharge = 1
    neutral[0] = 0;
    if (uParams.tCharge > 1) charging[0][0] = -chargePrices[0];
    else charged[0] = -chargePrices[0];

    for (int i = 1; i < n; i++) {
        if (uParams.tCharge > 1) {
            charging[0][i] = neutral[i-1] - chargePrices[i];
            for (int j = 1; j < charging.size(); j++) {
                charging[j][i] = charging[j-1][i-1];
            }
            charged[i] = max(charged[i-1], charging[charging.size()-1][i-1]);
            if (charging[charging.size()-1][i-1] > charged[i-1]) rInfo.chargeDates.push_back(i-uParams.tCharge+1);
        } else {
            charged[i] = max(charged[i-1], neutral[i-1] - chargePrices[i]);
        }
        if (uParams.tDischarge > 1) {
            discharging[0][i] = charged[i-1] + dischargePrices[i];
            for (int j = 1; j < discharging.size(); j++) {
                discharging[j][i] = discharging[j-1][i-1];
            }
            neutral[i] = max(neutral[i-1], discharging[discharging.size()-1][i-1]);
            if (discharging[discharging.size()-1][i-1] > neutral[i-1]) rInfo.dischargeDates.push_back(i-uParams.tDischarge+1);
        } else {
            neutral[i] = max(neutral[i-1], charged[i-1] + dischargePrices[i]);
        }
    }

    // results
    rInfo.totalRevenue = neutral[n-1];
    if (uParams.tDischarge > 1) {
        rInfo.revenues = discharging[discharging.size()-1];
    }
    else {
        rInfo.revenues = charged;
    }
}

RevenueInfo RevenueCalculator::getRevenueInfo() {
    //if (!rInfo.initialized) throw "Revenue information not initialized!";
    return rInfo;
}

RevenueCalculator::RevenueCalculator() {
    rInfo.initialized = false;
    uParams.initialized = false;
}

void RevenueCalculator::calculateRInfo() {
    int k = uParams.maxCycles, n = uParams.prices.size();

    if (k > n / 2) {
        // no cycle limitation
        vector<double> neutral(n, 0);
        vector<double> charged(n, INT32_MIN);
        charged[0] = -chargePrices[0];
        for (int i = 1; i < n; i++) {
            int chargeCoolDown = max(0, i-uParams.tCharge);
            int dischargeCoolDown = max(0, i-uParams.tDischarge);
            neutral[i] = max(neutral[i-1], charged[chargeCoolDown] + dischargePrices[i]);
            charged[i] = max(charged[i-1], neutral[dischargeCoolDown] - chargePrices[i]);
        }
        rInfo.totalRevenue = neutral[n-1];
        return;
    }

    vector<vector<double>> neutral(n, vector<double>(k+1, 0));
    vector<vector<double>> charged(n, vector<double>(k+1, INT32_MIN));
    charged[0][0] = -chargePrices[0];
    for (int i = 1; i < n; i++) {
        charged[i][0] = max(charged[i-1][0], -chargePrices[i]);
    }
    for (int j = 1; j <= k; j++) {
        charged[0][j] = -chargePrices[0];
    }

    for (int i = 1; i < n; i++) {
        for (int j = 1; j <= k; j++) {
            int chargeCoolDown = max(0, i-uParams.tCharge);
            int dischargeCoolDown = max(0, i-uParams.tDischarge);
            // hold vs discharge
            neutral[i][j] = max(neutral[i-1][j], charged[chargeCoolDown][j-1] + dischargePrices[i]);
            // rest vs charge
            charged[i][j] = max(charged[i-1][j], neutral[dischargeCoolDown][j] - chargePrices[i]);
        }
    }
    rInfo.totalRevenue = neutral[n-1][k];
}
