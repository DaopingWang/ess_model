﻿//
// Created by Daoping Wang on 30.07.20.
//

#include "RevenueCalculator.h"

RevenueCalculator::RevenueCalculator() {
    rInfo.initialized = false;
    uParams.initialized = false;

}

void
RevenueCalculator::setUserParams(vector<double> *prices, const int tCharge, const int tDischarge, const double minPriceDiff, const int maxCycles) {
    // flush history
    if (uParams.initialized) {
        chargePrices.clear();
        dischargePrices.clear();
    }

    uParams.prices = prices;
    uParams.maxCycles = maxCycles;
    uParams.minPriceDiff = minPriceDiff;
    uParams.tCharge = tCharge;
    uParams.tDischarge = tDischarge;
    uParams.initialized = true;

    for (int i = 0; i < prices->size(); i++) {
        // calculate charge and discharge prices
        double c = 0, d = 0;
        for (int j = i; j < i + tCharge; j++) {
            if (i - 1 + tCharge >= prices->size()) {
                // remaining time not enough for a full charge
                break;
            }
            c += prices->at(j);
        }

        for (int j = i; j < i + tDischarge; j++) {
            if (i - 1 + tDischarge >= prices->size()) {
                break;
            }
            d += prices->at(j);
        }
        c /= tCharge;       // caution: divided by zero
        d /= tDischarge;

        if (i - 1 + tCharge >= prices->size()) c = INT32_MAX;
        chargePrices.push_back(c);

        if (i - 1 + tDischarge >= prices->size()) d = INT32_MIN;
        dischargePrices.push_back(d);
    }
}

RevenueInfo RevenueCalculator::getRevenueInfo() {
    return rInfo;
}

// price diff is considered here
bool RevenueCalculator::cycleValidation()
{
    double r = 0;
    double filteredR = 0;
    for (pair<int, int> c : rInfo.cycleTiming) {
        double revenue = dischargePrices[c.second] - chargePrices[c.first];
        r += revenue;
        if (revenue >= uParams.minPriceDiff) {
            filteredR += revenue;
            rInfo.filteredCycleTiming.push_back(c);
        }
    }
    rInfo.validationSum = r;
    rInfo.filteredSum = filteredR;
    if ((int) r != (int) rInfo.totalRevenue || rInfo.cycleTiming.size() > uParams.maxCycles) return false;
    return true;

}

UserParams RevenueCalculator::getUParams() const
{
    return uParams;
}

void RevenueCalculator::calRMaxCycleUnlimited()
{
    int n = uParams.prices->size();
    vector<double> neutral(n, 0);
    vector<double> charged(n, INT32_MIN);
    vector<int> chargeDates, dischargeDates;
    charged[0] = -chargePrices[0];
    chargeDates.push_back(0);

    vector<RevenueLink*> chargeRL(n, nullptr), dischargeRL(n, nullptr);

    RevenueLink* initialRL = new RevenueLink();
    initialRL->r = -chargePrices[0];
    initialRL->parent = nullptr;
    initialRL->ind = 0;
    initialRL->type = 1;
    chargeRL[0] = initialRL;

    for (int i = 1; i < n; i++) { //- (uParams.tDischarge - 1)
        int chargeCoolDown = max(0, i-uParams.tCharge);
        int dischargeCoolDown = max(0, i-uParams.tDischarge);

        // discharge?
        if (charged[chargeCoolDown] + dischargePrices[i] > neutral[i-1]) {
            //dischargeDates.push_back(i);
            neutral[i] = charged[chargeCoolDown] + dischargePrices[i];

            RevenueLink* rl = new RevenueLink();
            rl->r = neutral[i];
            rl->parent = i-uParams.tCharge < 0 ? nullptr : chargeRL[i-uParams.tCharge];
            rl->ind = i;
            rl->type = 2;
            dischargeRL[i] = rl;

        } else {
            neutral[i] = neutral[i-1];
            dischargeRL[i] = dischargeRL[i-1];
        }


        // charge?
        if (neutral[dischargeCoolDown] - chargePrices[i] > charged[i-1]) {
            //chargeDates.push_back(i);
            charged[i] = neutral[dischargeCoolDown] - chargePrices[i];

            RevenueLink* rl = new RevenueLink();
            rl->r = charged[i];
            rl->parent = i-uParams.tDischarge < 0 ? nullptr : dischargeRL[i-uParams.tDischarge];
            rl->ind = i;
            rl->type = 1;
            chargeRL[i] = rl;

        } else {
            charged[i] = charged[i-1];
            chargeRL[i] = chargeRL[i-1];
        }
    }

    RevenueLink* rl = dischargeRL[n-1];
    pair<int, int> p;
    while (rl != nullptr) {
        if (rl->type == 2) p.second = rl->ind;
        else {
            p.first = rl->ind;
            rInfo.cycleTiming.push_back(p);
        }
        rl = rl->parent;
    }

    rInfo.totalRevenue = neutral[n-1];
}

// CORE ALGORITHM
void RevenueCalculator::calculateRInfo() {
    // flush history
    rInfo.cycleTiming.clear();
    rInfo.filteredCycleTiming.clear();

    int k = uParams.maxCycles, n = uParams.prices->size();

    // no cycle limitation
    if (k > n / 2) {
        calRMaxCycleUnlimited();
        return;
    }

    // cycle number limited
    vector<vector<double>> neutral(n, vector<double>(k+1, 0));
    vector<vector<double>> charged(n, vector<double>(k+1, INT32_MIN));
    vector<vector<RevenueLink*>> neutralRL(n, vector<RevenueLink*>(k+1, nullptr));
    vector<vector<RevenueLink*>> chargeRL(n, vector<RevenueLink*>(k+1, nullptr));

    vector<vector<int>> chargeDates(k+1, vector<int>()), dischargeDates(k+1, vector<int>());

    for (int i = 1; i <= k; i++) {
        chargeDates[i].push_back(0);
    }

    charged[0][0] = -chargePrices[0];
    for (int i = 1; i < n; i++) {
        charged[i][0] = max(charged[i-1][0], -chargePrices[i]);
    }
    for (int j = 1; j <= k; j++) {
        charged[0][j] = -chargePrices[0];
        RevenueLink* rl = new RevenueLink();
        rl->r = -chargePrices[0];
        rl->parent = nullptr;
        rl->ind = 0;
        rl->type = 1;
        chargeRL[0][j] = rl;
    }

    for (int j = 1; j <= k; j++) {
        for (int i = 1; i < n; i++) {
            int chargeCoolDown = max(0, i-uParams.tCharge);
            int dischargeCoolDown = max(0, i-uParams.tDischarge);

            // hold vs discharge
            if (charged[chargeCoolDown][j] + dischargePrices[i] > neutral[i-1][j]) {
                //dischargeDates[j].push_back(i);
                neutral[i][j] = charged[chargeCoolDown][j] + dischargePrices[i];

                RevenueLink* rl = new RevenueLink();
                rl->r = neutral[i][j];
                rl->parent = i-uParams.tCharge < 0 ? nullptr : chargeRL[i-uParams.tCharge][j];
                rl->ind = i;
                rl->type = 2;
                neutralRL[i][j] = rl;

            } else {
                neutral[i][j] = neutral[i-1][j];
                neutralRL[i][j] = neutralRL[i-1][j];
            }

            // rest vs charge
            if (neutral[dischargeCoolDown][j-1] - chargePrices[i] > charged[i-1][j]) {
                //chargeDates[j].push_back(i);
                charged[i][j] = neutral[dischargeCoolDown][j-1] - chargePrices[i];

                RevenueLink* rl = new RevenueLink();
                rl->r = charged[i][j];
                rl->parent = i-uParams.tDischarge < 0 ? nullptr : neutralRL[i-uParams.tDischarge][j-1];
                rl->ind = i;
                rl->type = 1;
                chargeRL[i][j] = rl;
            } else {
                charged[i][j] = charged[i-1][j];
                chargeRL[i][j] = chargeRL[i-1][j];
            }
        }
    }

    RevenueLink* rl = neutralRL[n-1][k];
    pair<int, int> p;
    while (rl != nullptr) {
        if (rl->type == 2) p.second = rl->ind;
        else {
            p.first = rl->ind;
            rInfo.cycleTiming.push_back(p);
        }
        rl = rl->parent;
    }

    rInfo.totalRevenue = neutral[n-1][k];
}

int RevenueCalculator::rMaxReference(int k) {
    vector<double> T_ik0(k+1, 0);
    vector<double> T_ik1(k+1, INT32_MIN);
    vector<double> prices = *(uParams.prices);

    for (double p : prices) {
        for (int j = k; j > 0; j--) {
            T_ik0[j] = max(T_ik0[j], T_ik1[j] + p);
            T_ik1[j] = max(T_ik1[j], T_ik0[j - 1] - p);
        }
    }

    return T_ik0[k];
}

// DEPRECATED
void RevenueCalculator::addCycleTiming(int prevDischarge, vector<int> &chargeDates)
{
    // seek the last valid charge before prevDischarge
    int prevCharge = chargeDates[0];
    int j = 1;
    while (j < chargeDates.size() && (prevDischarge - chargeDates[j] >= uParams.tDischarge)) {
        prevCharge = chargeDates[j];
        j++;
    }
    rInfo.cycleTiming.push_back({prevCharge, prevDischarge});
}

// DEPRECATED
void RevenueCalculator::automataReference() {
    //if (!uParams.initialized) throw "User parameters not initialized!";
    // dynamic programming with Finite State Machine
    // https://www.youtube.com/watch?v=pkiJyNijgBw
    int n = uParams.prices->size();
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
            //if (charging[charging.size()-1][i-1] > charged[i-1]) rInfo.chargeDates.push_back(i-uParams.tCharge+1);
        } else {
            charged[i] = max(charged[i-1], neutral[i-1] - chargePrices[i]);
        }
        if (uParams.tDischarge > 1) {
            discharging[0][i] = charged[i-1] + dischargePrices[i];
            for (int j = 1; j < discharging.size(); j++) {
                discharging[j][i] = discharging[j-1][i-1];
            }
            neutral[i] = max(neutral[i-1], discharging[discharging.size()-1][i-1]);
            //if (discharging[discharging.size()-1][i-1] > neutral[i-1]) rInfo.dischargeDates.push_back(i-uParams.tDischarge+1);
        } else {
            neutral[i] = max(neutral[i-1], charged[i-1] + dischargePrices[i]);
        }
    }

    // results
    rInfo.totalRevenue = neutral[n-1];
}
