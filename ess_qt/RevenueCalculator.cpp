//
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
        meaningfulChargingPos.clear();
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

    // evaluate if a charge price is meaningful regarding minPriceDiff
    if (minPriceDiff == 0) {
        for (int i = 0; i < chargePrices.size(); i++) meaningfulChargingPos.push_back(true);
    } else {
        for (int i = 0; i < chargePrices.size(); i++) meaningfulChargingPos.push_back(false);
        for (int i = 0; i < chargePrices.size(); i++) {
            for (int j = i + tCharge; j < dischargePrices.size(); j++) {
                if (dischargePrices[j] - chargePrices[i] >= minPriceDiff) {
                    meaningfulChargingPos[i] = true;
                }
            }
        }
    }

    if (meaningfulChargingPos.size() != chargePrices.size()) qDebug() << "meaningful cp wrong size: "
                                                                      << QString::number(meaningfulChargingPos.size())
                                                                      << " vs " << QString::number(chargePrices.size()) << endl;
}

RevenueInfo RevenueCalculator::getRevenueInfo() {
    return rInfo;
}

// price diff is considered here
bool RevenueCalculator::cycleValidation()
{
    double r = 0;
    bool firstRound = true;
    for (int i = 0; i < rInfo.cycleTiming.size(); i++) {
        pair<int, int> c = rInfo.cycleTiming.at(i);
        if (!firstRound && rInfo.cycleTiming.at(i-1).first - c.second < uParams.tDischarge) qDebug() << "charge cycle violation: "
                                                                                                     << QString::number(rInfo.cycleTiming.at(i-1).first) << " "
                                                                                                     << QString::number(c.second) << endl;
        firstRound = false;
        double revenue = dischargePrices[c.second] - chargePrices[c.first];
        r += revenue;
        rInfo.revenues.push_back(revenue);
        if (revenue < uParams.minPriceDiff) qDebug() << "revenue too low: " << QString::number(revenue) << endl;
        if (c.second - c.first < uParams.tCharge) qDebug() << "discharge cycle violation"
                                                           << QString::number(c.second) << " "
                                                           << QString::number(c.first) << endl;
    }
    rInfo.validationSum = r;
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
    charged[0] = -chargePrices[0];

    vector<RevenueLink*> chargeRL(n, nullptr), dischargeRL(n, nullptr);
    vector<RevenueLink*> garbageCollector;

    RevenueLink* initialRL = new RevenueLink();
    initialRL->r = -chargePrices[0];
    initialRL->parent = nullptr;
    initialRL->ind = 0;
    initialRL->type = 1;
    chargeRL[0] = initialRL;

    for (int i = 1; i < n; i++) { //- (uParams.tDischarge - 1)
        int chargeCoolDown = i-uParams.tCharge;
        int dischargeCoolDown = max(0, i-uParams.tDischarge);

        // discharge?
        double curRevenue = chargeCoolDown < 0 ? INT32_MIN : dischargePrices[i] - chargePrices[chargeRL[chargeCoolDown]->ind];
        if (chargeCoolDown < 0 || curRevenue < uParams.minPriceDiff) {
            neutral[i] = neutral[i-1];
            dischargeRL[i] = dischargeRL[i-1];
        } else if (charged[chargeCoolDown] + dischargePrices[i] > neutral[i-1]) {
            neutral[i] = charged[chargeCoolDown] + dischargePrices[i];

            RevenueLink* rl = new RevenueLink();
            rl->r = neutral[i];
            rl->parent = i-uParams.tCharge < 0 ? nullptr : chargeRL[i-uParams.tCharge];
            rl->ind = i;
            rl->type = 2;
            dischargeRL[i] = rl;
            garbageCollector.push_back(rl);

        } else {
            neutral[i] = neutral[i-1];
            dischargeRL[i] = dischargeRL[i-1];
        }


        // charge?
        if (meaningfulChargingPos[i] && neutral[dischargeCoolDown] - chargePrices[i] > charged[i-1]) {
            charged[i] = neutral[dischargeCoolDown] - chargePrices[i];

            RevenueLink* rl = new RevenueLink();
            rl->r = charged[i];
            rl->parent = i-uParams.tDischarge < 0 ? nullptr : dischargeRL[i-uParams.tDischarge];
            rl->ind = i;
            rl->type = 1;
            chargeRL[i] = rl;
            garbageCollector.push_back(rl);
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

    // clean up mem
    for (auto pIter = garbageCollector.begin(); pIter != garbageCollector.end(); pIter++) delete (*pIter);

    rInfo.totalRevenue = neutral[n-1];
}

// CORE ALGORITHM
void RevenueCalculator::calculateRInfo() {
    // flush history
    rInfo.cycleTiming.clear();
    rInfo.filteredCycleTiming.clear();
    rInfo.revenues.clear();
    rInfo.filteredRevenues.clear();

    int k = uParams.maxCycles, n = uParams.prices->size();

    // no cycle limitation
    if (k > n / 2) {
        //k = n / 2 + 1;
        calRMaxCycleUnlimited();
        return;
    }

    // cycle number limited
    vector<vector<double>> neutral(n, vector<double>(k+1, 0));
    vector<vector<double>> charged(n, vector<double>(k+1, INT32_MIN));
    vector<vector<RevenueLink*>> dischargeRL(n, vector<RevenueLink*>(k+1, nullptr));
    vector<vector<RevenueLink*>> chargeRL(n, vector<RevenueLink*>(k+1, nullptr));

    // garbage collector
    vector<RevenueLink*> garbageCollector;

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
        garbageCollector.push_back(rl);
    }

    for (int j = 1; j <= k; j++) {
        for (int i = 1; i < n; i++) {
            int chargeCoolDown = i-uParams.tCharge;
            int dischargeCoolDown = max(0, i-uParams.tDischarge);

            // hold vs discharge
            double curRevenue = chargeCoolDown < 0 ? INT32_MIN : dischargePrices[i] - chargePrices[chargeRL[chargeCoolDown][j]->ind];
            if (chargeCoolDown < 0 || curRevenue < uParams.minPriceDiff) {
                neutral[i][j] = neutral[i-1][j];
                dischargeRL[i][j] = dischargeRL[i-1][j];
            } else if (charged[chargeCoolDown][j] + dischargePrices[i] > neutral[i-1][j] ) {
                neutral[i][j] = charged[chargeCoolDown][j] + dischargePrices[i];

                RevenueLink* rl = new RevenueLink();
                rl->r = neutral[i][j];
                rl->parent = i-uParams.tCharge < 0 ? nullptr : chargeRL[i-uParams.tCharge][j];
                rl->ind = i;
                rl->type = 2;
                dischargeRL[i][j] = rl;
                garbageCollector.push_back(rl);

            } else {
                neutral[i][j] = neutral[i-1][j];
                dischargeRL[i][j] = dischargeRL[i-1][j];
            }

            // rest vs charge
            if (meaningfulChargingPos[i] && neutral[dischargeCoolDown][j-1] - chargePrices[i]> charged[i-1][j]) {
                charged[i][j] = neutral[dischargeCoolDown][j-1] - chargePrices[i];

                RevenueLink* rl = new RevenueLink();
                rl->r = charged[i][j];
                rl->parent = i-uParams.tDischarge < 0 ? nullptr : dischargeRL[i-uParams.tDischarge][j-1];
                rl->ind = i;
                rl->type = 1;
                chargeRL[i][j] = rl;
                garbageCollector.push_back(rl);
            } else {
                charged[i][j] = charged[i-1][j];
                chargeRL[i][j] = chargeRL[i-1][j];
            }
        }
    }

    RevenueLink* rl = dischargeRL[n-1][k];
    pair<int, int> p;
    while (rl != nullptr) {
        if (rl->type == 2) p.second = rl->ind;
        else {
            p.first = rl->ind;
            rInfo.cycleTiming.push_back(p);
        }
        rl = rl->parent;
    }

    // clean up mem
    for (auto pIter = garbageCollector.begin(); pIter != garbageCollector.end(); pIter++) delete (*pIter);

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
