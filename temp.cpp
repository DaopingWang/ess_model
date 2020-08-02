//
// Created by Daoping Wang on 01.08.20.
//

for (int i = 1; i < n; i++) {
// FSM iteration and decision making, charging part
bool chargeNow;
int newProfit;
if (uParams.tCharge > 1) {
charging[0][i] = neutral[i-1];
for (int j = 1; j < charging.size(); j++) {
charging[j][i] = charging[j-1][i-1];
}
charged[i] = max(charged[i-1], charging[charging.size()-1][i-1] - chargePrices[i]);
//chargeNow = charged[i-1] < charging[charging.size()-1][i-1] - chargePrices[i];
//newProfit = charging[charging.size()-1][i-1] - chargePrices[i];
} else {
charged[i] = max(charged[i-1], neutral[i-1] - chargePrices[i]);
//chargeNow = charged[i-1] < neutral[i-1] - chargePrices[i];
//newProfit = charging[charging.size()-1][i-1] - chargePrices[i];
}
/*if (chargeNow) {
    charged[i] = newProfit;
    rInfo.chargeDates.push_back(i - uParams.tCharge);
} else {
    charged[i] = charged[i-1];
}*/

// FSM iteration and decision making, charging part
bool dischargeNow;
if (uParams.tDischarge > 1) {
discharging[0][i] = charged[i-1] + dischargePrices[i];
for (int j = 1; j < discharging.size(); j++) {
discharging[j][i] = discharging[j-1][i-1];
}
neutral[i] = max(neutral[i-1], discharging[discharging.size()-1][i-1]);
//dischargeNow = neutral[i-1] < discharging[discharging.size()-1][i-1];
//newProfit = discharging[discharging.size()-1][i-1];
} else {
neutral[i] = max(neutral[i-1], charged[i-1] + dischargePrices[i]);
//dischargeNow = neutral[i-1] < charged[i-1] + dischargePrices[i];
//newProfit = charged[i-1] + dischargePrices[i];
}
/*if (dischargeNow) {
    neutral[i] = newProfit;
    rInfo.dischargeDates.push_back(i - uParams.tDischarge);
} else {
    neutral[i] = neutral[i-1];
}*/
}