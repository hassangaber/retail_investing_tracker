#!/bin/bash

cd src

g++ -std=c++17 portfolio.cpp message_pushbullet.cpp -lcurl -I/opt/homebrew/Cellar/nlohmann-json/3.11.3/include -L/opt/homebrew/lib -o portfolio_monitor.out

chmod +x portfolio_monitor.out

./portfolio_monitor.out

