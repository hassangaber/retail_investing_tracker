Want to track my equities investments and try and see new opportunities without having to open the WealthSimple App. So I'm gonna create a scheduled portfolio update through a Docker-hosted gRPC server written in C++.


`g++ -std=c++17 portfolio.cpp -lcurl -I/opt/homebrew/Cellar/nlohmann-json/3.11.3/include -L/opt/homebrew/lib -o portfolio`
