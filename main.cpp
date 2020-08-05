#include <iostream>
#include <vector>
#include "RevenueCalculator.h"

int main() {
    RevenueCalculator rCal;
    //vector<double> prices = {14, 15, 10, 11, 17, 20, 32, 11, 10, 33, 33}; // cents
    vector<double> prices = {24.14,22.12,13.01,4.97,9.81,18.37,23.50,27.17,36.56,
                             40.43,32.22,38.98,38.60,37.90,38.00,39.58,42.28,46.06,
                             47.73,46.00,42.20,39.74,38.88,37.39};
    rCal.setUserParams(prices, 2, 2, 1, 3);
    rCal.calculateRevenueInfo();
    cout << rCal.getRevenueInfo().totalRevenue << endl;
    rCal.calculateRInfo();
    cout << "Maximum revenue for given uParams: " << rCal.getRevenueInfo().totalRevenue << endl;
    //std::cout << "Hello, World!" << std::endl;
    return 0;
}
