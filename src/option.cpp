#include <iostream>
#include <vector>
#include <cmath>
#include <string>

class Option {
private:
    std::string type;          // "call" or "put"
    double strike_price;       // Strike price of the option
    double current_price;      // Current price of underlying asset
    double expiry_time;        // Time to expiration in years
    std::vector<double> historical_prices; // Historical prices of underlying asset

public:
    // Constructor
    Option(std::string type, double strike, double current, double expiry) 
        : type(type), strike_price(strike), current_price(current), expiry_time(expiry) {}

    // Add historical price data
    void add_historical_price(double price) {
        historical_prices.push_back(price);
    }

    // Calculate historical volatility
    float calculate_volatility() {
        if (historical_prices.size() < 2) {
            return 0.0;
        }

        // Calculate daily returns
        std::vector<double> returns;
        for (size_t i = 1; i < historical_prices.size(); i++) {
            double daily_return = log(historical_prices[i] / historical_prices[i-1]);
            returns.push_back(daily_return);
        }

        double mean = 0.0;
        for (double r : returns) {
            mean += r;
        }
        mean /= returns.size();

        // Calculate variance
        double variance = 0.0;
        for (double r : returns) {
            variance += pow(r - mean, 2);
        }
        variance /= (returns.size() - 1);


        float annualized_volatility = sqrt(variance * 252) * 100;  

        return annualized_volatility;
    }

    // Getters
    std::string get_type() const { return type; }
    double get_strike() const { return strike_price; }
    double get_current_price() const { return current_price; }
    double get_expiry_time() const { return expiry_time; }
};

int main() {
    Option call_option("call", 100.0, 102.5, 0.5);  // Strike $100, Current price $102.50, 6 months to expiry

    std::vector<double> sample_prices = {
        102.50, 103.20, 102.80, 103.50, 104.20,
        103.90, 104.50, 105.20, 104.80, 105.50,
        105.20, 104.80, 104.20, 103.80, 104.50,
        105.20, 105.80, 106.20, 105.80, 105.20,
        104.80, 105.50, 106.20, 106.80, 106.50,
        106.20, 105.80, 106.50, 107.20, 107.50
    };

    for (double price : sample_prices) {
        call_option.add_historical_price(price);
    }

    // Calculate and display volatility
    float volatility = call_option.calculate_volatility();
    
    std::cout << "Type: " << call_option.get_type() << std::endl;
    std::cout << "Strike Price: $" << call_option.get_strike() << std::endl;
    std::cout << "Current Price: $" << call_option.get_current_price() << std::endl;
    std::cout << "Time to Expiry (years): " << call_option.get_expiry_time() << std::endl;
    std::cout << "Historical Volatility: " << volatility << "%" << std::endl;

    return 0;
}