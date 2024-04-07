#include "intercom.h"
#include "os/console.h"
#include "settings.h"
#include "comUtilities.h"

INSTANTIATE_SERVICE(Intercom)

Intercom::Intercom() : Service(ID_INTERCOM),  _stream(INTERCOM_SERIAL) {}

void Intercom::onAttach() {
    Console::info() << "Intercom activated" << Console::endl;
}

void Intercom::onUpdate() {
    if(!enabled()) return;

    if (_stream.available()) {
        _lastStream = millis();
        _processIncomingData();         
    }

    _processPendingRequests();

    if(millis() - _lastPing > 500 && (!_connected || (_connected && millis() - _lastStream > 2000))){
        sendMessage("ping");
        _lastPing = millis();
    }
    if(_connected && millis() - _lastStream > 4000){
        connectionLost();
    }
}

void Intercom::enable(){
    Service::enable();
    INTERCOM_SERIAL.begin(INTERCOM_BAUDRATE);
    sendMessage("ping");
}

void Intercom::disable(){
    INTERCOM_SERIAL.end();
    Service::disable();
}

void Intercom::sendMessage(const char* message) {
    _stream.print(message);
    _stream.write('\n');
    Console::trace("Intercom") << ">" << message << Console::endl;
}

void Intercom::sendMessage(const String& message) {
    _stream.print(message);
    _stream.write('\n');
    Console::trace("Intercom") << ">" << message.c_str() << Console::endl;
}

uint32_t Intercom::sendRequest(const String& payload, long timeout,  requestCallback_ptr cbfunc,  callback_ptr func){
    Request req(payload, timeout, cbfunc, func);
    _sentRequests.insert({req.ID(), req});
    req.send(*this);
    return req.ID();
}

void Intercom::pingReceived() {
    sendMessage("pong");
}

bool Intercom::closeRequest(const uint32_t& uid) {
    if(_sentRequests.count(uid)){
        _sentRequests.find(uid)->second.close();
        return true;
    }else{
        Console::warn("Intercom") << __FILE__ << " at line " << __LINE__ << " request " << int(uid) << " does not exist" << Console::endl;
        return false;
    };
}

void Intercom::setConnectLostCallback(callback_ptr callback){
    onConnectionLost = callback;
}

void Intercom::setConnectionSuccessCallback(callback_ptr callback){
    onConnectionSuccess = callback;
}

String Intercom::getRequestResponse(const uint32_t& uid) {
    if(_sentRequests.count(uid) > 0){
        return _sentRequests.find(uid)->second.getResponse();
    }else{
        Console::warn("Intercom") << __FILE__ << " at line " << __LINE__ << " request " << int(uid)   << " does not exist" << Console::endl;
        return "ERROR";
    };
}

void Intercom::connectionLost() {
    Console::warn("Intercom") << "Connection lost." << Console::endl;
    _connected = false;
    if(onConnectionLost!=nullptr){
        onConnectionLost();
    }
}

void Intercom::connectionSuccess(){
    Console::info("Intercom") << "Connection successful." << Console::endl;
    _connected = true;
    if(onConnectionSuccess!=nullptr){
        onConnectionSuccess();
    }
}

void Intercom::_processIncomingData() {
    while (_stream.available()) {
        String incomingMessage = _stream.readStringUntil('\n');
        incomingMessage.trim(); // Remove any leading/trailing whitespace or newline characters
        Console::trace("Intercom") << "<" << incomingMessage.c_str() << Console::endl;
        
        if (incomingMessage.startsWith("ping")) {
            pingReceived();
        } else if (incomingMessage.startsWith("pong")) {
            if(!isConnected())connectionSuccess();
        }else if (!_sentRequests.empty()) {
            
            String responseIdRaw = incomingMessage.substring(0, 4);
            if(responseIdRaw.length() != 4) {
                //Bad response id
                continue;
            }

            byte buf[4];
            responseIdRaw.getBytes(buf, 4);

            uint32_t responseId = parseUInt32(buf);
            String responseData = incomingMessage.substring(5, incomingMessage.length()-1); //without crc

            if(checkCRC(responseData, incomingMessage[incomingMessage.length()-1])){
                Console::error("Intercom") << "Bad crc for message" << incomingMessage << Console::endl;
            }

            auto requestIt = _sentRequests.find(responseId);
            if (requestIt != _sentRequests.end()) {
                Request& request = requestIt->second;
                request.onResponse(responseData);
            }
        }
    }
}

void Intercom::_processPendingRequests() {
    for (auto it = _sentRequests.begin(); it != _sentRequests.end();) {

        Request& request = it->second;
        Request::Status status = request.getStatus();
    
        if(status != Request::Status::CLOSED && status != Request::Status::IDLE && millis() - request.getLastSent() > 1000){
            Console::trace("Intercom") << ": request " << request.getPayload() << "too old, cleared." << Console::endl;
            request.close();
            ++it;
            continue;
        }

        if (status == Request::Status::IDLE) {
            request.send(*this);
            ++it;
        } else if (status == Request::Status::SENT) {
            if (request.isTimedOut()) {
                request.onTimeout();
                
            } else {
                ++it;
            }
        } else if (status == Request::Status::OK) {
            ++it;
        } else if (status == Request::Status::CLOSED) {
            Console::trace("Intercom") << int(_sentRequests.size()) << "currently in the buffer" << Console::endl;
            it = _sentRequests.erase(it); // Remove the request from the map

        }  else if (status == Request::Status::TIMEOUT) {
            request.close();
            Console::error("Intercom") << ": request " << request.getPayload() << "timedout." << Console::endl;
        } else if (status == Request::Status::ERROR) {
            request.close();
            Console::error("Intercom") << ": request " << request.getPayload() << "unknown error." << Console::endl;
        } else {
            ++it;
        }
    }
}

uint32_t Request::_uidCounter = 0;

Request::Request(const String& content, long timeout, requestCallback_ptr func_call, callback_ptr timeout_call)
    :   _uid(0),
        _content(content), 
        _lastSent(0),
        _responseTime(0),
        _timeout(timeout),
        _status(Status::IDLE), 
        _callback(func_call),
        _timeoutCallback(timeout_call)
        {_uid = _uidCounter++;}

void Request::setTimeoutCallback(callback_ptr func){
    _timeoutCallback = func;
}

void Request::setCallback(requestCallback_ptr func){
    _callback = func;
}

void Request::send(Intercom& channel){
    _status = Status::SENT;
    _lastSent = millis();
    channel.sendMessage(getPayload());
}

void Request::close(){
    _status = Status::CLOSED;
}

void Request::onResponse(const String& response){
    _status = Status::OK;
    _responseTime = millis();
    _response = response;
    if(_callback) _callback(response);
}

void Request::onTimeout(){
    _status = Status::TIMEOUT;
    if(_timeoutCallback) _timeoutCallback();
}

void Request::setStatus(Status status){
    _status = status;
}
    
uint32_t Request::ID() const{
    return _uid;
}

Request::Status Request::getStatus() const{
    return _status;
}

const String& Request::getMessage() const{
    return _content;
}

const String& Request::getResponse() const{

    return _response;
}

//uuidcontentcrc
String Request::getPayload() const{
    u_int size = _content.length();
    size += 4; // uuid:
    size += 1; // |crc
    byte uuid[4];
    serializeUInt32(uuid, _uid);
    
    char rawBuf[size];
    rawBuf[0] = uuid[0];
    rawBuf[1] = uuid[1];
    rawBuf[2] = uuid[2];
    rawBuf[3] = uuid[3];

    _content.toCharArray(rawBuf, size, 4);

    rawBuf[size-1] = CRC8.smbus((uint8_t*)rawBuf, size-1);
    return String(rawBuf);
}

bool Request::isTimedOut() const{
    return millis() - _lastSent >= _timeout;
}

unsigned long Request::getTimeout() const{
    return _timeout;
}

unsigned long Request::getResponseTime() const{
    return _responseTime;
}

unsigned long Request::getLastSent() const{
    return _lastSent;
}
