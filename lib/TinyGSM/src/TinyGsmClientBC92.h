/**
 * @file       TinyGsmClientBC92.h
 * @author     Volodymyr Shymanskyy and Aurelien BOUIN (SSL)
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Apr 2018, Aug 2023 (SSL)
 */

#ifndef SRC_TINYGSMCLIENTBC92_H_
#define SRC_TINYGSMCLIENTBC92_H_
// #pragma message("TinyGSM:  TinyGsmClientBC92")

// #define TINY_GSM_DEBUG Serial

#define TINY_GSM_MUX_COUNT 7
#define TINY_GSM_BUFFER_READ_AND_CHECK_SIZE
#ifdef AT_NL
#undef AT_NL
#endif
#define AT_NL "\r\n"

#ifdef MODEM_MANUFACTURER
#undef MODEM_MANUFACTURER
#endif
#define MODEM_MANUFACTURER "Quectel"

#ifdef MODEM_MODEL
#undef MODEM_MODEL
#endif
#define MODEM_MODEL "BC92"

#include "TinyGsmModem.tpp"
#include "TinyGsmTCP.tpp"
#include "TinyGsmSSL.tpp"
#include "TinyGsmGPRS.tpp"
#include "TinyGsmCalling.tpp"
#include "TinyGsmSMS.tpp"
#include "TinyGsmGPS.tpp"
#include "TinyGsmTime.tpp"
#include "TinyGsmNTP.tpp"
#include "TinyGsmBattery.tpp"
#include "TinyGsmTemperature.tpp"

enum BC92RegStatus {
  REG_NO_RESULT    = -1,
  REG_UNREGISTERED = 0,
  REG_SEARCHING    = 2,
  REG_DENIED       = 3,
  REG_OK_HOME      = 1,
  REG_OK_ROAMING   = 5,
  REG_UNKNOWN      = 4,
};

class TinyGsmBC92 : public TinyGsmModem<TinyGsmBC92>,
                    public TinyGsmGPRS<TinyGsmBC92>,
                    public TinyGsmTCP<TinyGsmBC92, TINY_GSM_MUX_COUNT>,
                    public TinyGsmSSL<TinyGsmBC92, TINY_GSM_MUX_COUNT>,
                    public TinyGsmCalling<TinyGsmBC92>,
                    public TinyGsmSMS<TinyGsmBC92>,
                    public TinyGsmGPS<TinyGsmBC92>,
                    public TinyGsmTime<TinyGsmBC92>,
                    public TinyGsmNTP<TinyGsmBC92>,
                    public TinyGsmBattery<TinyGsmBC92>,
                    public TinyGsmTemperature<TinyGsmBC92> {
  friend class TinyGsmModem<TinyGsmBC92>;
  friend class TinyGsmGPRS<TinyGsmBC92>;
  friend class TinyGsmTCP<TinyGsmBC92, TINY_GSM_MUX_COUNT>;
  friend class TinyGsmSSL<TinyGsmBC92, TINY_GSM_MUX_COUNT>;
  friend class TinyGsmCalling<TinyGsmBC92>;
  friend class TinyGsmSMS<TinyGsmBC92>;
  friend class TinyGsmGPS<TinyGsmBC92>;
  friend class TinyGsmTime<TinyGsmBC92>;
  friend class TinyGsmNTP<TinyGsmBC92>;
  friend class TinyGsmBattery<TinyGsmBC92>;
  friend class TinyGsmTemperature<TinyGsmBC92>;

/*
 * Custom Var to handle native mqtt
 */
int unhandledFlag = 0;
bool FLAG_QMT = 0;
int QMT_RES = 0;
uint8_t QMT_MUX = 0;
String unhandledData = "";


  /*
   * Inner Client
   */
 public:
  class GsmClientBC92 : public GsmClient {
    friend class TinyGsmBC92;

   public:
    GsmClientBC92() {}

    explicit GsmClientBC92(TinyGsmBC92& modem, uint8_t mux = 0) {
      ssl_sock = false;
      init(&modem, mux);
    }

    bool init(TinyGsmBC92* modem, uint8_t mux = 0) {
      this->at       = modem;
      sock_available = 0;
      prev_check     = 0;
      sock_connected = false;
      got_data       = false;

      if (mux < TINY_GSM_MUX_COUNT) {
        this->mux = mux;
      } else {
        this->mux = (mux % TINY_GSM_MUX_COUNT);
      }
      at->sockets[this->mux] = this;

      return true;
    }

    virtual int connect(const char* host, uint16_t port, int timeout_s) {
      stop();
      TINY_GSM_YIELD();
      rx.clear();
      sock_connected = at->modemConnect(host, port, mux, timeout_s);
      return sock_connected;
    }
    TINY_GSM_CLIENT_CONNECT_OVERRIDES

    virtual void stop(uint32_t maxWaitMs) {
      uint32_t startMillis = millis();
      dumpModemBuffer(maxWaitMs);
      at->sendAT(GF("+QICLOSE="), mux);
      sock_connected = false;
      at->waitResponse((maxWaitMs - (millis() - startMillis)));
    }
    void stop() override {
      stop(15000L);
    }

    /*
     * Extended API
     */

    String remoteIP() TINY_GSM_ATTR_NOT_IMPLEMENTED;

   protected:
    bool ssl_sock;
  };

  /*
   * Inner Secure Client
   */
 public:
  class GsmClientSecureBC92 : public GsmClientBC92 {
   public:
    GsmClientSecureBC92() {}

    explicit GsmClientSecureBC92(TinyGsmBC92& modem, uint8_t mux = 0)
        : GsmClientBC92(modem, mux) {
      ssl_sock = true;
    }

    bool setCertificate(const String& certificateName) {
      return at->setCertificate(certificateName, mux);
    }

    void stop(uint32_t maxWaitMs) override {
      uint32_t startMillis = millis();
      dumpModemBuffer(maxWaitMs);
      at->sendAT(GF("+QSSLCLOSE="), mux);
      sock_connected = false;
      at->waitResponse((maxWaitMs - (millis() - startMillis)));
    }
    void stop() override {
      stop(15000L);
    }
  };

  /*
   * Constructor
   */
 public:
  explicit TinyGsmBC92(Stream& stream) : stream(stream) {
    memset(sockets, 0, sizeof(sockets));
  }

  /*
   * Basic functions
   */
 protected:
  bool initImpl(const char* pin = nullptr) {
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);
    DBG(GF("### TinyGSM Compiled Module:  TinyGsmClientBC92"));

    if (!testAT()) { return false; }

    sendAT(GF("E0"));  // Echo Off
    if (waitResponse() != 1) { return false; }

#ifdef TINY_GSM_DEBUG
    sendAT(GF("+CMEE=2"));  // turn on verbose error codes
#else
    sendAT(GF("+CMEE=0"));  // turn off error codes
#endif
    waitResponse();

    DBG(GF("### Modem:"), getModemName());

    // Disable time and time zone URC's
    sendAT(GF("+CTZR=0"));
    if (waitResponse(10000L) != 1) { return false; }

    // Enable automatic time zone update
    sendAT(GF("+CTZU=1"));
    if (waitResponse(10000L) != 1) { return false; }

    SimStatus ret = getSimStatus();
    // if the sim isn't ready and a pin has been provided, try to unlock the sim
    if (ret != SIM_READY && pin != nullptr && strlen(pin) > 0) {
      simUnlock(pin);
      return (getSimStatus() == SIM_READY);
    } else {
      // if the sim is ready, or it's locked but no pin has been provided,
      // return true
      return (ret == SIM_READY || ret == SIM_LOCKED);
    }
  }

  /*
   * Power functions
   */
 protected:
  bool restartImpl(const char* pin = nullptr) {
    if (!testAT()) { return false; }
    if (!setPhoneFunctionality(1, true)) { return false; }
    waitResponse(10000L, GF("APP RDY"));
    return init(pin);
  }

  bool powerOffImpl() {
    sendAT(GF("+QPOWD=1"));
    waitResponse(300);  // returns OK first
    return waitResponse(300, GF("POWERED DOWN")) == 1;
  }

  // When entering into sleep mode is enabled, DTR is pulled up, and WAKEUP_IN
  // is pulled up, the module can directly enter into sleep mode.If entering
  // into sleep mode is enabled, DTR is pulled down, and WAKEUP_IN is pulled
  // down, there is a need to pull the DTR pin and the WAKEUP_IN pin up first,
  // and then the module can enter into sleep mode.
  bool sleepEnableImpl(bool enable = true) {
    sendAT(GF("+QSCLK="), enable);
    return waitResponse() == 1;
  }

  bool setPhoneFunctionalityImpl(uint8_t fun, bool reset = false,
                                 uint32_t timeout_ms = 15500L) {
    sendAT(GF("+CFUN="), fun, reset ? ",1" : "");
    return waitResponse(timeout_ms, GF("OK")) == 1;
  }

  /*
   * Generic network functions
   */
 public:
  BC92RegStatus getRegistrationStatus() {
    return (BC92RegStatus)getRegistrationStatusXREG("CEREG");
  }

  BC92RegStatus getRegistrationStatus2G() {
    return (BC92RegStatus)getRegistrationStatusXREG("CREG");
  }

 protected:
  bool isNetworkConnectedImpl() {
    BC92RegStatus s = getRegistrationStatus();
    return (s == REG_OK_HOME || s == REG_OK_ROAMING);
  }

  bool isNetworkConnected2gImpl() {
    BC92RegStatus s = getRegistrationStatus2G();
    return (s == REG_OK_HOME || s == REG_OK_ROAMING);
  }

  /*
   * Secure socket layer (SSL) functions
   */
  // Follows functions as inherited from TinyGsmSSL.tpp

  /*
   * WiFi functions
   */
  // No functions of this type supported

  /*
   * GPRS functions
   */
 protected:
  bool gprsConnectImpl(const char* apn, const char* user = nullptr,
                       const char* pwd = nullptr) {
    gprsDisconnect();

    sendAT(GF("+CGACT=1,1"));
    if (waitResponse(150000L) != 1) { return false; }
	  // Activate GPRS/CSD Context
    sendAT(GF("+CGPADDR=1"));
    if (waitResponse(60000L) != 1) { return false; }

    // Attach to Packet Domain service - is this necessary?
    sendAT(GF("+CGATT=1"));
    if (waitResponse(60000L) != 1) { return false; }

    return true;
  }

  bool gprsConnectImpl2g(const char* apn, const char* user = NULL,
                       const char* pwd = NULL) {
    gprsDisconnect();
    // sendAT(GF("+CGDCONT=1,\"IP\",\"M2MINTERNET\""));
    sendAT(GF("+CGDCONT=1,\"IPV4V6\",\""), apn, GF("\"")); // QUECTEL SIMCARD
    // sendAT(GF("+CGDCONT=1,\"IP\",\""), apn, GF("\""));
    if (waitResponse() != 1) { return false; }

    // Configure the TCPIP Context
    // sendAT(GF("+QICSGP=1,1,\""), apn, GF("\",\""), user, GF("\",\""), pwd,
    //        GF("\""));
    // if (waitResponse() != 1) { return false; }

    // Activate GPRS/CSD Context
    sendAT(GF("+CGACT=1,1"));
    if (waitResponse(150000L) != 1) { return false; }
	// Activate GPRS/CSD Context
    sendAT(GF("+CGPADDR=1"));
    if (waitResponse(60000L) != 1) { return false; }

    // Attach to Packet Domain service - is this necessary?
    sendAT(GF("+CGATT=1"));
    if (waitResponse(60000L) != 1) { return false; }

    return true;
  }

  bool gprsDisconnectImpl() {
    sendAT(GF("+CGACT=0"));  // Deactivate the bearer context
    if (waitResponse(40000L) != 1) { return false; }

    return true;
  }

  String getProviderImpl() {
    sendAT(GF("+QSPN?"));
    if (waitResponse(GF("+QSPN:")) != 1) { return ""; }
    streamSkipUntil('"');                      // Skip mode and format
    String res = stream.readStringUntil('"');  // read the provider
    waitResponse();                            // skip anything else
    return res;
  }

  /*
   * SIM card functions
   */
 protected:
  String getSimCCIDImpl() {
    sendAT(GF("+QCCID"));
    if (waitResponse(GF(AT_NL "+QCCID:")) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    waitResponse();
    res.trim();
    return res;
  }

  /*
   * Phone Call functions
   */
  // Follows all phone call functions as inherited from TinyGsmCalling.tpp

  /*
   * Audio functions
   */
  // No functions of this type supported

  /*
   * Text messaging (SMS) functions
   */
  // Follows all text messaging (SMS) functions as inherited from TinyGsmSMS.tpp

  /*
   * GSM Location functions
   */
 protected:
  // NOTE:  As of application firmware version 01.016.01.016 triangulated
  // locations can be obtained via the QuecLocator service and accompanying AT
  // commands.  As this is a separate paid service which I do not have access
  // to, I am not implementing it here.

  /*
   * GPS/GNSS/GLONASS location functions
   */
 protected:
  // enable GPS
  

  /*
   * Time functions
   */
 protected:
 
  /*
   * Client related functions
   */

 public:
  bool tcpConnect(const char* host, uint16_t port, uint8_t mux,
                    bool ssl = false, int timeout_s = 150) {
    if (ssl) { DBG("SSL not yet supported on this module!"); }

    uint32_t timeout_ms = ((uint32_t)timeout_s) * 1000;

    // <PDPcontextID>(1-16), <connectID>(0-11),
    // "TCP/UDP/TCP LISTENER/UDPSERVICE", "<IP_address>/<domain_name>",
    // <remote_port>,<local_port>,<access_mode>(0-2; 0=buffer)
    sendAT(GF("+QIOPEN=1,"), mux, GF(",\""), GF("TCP LISTENER"), GF("\",\""), host, GF("\",1,"), port, GF(",0"));
    // sendAT(GF("+QIOPEN=1,"), mux, GF(",\""), GF("TCP"), GF("\",\""), host, GF("\","), port, GF(",0,0"));
	// sendAT(GF("+QIOPEN=1,"), mux, GF(",\""), GF("TCP"), GF("\",\"139.59.219.194\",1883"), GF(",0,0"));
    waitResponse();

    if (waitResponse(timeout_ms, GF(AT_NL "+QIOPEN:")) != 1) { return false; }

    if (streamGetIntBefore(',') != mux) { return false; }
    // Read status
    return (0 == streamGetIntBefore('\n'));
  }
 
  int isNativeMqttConn(){
	  sendAT(GF("+QMTCONN?"));
    // if (waitResponse(1000L, GF("OK")) != 1) { return false; }
    if (waitResponse(4000L,GF("+QMTCONN:"))) { 
      goto FOUNDKEY;
    } else if (waitResponse(4000L,GF("+QMTCONN:"))) { 
      goto FOUNDKEY;
    } else if (waitResponse(4000L,GF("+QMTCONN:")) != 1) { 
      return false; 
    }
    FOUNDKEY:
	  // if (waitResponse(20000L,GF("+QMTCONN:")) != 1) { return false; }
    // streamSkipUntil(',');                  // Skip mux
      int8_t mux = streamGetIntBefore(',');
	    int8_t res = streamGetIntBefore('\n');  // socket state

    waitResponse();
	  if(res == 3){
		  return (3+mux); // MQTT connected
	  }else{
		  return 0; // MQTT not connected
	  }
    // return res;
  }

  bool nativeMqttOpen(uint8_t mux, String broker, String mqttport) {
    sendAT(GF("+QMTOPEN="), mux, ",\"", broker, "\",",mqttport);
	  if (waitResponse(1000L, GF("OK")) != 1) { return false; }
	  if (waitResponse(30000L,GF("+QMTOPEN:"))) { 
      goto FOUNDKEY3;
    } else if (waitResponse(30000L,GF("+QMTOPEN:"))) { 
      goto FOUNDKEY3;
    } else if (waitResponse(150000L,GF("+QMTOPEN:")) != 1) { 
      return false; 
    }
    FOUNDKEY3:

    streamSkipUntil(',');                  // Skip mux
    int8_t res = streamGetIntBefore('\n');  // socket state

    waitResponse();
	  return !res;
  }
  
  bool nativeMqttConn(uint8_t mux, String clid, String mqttuser, String mqttpass) {
    sendAT(GF("+QMTCONN="), mux, ",\"", clid, "\",\"",mqttuser ,"\",\"",mqttpass ,"\"");
	  if (waitResponse(2000L, GF("OK")) != 1) { return false; }
    // if (waitResponse(1000L, GF("+QMTSTAT")) == 1) { DBG("wrong state"); }
	  if (waitResponse(10000L,GF("+QMTCONN:"))) { 
      goto FOUNDKEY2;
    } else if (waitResponse(10000L,GF("+QMTCONN:"))) { 
      goto FOUNDKEY2;
    } else if (waitResponse(30000L,GF("+QMTCONN:")) != 1) { 
      return false; 
    }
    FOUNDKEY2: 
    streamSkipUntil(',');                  // Skip mux
	  streamSkipUntil(',');  //skip result
    int8_t res = streamGetIntBefore('\n');  // socket state

    waitResponse();
	  return !res;
  }
  // +QMTSUB=1,1,"topic/example",2
  int nativeMQTTSub(uint8_t mux, char *topicSub) {
	  sendAT(GF("+QMTSUB="), mux, ",1,\"", topicSub, "\",1");
	  if (waitResponse(2000L, GF("OK")) != 1) { return 2; }
	  if (waitResponse(40000L,GF("+QMTSUB:")) != 1) { return false; }
	  streamSkipUntil(',');                  // Skip mux
	  streamSkipUntil(',');                  // Skip msgid
	  int8_t res = streamGetIntBefore(',');
    streamSkipUntil('\n');
	  waitResponse();
	  if (res == 0){
	  	return 1;
	  } else {
	  	return 0;
	  }
  }
  int nativeMQTTSub(uint8_t mux, char *topicSub, char *topicSub2) {
	  sendAT(GF("+QMTSUB="), mux, ",1,\"", topicSub, "\",1,\"", topicSub2, "\",1");
	  if (waitResponse(2000L, GF("OK")) != 1) { return 2; }
	  if (waitResponse(40000L,GF("+QMTSUB:")) != 1) { return false; }
	  streamSkipUntil(',');                  // Skip mux
	  streamSkipUntil(',');                  // Skip msgid
	  int8_t res = streamGetIntBefore(',');
    streamSkipUntil('\n');
	  waitResponse();
	  if (res == 0){
	  	return 1;
	  } else {
	  	return 0;
	  }
  }
  // AT+QMTPUB=3,0,0,0,"topic/pub"
  bool nativeMQTTPub(uint8_t mux, char *topicPub, char *message) {
	  sendAT(GF("+QMTPUB="), mux, ",0,0,0,\"", topicPub, "\"");
	  if (waitResponse(300L, GF(">")) != 1) {
      streamWrite("",(char)26); 
      if (waitResponse(1000L, GF("OK")) != 1) { return false; }
      return false; 
    }
	  streamWrite(message,(char)26);
	  if (waitResponse(1000L, GF("OK")) != 1) { return false; }
	  if (waitResponse(1000L, GF("+QMTPUB:")) != 1) { return false; }
	  streamSkipUntil(',');                  // Skip mux
	  streamSkipUntil(','); 
	  int8_t res = streamGetIntBefore('\n');
	  waitResponse();
	  if (res == 0){
	  	return !res;
	  } else {
	  	return 1;
	  }
  }
  
  String handleNativeMQTT(){
	  if (unhandledFlag){
	    unhandledFlag = 0;
	    return unhandledData;
	  }
	  if (waitResponse(300L, GF("+QMTRECV:")) != 1) { return ""; }
	  streamSkipUntil(',');                  // Skip mux
	  streamSkipUntil(',');                  // Skip msgid
	  streamSkipUntil(',');                  // Skip topicsub
	  String res = stream.readStringUntil('\r');
    waitResponse();
    res.trim();
    return res;
	
  }

  bool handleUrcMqtt(uint8_t *pMux, uint8_t *resUrc){
    if (FLAG_QMT){
      FLAG_QMT = 0;
      *pMux = QMT_MUX;
      *resUrc = QMT_RES;
      return 1;
    } else {
      return 0;
    }

  }

 protected:
  bool modemConnect(const char* host, uint16_t port, uint8_t mux,
                    int timeout_s = 150) {
    uint32_t timeout_ms = ((uint32_t)timeout_s) * 1000;
    bool     ssl        = sockets[mux]->ssl_sock;

    if (ssl) {
    
      return false;
   
    } else {
      // AT+QIOPEN=1,0,"TCP","220.180.239.212",8009,0,0
      // <PDPcontextID>(1-16), <connectID>(0-11),
      // "TCP/UDP/TCP LISTENER/UDPSERVICE", "<IP_address>/<domain_name>",
      // <remote_port>,<local_port>,<access_mode>(0-2; 0=buffer)

      // sendAT(GF("+QIOPEN=1,"), mux, GF(",\""), GF("TCP"), GF("\",\""), host,
      //        GF("\","), port, GF(",0,0"));
      sendAT(GF("+QIOPEN=1,"), mux, GF(",\""), GF("TCP"), GF("\",\""), host,
             GF("\","), port, GF(",0,1"));
      waitResponse();

      if (waitResponse(timeout_ms, GF(AT_NL "+QIOPEN:")) != 1) { return false; }

      if (streamGetIntBefore(',') != mux) { return false; }
    }
    // Read status
    return (0 == streamGetIntBefore('\n'));
  }

  int16_t modemSend(const void* buff, size_t len, uint8_t mux) {
    bool ssl = sockets[mux]->ssl_sock;
    if (ssl) {
      sendAT(GF("+QSSLSEND="), mux, ',', (uint16_t)len);
    } else {
      sendAT(GF("+QISEND="), mux, ',', (uint16_t)len);
    }
    if (waitResponse(GF(">")) != 1) { return 0; }
    stream.write(reinterpret_cast<const uint8_t*>(buff), len);
    stream.flush();
    if (waitResponse(GF(AT_NL "SEND OK")) != 1) { return 0; }
    // TODO(?): Wait for ACK? (AT+QISEND=id,0 or AT+QSSLSEND=id,0)
    return len;
  }

  size_t modemRead(size_t size, uint8_t mux) {
    if (!sockets[mux]) return 0;
    bool ssl = sockets[mux]->ssl_sock;

    if (ssl) {
      return 0;
    } else {
      // sendAT(GF("+QIRD="), mux, ',', (uint16_t)size);
      // if (waitResponse(GF("+QIRD:")) != 1) { return 0; }

      if (waitResponse(GF("+QIURC:")) != 1) { return 0; }

    }
    int16_t len = streamGetIntBefore('\n');

    for (int i = 0; i < len; i++) { moveCharFromStreamToFifo(mux); }
    waitResponse();
    // DBG("### READ:", len, "from", mux);
    sockets[mux]->sock_available = modemGetAvailable(mux);
    return len;
  }

  size_t modemGetAvailable(uint8_t mux) {
    if (!sockets[mux]) return 0;
    bool   ssl    = sockets[mux]->ssl_sock;
    size_t result = 0;
    if (ssl) {
      sendAT(GF("+QSSLRECV="), mux, GF(",0"));
      if (waitResponse(GF("+QSSLRECV:")) == 1) {
        streamSkipUntil(',');  // Skip total received
        streamSkipUntil(',');  // Skip have read
        result = streamGetIntBefore('\n');
        if (result) { DBG("### DATA AVAILABLE:", result, "on", mux); }
        waitResponse();
      }
    } else {
      sendAT(GF("+QIRD="), mux, GF(",1024"));
      if (waitResponse(GF("+QIRD:")) == 1) {
        streamSkipUntil(',');  // Skip total received
        streamSkipUntil(',');  // Skip have read
        result = streamGetIntBefore('\n');
        if (result) { DBG("### DATA AVAILABLE:", result, "on", mux); }
        waitResponse();
      }
    }
    if (!result) { sockets[mux]->sock_connected = modemGetConnected(mux); }
    return result;
  }

  

  bool modemGetConnected(uint8_t mux) {
    bool ssl = sockets[mux]->ssl_sock;
    if (ssl) {
      sendAT(GF("+QSSLSTATE=1,"), mux);
      // +QSSLSTATE: 0,"TCP","151.139.237.11",80,5087,4,1,0,0,"uart1"

      if (waitResponse(GF("+QSSLSTATE:")) != 1) { return false; }

      streamSkipUntil(',');                  // Skip clientID
      streamSkipUntil(',');                  // Skip "SSLClient"
      streamSkipUntil(',');                  // Skip remote ip
      streamSkipUntil(',');                  // Skip remote port
      streamSkipUntil(',');                  // Skip local port
      int8_t res = streamGetIntBefore(',');  // socket state

      waitResponse();

      // 0 Initial, 1 Opening, 2 Connected, 3 Listening, 4 Closing
      return 2 == res;
    } else {
      sendAT(GF("+QISTATE=1,"), mux);
      // +QISTATE: 0,"TCP","151.139.237.11",80,5087,4,1,0,0,"uart1"

      if (waitResponse(GF("+QISTATE:")) != 1) { return false; }

      streamSkipUntil(',');                  // Skip mux
      streamSkipUntil(',');                  // Skip socket type
      streamSkipUntil(',');                  // Skip remote ip
      streamSkipUntil(',');                  // Skip remote port
      streamSkipUntil(',');                  // Skip local port
      int8_t res = streamGetIntBefore(',');  // socket state

      waitResponse();

      // 0 Initial, 1 Opening, 2 Connected, 3 Listening, 4 Closing
      return 2 == res;
    }
  }

  /*
   * Utilities
   */
 public:
  bool checkDrop(){
    return sockets[0]->sock_connected;
  }

  bool handleURCs(String& data) {
    if (data.endsWith(GF(AT_NL "+QIURC:"))) {
      streamSkipUntil('\"');
      String urc = stream.readStringUntil('\"');
      streamSkipUntil(',');
      if (urc == "recv") {
        // int8_t mux = streamGetIntBefore('\n');
        int8_t mux = streamGetIntBefore(',');
        uint16_t dtlen = streamGetIntBefore('\n');
        DBG("### URC RECV:", mux, "len:", dtlen);
        if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
          sockets[mux]->got_data = true;
          sockets[mux]->sock_available = dtlen;
        }
      } else if (urc == "closed") {
        int8_t mux = streamGetIntBefore('\n');
        DBG("### URC CLOSE:", mux);
        if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
          sockets[mux]->sock_connected = false;
        }
      } else {
        streamSkipUntil('\n');
      }
      data = "";
      return true;
    }
    return false;
  }

  bool handleQMTs(String& data) {
    if (data.endsWith(GF(AT_NL "+QMTRECV:"))) {
      streamSkipUntil(',');                  // Skip mux
		  streamSkipUntil(',');                  // Skip msgid
		  streamSkipUntil(',');                  // Skip topicsub
		  unhandledData = stream.readStringUntil('\n');
		  waitResponse();
		  // unhandledData.trim();
		  unhandledFlag = 1;
      DBG("### unhandledFlag:", unhandledFlag);
		  DBG("### +QMTRECV:", unhandledData);
      data = "";
      return true;
    }
    if (data.endsWith(GF(AT_NL "+QMTRECV:"))) {
      streamSkipUntil(','); // SKIP MUX
      int res = streamGetIntBefore('\n');
      DBG("### QMT:", res);
      FLAG_QMT = true;
      data = "";
      return true;
    }
    return false;
  }

 public:
  Stream& stream;

 protected:
  GsmClientBC92* sockets[TINY_GSM_MUX_COUNT];
  String         certificates[TINY_GSM_MUX_COUNT];
};

#endif  // SRC_TINYGSMCLIENTBC92_H_
