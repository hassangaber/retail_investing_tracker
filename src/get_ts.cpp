#include <array>
#include <string>
#include <iostream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <chrono>
#include <ctime>
#include <thread>
#include <iomanip>
#include <sstream>

using json = nlohmann::json;

static const std::array<std::string,7> TICKERS = {"HXQ", "QQQ", "TQQQ", "SPLG", "SPY", "XEQT", "BTCUSD"};

// Helper functions
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

long get_timestamp(int days_ago) {
    auto now = std::chrono::system_clock::now();
    auto time_ago = now - std::chrono::hours(24 * days_ago);
    return std::chrono::system_clock::to_time_t(time_ago);
}

std::string format_timestamp(time_t timestamp) {
    std::stringstream ss;
    ss << std::put_time(std::localtime(&timestamp), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

// Function 1: Get hourly data for last 7 days
bool fetch_hourly_data(const std::string& ticker) {
    CURL* curl = curl_easy_init();
    if (!curl) return false;

    std::string readBuffer;
    long period2 = std::time(nullptr);
    long period1 = get_timestamp(7); // Last 7 days for hourly data

    std::string url = "https://query1.finance.yahoo.com/v8/finance/chart/" + 
                     ticker + "?period1=" + std::to_string(period1) + 
                     "&period2=" + std::to_string(period2) + 
                     "&interval=1h&includePrePost=true";

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "Curl failed: " << curl_easy_strerror(res) << std::endl;
        curl_easy_cleanup(curl);
        return false;
    }

    try {
        json j = json::parse(readBuffer);
        auto data = j["chart"]["result"][0];
        
        std::cout << "\nHOURLY DATA for " << ticker << " (Last 7 days)\n";
        std::cout << "DateTime,Open,High,Low,Close,Volume\n";
        
        auto timestamps = data["timestamp"];
        auto quotes = data["indicators"]["quote"][0];
        
        for (size_t i = 0; i < timestamps.size(); ++i) {
            if (!quotes["close"][i].is_null()) {
                std::cout << format_timestamp(timestamps[i]) << ","
                         << quotes["open"][i] << ","
                         << quotes["high"][i] << ","
                         << quotes["low"][i] << ","
                         << quotes["close"][i] << ","
                         << quotes["volume"][i] << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing hourly data: " << e.what() << std::endl;
        curl_easy_cleanup(curl);
        return false;
    }
    
    curl_easy_cleanup(curl);
    return true;
}

// Function 2: Get daily data for last 30 days
bool fetch_daily_data(const std::string& ticker) {
    CURL* curl = curl_easy_init();
    if (!curl) return false;

    std::string readBuffer;
    long period2 = std::time(nullptr);
    long period1 = get_timestamp(30); // Last 30 days

    std::string url = "https://query1.finance.yahoo.com/v8/finance/chart/" + 
                     ticker + "?period1=" + std::to_string(period1) + 
                     "&period2=" + std::to_string(period2) + 
                     "&interval=1d";

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "Curl failed: " << curl_easy_strerror(res) << std::endl;
        curl_easy_cleanup(curl);
        return false;
    }

    try {
        json j = json::parse(readBuffer);
        auto data = j["chart"]["result"][0];
        
        std::cout << "\nDAILY DATA for " << ticker << " (Last 30 days)\n";
        std::cout << "Date,Open,High,Low,Close,Volume\n";
        
        auto timestamps = data["timestamp"];
        auto quotes = data["indicators"]["quote"][0];
        
        for (size_t i = 0; i < timestamps.size(); ++i) {
            if (!quotes["close"][i].is_null()) {
                std::cout << format_timestamp(timestamps[i]).substr(0, 10) << ","
                         << quotes["open"][i] << ","
                         << quotes["high"][i] << ","
                         << quotes["low"][i] << ","
                         << quotes["close"][i] << ","
                         << quotes["volume"][i] << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing daily data: " << e.what() << std::endl;
        curl_easy_cleanup(curl);
        return false;
    }
    
    curl_easy_cleanup(curl);
    return true;
}

// Function 3: Get only open/close for last 30 days
bool fetch_open_close_data(const std::string& ticker) {
    CURL* curl = curl_easy_init();
    if (!curl) return false;

    std::string readBuffer;
    long period2 = std::time(nullptr);
    long period1 = get_timestamp(30);

    std::string url = "https://query1.finance.yahoo.com/v8/finance/chart/" + 
                     ticker + "?period1=" + std::to_string(period1) + 
                     "&period2=" + std::to_string(period2) + 
                     "&interval=1d";

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "Curl failed: " << curl_easy_strerror(res) << std::endl;
        curl_easy_cleanup(curl);
        return false;
    }

    try {
        json j = json::parse(readBuffer);
        auto data = j["chart"]["result"][0];
        
        std::cout << "\nOPEN/CLOSE DATA for " << ticker << " (Last 30 days)\n";
        std::cout << "Date,Open,Close\n";
        
        auto timestamps = data["timestamp"];
        auto quotes = data["indicators"]["quote"][0];
        
        for (size_t i = 0; i < timestamps.size(); ++i) {
            if (!quotes["close"][i].is_null() && !quotes["open"][i].is_null()) {
                std::cout << format_timestamp(timestamps[i]).substr(0, 10) << ","
                         << quotes["open"][i] << ","
                         << quotes["close"][i] << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing open/close data: " << e.what() << std::endl;
        curl_easy_cleanup(curl);
        return false;
    }
    
    curl_easy_cleanup(curl);
    return true;
}

int main() {
    curl_global_init(CURL_GLOBAL_ALL);

    for (const auto& ticker : TICKERS) {
        std::cout << "\n=== Processing " << ticker << " ===\n";
        fetch_hourly_data(ticker);
        fetch_daily_data(ticker);
        fetch_open_close_data(ticker);
        //std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    curl_global_cleanup();
    return 0;
}
