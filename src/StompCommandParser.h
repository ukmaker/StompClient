#ifndef STOMP_MESSAGE_PARSER_H
#define STOMP_MESSAGE_PARSER_H

#include "Stomp.h"

namespace Stomp {


class StompCommandParser {

  public:

    static StompCommand parse(String data) {

      // command EOL
      // * (header EOL)
      // EOL
      // * Octet
      // NULL
      // * (EOL)

      String EOL = "\\n";
      String EOL2 = "\\n\\n";

      int headersStart = data.indexOf(EOL);
      int bodyStart = data.indexOf(EOL2);

      StompCommand cmd;
      String headers;
      int start = 0;
      int end = 0;

      if (headersStart == -1) {
        cmd.command = data;
      } else {
        cmd.command = data.substring(0, headersStart);
        headersStart += EOL.length();
      }
      cmd.command.trim();

      if (bodyStart == -1) {
        headers = data.substring(headersStart);
        cmd.body = "";
      } else {
        headers = data.substring(headersStart, bodyStart);

        bodyStart += EOL2.length();
        cmd.body = data.substring(bodyStart);
      }

      headers.trim();
      cmd.body.trim();

      String header;

      while (start < headers.length()) {
        end = headers.indexOf(EOL, start);
        if (end == -1) {
          header = headers.substring(start);
          start = headers.length();
        } else {
          header = headers.substring(start, end);
          start = end + EOL.length();
        }
        header.trim();
        // now split it into key and value
        int idx = header.indexOf(":");
        if (idx != -1) {
          StompHeader h;
          h.key = header.substring(0, idx);
          h.key.trim();
          h.value = header.substring(idx + 1);
          h.value.trim();
          cmd.headers.append(h);
        }
      }

      return cmd;
    }
};

}
#endif

