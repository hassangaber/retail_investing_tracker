#include <array>
#include <string>
#include <iostream>

static const std::array<std::string,7> TICKERS = {"HXQ", "QQQ", "TQQQ", "SPLG", "SPY", "XEQT", "BTCUSD"};

int main(){
    for(const auto& ticker : TICKERS){
        std::cout << ticker << " ";
    }
    std::cout << std::endl;
    return 0;
}
