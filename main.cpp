#include <iostream>
#include <vector>
#include "RevenueCalculator.h"

int main() {
    RevenueCalculator rCal;
    //vector<double> prices = {14, 15, 10, 11, 17, 20, 32, 11, 10, 33, 33}; // cents
    vector<double> prices = {11.44,0.07,-19.97,-67.98,-10.09,-19.93,4.90,12.70,15.25,17.46,19.86,23.10};
    rCal.setUserParams(prices, 2, 2, 1, 3);
    rCal.calculateRevenueInfo();
    cout << rCal.getRevenueInfo().totalRevenue << endl;
    rCal.calculateRInfo();
    cout << rCal.getRevenueInfo().totalRevenue << endl;
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
