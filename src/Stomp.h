#ifndef STOMP_H
#define STOMP_H

#ifndef STOMP_MAX_COMMAND_HEADERS
#define STOMP_MAX_COMMAND_HEADERS 16
#endif

namespace Stomp {

/**
 * Enumeration of the acknowledgement mode specified by a client when it subscribes to a topic
 */
typedef enum {

  AUTO,
  CLIENT,
  CLIENT_INDIVIDUAL
} Stomp_AckMode_t;

/**
 * Enumeration of the values to be returned by a client message handler
 * ACK - return an ACK to the server
 * NACK - return a NACK to the server
 * CONTINUE - return nothing to the server. Used when the acknowledgement mode is CLIENT to allow batching of ACK/NACKs
 */
typedef enum {

  ACK,
  NACK,
  CONTINUE
} Stomp_Ack_t;

/**
 * The current state of the STOMP connection
 */
typedef enum {
  OPENING,
  CONNECTED,
  DISCONNECTING,
  DISCONNECTED
} Stomp_State_t;

typedef struct {
    String key;
    String value;
} StompHeader;

/**
 * A simple container class for StompHeaders
 * The define STOMP_MAX_COMMAND_HEADERS sets the maximum number of headers which can be stored for each received command.
 */
class StompHeaders {

  public:

    void append(String key, String value) {
      StompHeader h;
      h.key = key;
      h.value = value;
      append(h);
    }

  /**
   * Append a new header. Silently drop the header if STOMP_MAX_COMMAND_HEADERS is exceeded
   */
    void append(StompHeader h) {
      _idx++;
      if (_idx+1 >= STOMP_MAX_COMMAND_HEADERS) return;
      _headers[_idx] = h;
    }

    uint8_t size() {
      return _idx + 1;
    }

    StompHeader get(uint8_t idx) {
      return _headers[idx];
    }

    /**
     * Return the value of the header with the given key
     */
    String getValue(String key) {

      for (uint8_t i = 0; i < _idx; i++) {
        if (_headers[i].key.equals(key)) {
          return _headers[i].value;
        }
      }

      return "";
    }

  private:
    uint8_t _idx = -1;
    StompHeader _headers[STOMP_MAX_COMMAND_HEADERS];

};

typedef struct {
  String command;
  StompHeaders headers;
  String body;

} StompCommand;

/**
 * Signature of functions which handle incoming MESSAGEs
 */
typedef Stomp_Ack_t (*StompMessageHandler)(const StompCommand message);

/**
 * Signature of functions which handle other types of incoming command
 */
typedef void (*StompStateHandler)(const StompCommand message);

typedef struct {
  long id;
  StompMessageHandler messageHandler;
} StompSubscription;
}

#endif
