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
        //return !(getField(url.substring(url.indexOf('/')), fields, fieldCount).name);
        if(url.indexOf("/home") != -1){
            return false;
        }


        return url.indexOf("fieldValue") == -1;
    }
  }

  void handleRequest(AsyncWebServerRequest *request) {
    request->redirect("/home");
  }
};
