//https://github.com/me-no-dev/ESPAsyncWebServer/blob/master/examples/CaptivePortal/CaptivePortal.ino
const short DNS_PORT = 53;
DNSServer dnsServer;
AsyncWebSocket ws("/ws");

class CaptiveRequestHandler : public AsyncWebHandler {
public:
  CaptiveRequestHandler() {}
  virtual ~CaptiveRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request){
    if(request->hasParam("key")){ // we get the bypass key "pogger"
        return (request->getParam("key")->value().compareTo(bypass_pass) != 0);
    }
    else{ // or our request is a getFieldRequest
        String url = request->url();
        Serial.println("DNS Handling " + url);
        //return !(getField(url.substring(url.indexOf('/')), fields, fieldCount).name);
        if(url.indexOf("/home") != -1){
            return false;
        }
        if(url.indexOf("home.htm") != -1 || url.indexOf(".jpg") != -1 || url.indexOf(".css") != -1 || url.compareTo("/") == 0){
          return false;
        }
        if(url.indexOf(".js") != -1 || url.indexOf(".jpg") != -1 || url.indexOf(".css") != -1 || url.indexOf("index.htm") != -1){
          return false;
        }


        return url.indexOf("fieldValue") == -1;
    }
  }

  void handleRequest(AsyncWebServerRequest *request) {
    Serial.println("DNS Redirect");
    request->redirect("/home");
  }
};
