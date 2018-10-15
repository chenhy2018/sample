// Last Update:2018-06-12 19:14:06
/**
 * @file sdk_interface.h
 * @brief 
 * @author 
 * @version 0.1.00
 * @date 2018-05-25
 */

#ifndef SDK_INTERFACE_H
#define SDK_INTERFACE_H

#include <stdint.h>

typedef enum {
        CALL_STATUS_REGISTERED,
        CALL_STATUS_INCOMING,
        CALL_STATUS_ESTABLISHED,
        CALL_STATUS_RING,
        CALL_STATUS_REJECT,
        CALL_STATUS_HANGUP,
        CALL_STATUS_ERROR
} CallStatus;

typedef enum {
        MESSAGE_STATUS_CONNECT,
        MESSAGE_STATUS_DATA,
        MESSAGE_STATUS_DISCONNECT,
        MESSAGE_STATUS_ERROR
} MessageStatus;

typedef enum {
        EVENT_CALL,
        EVENT_MESSAGE,
} EventType;

typedef enum {
        RET_OK,
        RET_FAIL,
        RET_INTERAL_FAIL,
        RET_REGISTERING,
        RET_RETRY,
        RET_MEM_ERROR = 1001,
        RET_PARAM_ERROR,
        RET_INIT_ERROR,
        RET_ACCOUNT_NOT_EXIST,
        RET_CALL_NOT_EXIST,
        RET_REGISTER_TIMEOUT,
        RET_TIMEOUT_FROM_SERVER,
        RET_USER_UNAUTHORIZED,
        RET_CALL_INVAILD_CONNECTION,
        RET_CALL_INVAILD_OPERATING,
        RET_SDK_ALREADY_INITED,
        MESSAGE_ERR_NOMEM = 3003,
        MESSAGE_ERR_PROTOCOL,
        MESSAGE_ERR_INVAL,
        MESSAGE_ERR_NO_CONN,
        MESSAGE_ERR_CONN_REFUSED,
        MESSAGE_ERR_NOT_FOUND,
        MESSAGE_ERR_CONN_LOST,
        MESSAGE_ERR_TLS,
        MESSAGE_ERR_PAYLOAD_SIZE,
        MESSAGE_ERR_NOT_SUPPORTED,
        MESSAGE_ERR_AUTH,
        MESSAGE_ERR_ACL_DENIED,
        MESSAGE_ERR_UNKNOWN,
        MESSAGE_ERR_ERRNO,
        MESSAGE_ERR_EAI,
        MESSAGE_ERR_PROXY,
        MESSAGE_ERR_CONN_PENDING,
        MESSAGE_ERR_OTHERS
} ErrorID;

typedef enum {
        REASON_OK = 0,
        REASON_REGISTERING_FAIL = 101,
        REASON_CALL_BAD_REQUEST = 400,
        REASON_CALL_UNAUTHORIZED = 401,
        REASON_CALL_PAYMENT_REQUIRED = 402,
        REASON_CALL_FORBIDDEN = 403,
        REASON_CALL_NOT_FOUND = 404,
        REASON_CALL_REQUEST_TIMEOUT = 408,
        REASON_CALL_GONE = 410,
        REASON_CALL_REQUEST_TERMINATED = 487,
        REASON_CALL_BAD_EVENT = 489,
        REASON_CALL_INTERNAL_SERVER_ERROR = 500,
        REASON_CALL_BAD_GATEWAY = 502,
        REASON_CALL_SERVICE_UNAVAILABLE = 503,
        REASON_CALL_DECLINE = 603
} ReasonCode;

typedef enum {
        STREAM_NONE,
        STREAM_AUDIO,
        STREAM_VIDEO,
        STREAM_DATA
} Stream;

typedef enum {
    CODEC_NONE,
    CODEC_H264,
    CODEC_G711A,
    CODEC_G711U,
} Codec;

typedef int AccountID;

#define URL_LEN_MAX (128)
#define STREAM_PACKET_LEN (256)
#define AUDIO_CODEC_MAX 16
#define VIDEO_CODEC_MAX 16

// Define logging levels
#define LOG_FATAL    0    // A fatal error has occured: program will exit immediately
#define LOG_ERROR    1    // An error has occured: program may not exit
#define LOG_INFO     2     // Nessessary information regarding program operation
#define LOG_WARN     3     // Any circumstance that may not affect normal operation
#define LOG_DEBUG    4     // Standard debug messages
#define LOG_VERBOSE  5     // All debug messages

typedef enum {
        LOG_PRINT = 0,
        LOG_FILE  //Todo.
} LOG_MODE;

void SetLogLevel(int level);
void SetLogMode(LOG_MODE mode);

// for test::
void setPjLogLevel(int level);

typedef struct {
    int callID;
    CallStatus status;
    ReasonCode reasonCode;
    void *context;
} CallEvent;

typedef struct {
    char *message;
    MessageStatus status;
    char *topic;
} MessageEvent;

typedef struct {
    Stream streamType;
    Codec codecType;
    int sampleRate;
    int channels;
} Media;

typedef struct {
    Media media[2];
    int nCount;
} MediaInfo;

typedef struct {
    int eventID;
    int time;
    EventType type;
    union {
        CallEvent callEvent;
        MessageEvent messageEvent;
    } body;
} Event;

// library initial, application only need to call one time
ErrorID InitSDK( Media* mediaConfigs, int size);
ErrorID UninitSDK();
// register a account
// @return if return value > 0, it is the account id, if < 0, it is the ErrorID
AccountID Register(const char* id, const char* password, const char* sigHost, const char* imHost);

ErrorID UnRegister( AccountID id );
// make a call, user need to save call id
ErrorID MakeCall(AccountID accountID, const char* id, const char* host, int* callID);
ErrorID AnswerCall( AccountID id, int nCallId );
ErrorID RejectCall( AccountID id, int nCallId );
// hangup a call
ErrorID HangupCall( AccountID id, int nCallId );
// poll a event
// if the EventData have video or audio data
// the user shuould copy the the packet data as soon as possible
ErrorID PollEvent(AccountID id, EventType* type, Event** event, int timeOut);

// mqtt report
ErrorID Report(AccountID id, const char* topic, const char* message, int length);
ErrorID Subscribe(AccountID id, const char* topic);
ErrorID Unsubscribe(AccountID id, const char* topic);

#endif  /*SDK_INTERFACE_H*/

