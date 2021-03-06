#include "http2.h"
#include <iostream>
#include <functional>
#include "buffer_view.h"
#include "string_util_join.h"

void dump_settings_payload(const http2_frame::payload_type& payload) {
  auto buffer = buffer_view(&payload[0], payload.size());

  std::size_t offset = 0;

  while (buffer.test_range(offset, 6)) {
    uint16_t id = buffer.at_uint16(offset);
    uint32_t value = buffer.at_uint32(offset + 2);

    switch (id) {
    case 1:
      std::cout << "SETTINGS_HEADER_TABLE_SIZE: ";
      break;
    case 2:
      std::cout << "SETTINGS_ENABLE_PUSH: ";
      break;
    case 3:
      std::cout << "SETTINGS_MAX_CONCURRENT_STREAMS: ";
      break;
    case 4:
      std::cout << "SETTINGS_INITIAL_WINDOW_SIZE: ";
      break;
    case 5:
      std::cout << "SETTINGS_MAX_FRAME_SIZE: ";
      break;
    case 6:
      std::cout << "SETTINGS_MAX_HEADER_LIST_SIZE: ";
      break;
    default:
      std::cout << "Unknown: ";
    }
    std::cout << value << std::endl;

    offset += 6;
  }
}

std::string stringify_error_code(uint32_t code) {
  switch (code) {
  case NO_ERROR:
    return std::string("NO ERROR");
  case PROTOCOL_ERROR:
    return std::string("PROTOCOL ERROR");
  case INTERNAL_ERROR:
    return std::string("INTERNAL ERROR");
  case FLOW_CONTROL_ERROR:
    return std::string("FLOW CONTROL ERROR");
  case SETTINGS_TIMEOUT:
    return std::string("SETTINGS TIMEOUT");
  case STREAM_CLOSED:
    return std::string("STREAM CLOSED");
  case FRAME_SIZE_ERROR:
    return std::string("FRAME SIZE ERROR");
  case REFUSED_STREAM:
    return std::string("REFUSED STREAM");
  case CANCEL:
    return std::string("CANCEL");
  case COMPRESSION_ERROR:
    return std::string("COMPRESSION ERROR");
  case CONNECT_ERROR:
    return std::string("CONNECT ERROR");
  case ENHANCE_YOUR_CALM:
    return std::string("ENHANCE YOUR CALM");
  case INADEQUATE_SECURITY:
    return std::string("INADEQUATE SECURITY");
  case HTTP_1_1_REQUIRED:
    return std::string("HTTP1.1 REQUIRED");
  default:
    return std::string("Unknown");
  }
}

void dump_rst_stream_payload(const http2_frame::payload_type& payload) {
  auto buffer = buffer_view(&payload[0], payload.size());

  uint32_t error_code = buffer.at_uint32(0);

  std::cout << "Error Code: " << stringify_error_code(error_code) << std::endl;
}

void dump(const http2_frame& frame) {
  auto type = frame.type();
  std::string type_str;
  std::vector<std::string> flags;
  std::function<void(const http2_frame::payload_type&)> dump_payload;

  switch (type) {
  case http2_frame_header::TYPE_DATA:
    type_str = "DATA";
    if (frame.flags() & 0x01) { flags.push_back("END_STREAM"); }
    if (frame.flags() & 0x08) { flags.push_back("PADDED"); }
    break;
  case http2_frame_header::TYPE_HEADERS:
    type_str = "HEADERS";
    if (frame.flags() & 0x01) { flags.push_back("END_STREAM"); }
    if (frame.flags() & 0x04) { flags.push_back("END_HEADERS"); }
    if (frame.flags() & 0x08) { flags.push_back("PADDED"); }
    if (frame.flags() & 0x20) { flags.push_back("PRIORITY"); }
    break;
  case http2_frame_header::TYPE_PRIORITY:
    type_str = "PRIORITY";
    break;
  case http2_frame_header::TYPE_RST_STREAM:
    type_str = "RST_STREAM";
    dump_payload = dump_rst_stream_payload;
    break;
  case http2_frame_header::TYPE_SETTINGS:
    type_str = "SETTINGS";
    if (frame.flags() & 0x01) { flags.push_back("ACK"); }
    dump_payload = dump_settings_payload;
    break;
  case http2_frame_header::TYPE_PUSH_PROMISE:
    type_str = "PUSH_PROMISE";
    break;
  case http2_frame_header::TYPE_PING:
    type_str = "PING";
    break;
  case http2_frame_header::TYPE_GOAWAY:
    type_str = "GOAWAY";
    break;
  case http2_frame_header::TYPE_WINDOW_UPDATE:
    type_str = "WINDOW_UPDATE";
    break;
  case http2_frame_header::TYPE_CONTINUATION:
    type_str = "CONTINUATION";
    break;
  default:
    type_str = "Type: " + type;
    break;
  }

  std::cout << type_str;
  std::cout << std::endl;

  std::cout << "Header:";
  if (frame.flags()) {
    std::cout << " (Flags:" << string_util::join(",", flags) << ")";
  }
  std::cout << std::endl;

  dump(&frame.header(), sizeof(http2_frame_header));
  if (frame.payload().size() > 0) {
    std::cout << "Payload:" << std::endl;
    dump(frame.payload_buffer(), frame.payload().size(), 200);
    if (dump_payload) {
      dump_payload(frame.payload());
    }
  }
  std::cout << std::endl;
}



