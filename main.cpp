#include <iostream>
#include <vector>
#include "RevenueCalculator.h"

int main() {
    RevenueCalculator rCal;
    vector<double> prices = {14, 15, 10, 11, 17, 20, 32, 11, 10, 33, 33}; // cents
    //vector<int> prices = {1, 2, 3, 0, 2};
    rCal.setUserParams(prices, 2, 2, 1, 1);
    rCal.calculateRevenueInfo();
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
