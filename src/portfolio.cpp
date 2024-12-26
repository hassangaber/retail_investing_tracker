#include <thread>
#include <chrono>
#include <array>
#include <string>
#include <vector>
#include <iostream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <map>

using json = nlohmann::json;

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

class Position {
public:
    std::string ticker;
    std::string purchase_date;
    double purchase_price;
    double volume;
    
    Position(std::string t, std::string date, double vol)
        : ticker(t), purchase_date(date), purchase_price(0.0), volume(vol) {}
};

class Portfolio {
private:
    std::vector<Position> positions;
    std::map<std::string, std::vector<std::pair<long, double>>> historical_prices;
    
    long convert_date_to_timestamp(const std::string& date) {
        std::tm tm = {};
        std::stringstream ss(date);
        ss >> std::get_time(&tm, "%Y-%m-%d");
        auto time = std::mktime(&tm);
        return time;
    }
    
    bool fetch_historical_data(const std::string& ticker, const std::string& start_date) {
        CURL* curl = curl_easy_init();
        if (!curl) return false;

        long period2 = std::time(nullptr);
        long period1 = convert_date_to_timestamp(start_date);

        std::string url = "https://query1.finance.yahoo.com/v8/finance/chart/" + 
                         ticker + "?period1=" + std::to_string(period1) + 
                         "&period2=" + std::to_string(period2) + 
                         "&interval=1d";

        std::string readBuffer;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "Failed to fetch data for " << ticker << std::endl;
            curl_easy_cleanup(curl);
            return false;
        }

        try {
            json j = json::parse(readBuffer);
            auto timestamps = j["chart"]["result"][0]["timestamp"];
            auto closes = j["chart"]["result"][0]["indicators"]["quote"][0]["close"];
            
            std::vector<std::pair<long, double>> price_data;
            for (size_t i = 0; i < timestamps.size(); ++i) {
                if (!closes[i].is_null()) {
                    price_data.emplace_back(timestamps[i], closes[i]);
                }
            }
            
            historical_prices[ticker] = price_data;
            
        } catch (const std::exception& e) {
            std::cerr << "Error parsing data for " << ticker << ": " << e.what() << std::endl;
            curl_easy_cleanup(curl);
            return false;
        }

        curl_easy_cleanup(curl);
        return true;
    }

    double get_price_on_date(const std::string& ticker, long timestamp) {
        const auto& prices = historical_prices[ticker];
        for (const auto& [ts, price] : prices) {
            if (ts >= timestamp) {
                return price;
            }
        }
        return 0.0;
    }

public:
    void add_position(const std::string& ticker, const std::string& date, double volume) {
        positions.emplace_back(ticker, date, volume);
    }
    
    void generate_report() {
        for (auto& pos : positions) {
            fetch_historical_data(pos.ticker, pos.purchase_date);
            long purchase_ts = convert_date_to_timestamp(pos.purchase_date);
            pos.purchase_price = get_price_on_date(pos.ticker, purchase_ts);
            //std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
        double total_investment = 0;
        double current_value = 0;
        
        std::cout << "\n=== Portfolio Report ===\n\n";
        std::cout << std::setw(10) << "Ticker" 
                  << std::setw(15) << "Buy Date"
                  << std::setw(15) << "Buy Price"
                  << std::setw(10) << "Volume"
                  << std::setw(15) << "Book Value"
                  << std::setw(15) << "Curr Price"
                  << std::setw(15) << "Mkt Value"
                  << std::setw(15) << "Return %\n";
        
        for (const auto& pos : positions) {
            const auto& prices = historical_prices[pos.ticker];
            if (prices.empty()) continue;
            
            double current_price = prices.back().second;
            double book_value = pos.purchase_price * pos.volume;
            double market_value = current_price * pos.volume;
            double pos_return = (current_price - pos.purchase_price) / pos.purchase_price * 100;
            
            total_investment += book_value;
            current_value += market_value;
            
            std::cout << std::setw(10) << pos.ticker 
                      << std::setw(15) << pos.purchase_date
                      << std::setw(15) << std::fixed << std::setprecision(2) << pos.purchase_price
                      << std::setw(15) << pos.volume
                      << std::setw(15) << book_value
                      << std::setw(15) << current_price
                      << std::setw(15) << market_value
                      << std::setw(15) << pos_return << "%\n";
        }
        
        double total_return = (current_value - total_investment) / total_investment * 100;
        
        std::cout << "\nPortfolio Summary:\n";
        std::cout << "Total Book Value: $" << std::fixed << std::setprecision(2) << total_investment << "\n";
        std::cout << "Total Market Value: $" << current_value << "\n";
        std::cout << "Total Return: " << total_return << "%\n";
    }
};

void calculate_portfolio_value_USD() {
    curl_global_init(CURL_GLOBAL_ALL);
    
    Portfolio portfolio;
    
    portfolio.add_position("QQQ", "2024-12-12", 0.436);
    portfolio.add_position("QQQ", "2024-11-29", 0.498);
    portfolio.add_position("TQQQ", "2024-12-18", 5);
    portfolio.add_position("SPY", "2024-12-18", 1);
    portfolio.add_position("SPLG", "2024-12-9", 4);
    portfolio.add_position("SPLG", "2024-12-18", 0.708);
   
    portfolio.generate_report();
    
    curl_global_cleanup();
}

void calculate_portfolio_value_CAD() {
    curl_global_init(CURL_GLOBAL_ALL);
    
    Portfolio portfolio;
    
    portfolio.add_position("HXQ.TO", "2024-12-16", 3);
    portfolio.add_position("HXQ.TO", "2024-12-3", 2);
    portfolio.add_position("HXQ.TO", "2024-11-29", 3);
    portfolio.add_position("XEQT.TO", "2024-11-20",1);
    portfolio.add_position("XEQT.TO", "2024-11-29", 5);
    portfolio.add_position("XEQT.TO", "2024-12-3", 2);
    portfolio.add_position("XEQT.TO", "2024-12-11", 3);
    portfolio.add_position("XEQT.TO", "2024-12-12", 1);
    portfolio.add_position("XEQT.TO", "2024-12-16", 2);
    portfolio.generate_report();
    
    curl_global_cleanup();
}
