#include <iostream>
#include <curl/curl.h>
#include <string>
#include <sstream>
#include <streambuf>
#include <nlohmann/json.hpp>


using json = nlohmann::json;

extern void calculate_portfolio_value_USD();
extern void calculate_portfolio_value_CAD();


class StreambufCapture : public std::streambuf {
    std::string captured;
public:
    virtual int overflow(int ch) {
        if (ch != EOF) {
            captured += static_cast<char>(ch);
        }
        return ch;
    }
    std::string get_captured() const { return captured; }
};

class PushNotifier {
private:
    std::string api_key;
    CURL* curl;
    
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }




    std::string escapeJSON(const std::string& input) {
      std::string output;
      for (char c : input) {
          switch (c) {
              case '\"': output += "\\\""; break;
              case '\\': output += "\\\\"; break;
              case '\n': output += "\\n"; break;
              case '\r': output += "\\r"; break;
              case '\t': output += "\\t"; break;
              case '\b': output += "\\b"; break;
              case '\f': output += "\\f"; break;
              default:
                  if (static_cast<unsigned char>(c) < 32) {
                      char buf[8];
                      snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned char>(c));
                      output += buf;
                  } else {
                      output += c;
                  }
          }
      }
      return output;
}
public:
    PushNotifier(const std::string& key) : api_key(key) {
        curl_global_init(CURL_GLOBAL_ALL);
        curl = curl_easy_init();
    }
    
    ~PushNotifier() {
        if(curl) curl_easy_cleanup(curl);
        curl_global_cleanup();
    }
    bool sendMonospaceNotification(const std::string& title, const std::string& body) {
      if(!curl) return false;
      
      std::string readBuffer;
      struct curl_slist* headers = NULL;
      
      try {
          // Create JSON object using nlohmann::json
          nlohmann::json push_data = {
              {"type", "note"},
              {"title", title},
              {"body", "```\n" + body + "\n```"}  // Add monospace formatting
          };
          
          // Convert to string
          std::string json_str = push_data.dump();
          
          // Debug output
          std::cout << "JSON payload length: " << json_str.length() << std::endl;
          std::cout << "First 100 chars: " << json_str.substr(0, 100) << std::endl;
          
          // Set headers
          headers = curl_slist_append(headers, "Content-Type: application/json");
          std::string auth_header = "Access-Token: " + api_key;
          headers = curl_slist_append(headers, auth_header.c_str());
          
          // Set CURL options
          curl_easy_setopt(curl, CURLOPT_URL, "https://api.pushbullet.com/v2/pushes");
          curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
          curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str.c_str());
          curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
          curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
          
          // Keep verbose output for debugging
          curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
          
          // Perform request
          CURLcode res = curl_easy_perform(curl);
          
          // Print response and result
          std::cout << "Response: " << readBuffer << std::endl;
          std::cout << "CURL result: " << res << std::endl;
          
          curl_slist_free_all(headers);
          return (res == CURLE_OK);
          
      } catch (const nlohmann::json::exception& e) {
          std::cerr << "JSON error: " << e.what() << std::endl;
          if(headers) curl_slist_free_all(headers);
          return false;
    }
}

};


void send_portfolio_notification(const std::string& api_key) {
    // Print debug info
    std::cout << "Starting portfolio notification process..." << std::endl;

    // Capture the original buffer
    std::streambuf* orig_buf = std::cout.rdbuf();

    // Set up capture buffer
    StreambufCapture capture_buf;
    std::cout.rdbuf(&capture_buf);

    calculate_portfolio_value_USD();
    calculate_portfolio_value_CAD();

    // Restore original buffer
    std::cout.rdbuf(orig_buf);

    // Get captured output
    std::string portfolio_output = capture_buf.get_captured();

    // Print debug info
    std::cout << "Captured output length: " << portfolio_output.length() << std::endl;

    // Send notification
    PushNotifier notifier(api_key);
    bool sent = notifier.sendMonospaceNotification("Portfolio Update", portfolio_output);

    // Print result
    std::cout << "Notification sent: " << (sent ? "success" : "failed") << std::endl;
}




int main() {
    const std::string API_KEY = "o.XXXXX";
    
    // This will both print to console and send to phone
    send_portfolio_notification(API_KEY);
    
    return 0;
}
