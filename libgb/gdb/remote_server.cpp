#include "remote_server.hpp"

#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <unistd.h>
#include <array>
#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <format>
#include <iostream>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

using namespace gb::gdb;

namespace {

constexpr size_t MaxPacketSize = 100UL * 1024UL;
constexpr std::string Unsupported{};

auto format_ip_addr(in_addr_t network_order_ip) -> std::string {
  const auto host_order_ip = ntohl(network_order_ip);
  return std::format("{}.{}.{}.{}", (host_order_ip >> 24U) & 0xffU,
                     (host_order_ip >> 16U) & 0xffU,
                     (host_order_ip >> 8U) & 0xffU,
                     (host_order_ip >> 0U) & 0xffU);
}

auto encode_as_hex(uint8_t byte) -> std::string {
  std::stringstream ss;
  ss << std::hex << (uint16_t)((byte >> 4U) & 0x0FU)
     << (uint16_t)(byte & 0x0FU);
  return ss.str();
}

auto encode_as_hex(uint16_t word) -> std::string {
  return encode_as_hex((uint8_t)(word & 0xFFU)) +
         encode_as_hex((uint8_t)(word >> 8U));
}

auto decode_hex_int(std::string_view number) -> ssize_t {
  return std::stol(std::string{number}, nullptr, 16);
}

auto decode_hex_string(std::string_view string) -> std::string {
  std::string result;
  result.reserve(string.length() / 2);
  for (size_t i = 0; i < string.length() / 2; i++) {
    result.append(1, (char)decode_hex_int(string.substr(i * 2, 2)));
  }
  return result;
}

auto encode_as_hex(std::string_view ascii) -> std::string {
  std::string result;
  result.reserve(2 * ascii.length());
  for (const auto digit : ascii) {
    result.append(encode_as_hex((uint8_t)digit));
  }
  return result;
}

auto calculate_checksum(std::string_view payload) -> std::string {
  uint8_t checksum = 0;
  for (const auto digit : payload) {
    checksum += (uint8_t)digit;
  }
  return encode_as_hex(checksum);
}

auto parse_int(std::string_view string, int base = 10) -> size_t {
  size_t result = 0;
  if (auto err = std::from_chars(string.data(), string.data() + string.size(),
                                 result, base);
      err.ec != std::errc{}) {
    throw std::runtime_error(std::format("Cannot parse int: {}", string));
  }
  return result;
}

}  // namespace

auto RemoteServer::wait_next_packet_raw() const -> std::string {
  std::array<uint8_t, MaxPacketSize> buffer;
  const auto size = recv(m_gdb_connection_fd, buffer.data(), buffer.size(), 0);
  if (size <= 0) {
    throw std::runtime_error("Socket dropped");
  }
  return {buffer.data(), buffer.data() + size};
}

auto RemoteServer::wait_next_packet() const -> std::string {
  auto packet = wait_next_packet_raw();
  std::cout << "-> " << packet << std::endl;

  if (packet.empty() || packet == "+") {
    return packet;
  }

  if (packet.at(0) == '+') {
    // Only support TCP in QStartNoAckMode, remove the Acks
    packet = packet.substr(1);
  }

  if (packet.at(0) != '$') {
    throw std::runtime_error(
        std::format("Could not parse packet '{}', expected '$'", packet));
  }
  if (packet.at(packet.length() - 3) != '#') {
    throw std::runtime_error(
        std::format("Could not parse packet '{}', expected hash", packet));
  }

  const auto payload = packet.substr(1, packet.length() - 4);
  const auto checksum = packet.substr(packet.length() - 2);

  if (checksum != calculate_checksum(payload)) {
    throw std::runtime_error(
        std::format("Received packet with an invalid checksum '{}'", packet));
  }
  return payload;
}

auto RemoteServer::send_ack_response() const -> void {
  // Special case but only needed until llvm has established QStartNoAckMode
  std::cout << "+" << std::endl;
  ::send(m_gdb_connection_fd, "+", 1, 0);
}

auto RemoteServer::send_response(std::string_view data) const -> void {
  std::string response;

  response.reserve(data.size() + 4);
  response.append("$");
  response.append(data);
  response.append("#");
  response.append(calculate_checksum(data));

  std::cout << "<- " << response << std::endl;
  ::send(m_gdb_connection_fd, response.data(), response.size(), 0);
}

auto RemoteServer::process_breakpoint_request(std::string_view query,
                                              bool is_add) -> void {
  if (not query.starts_with("1")) {
    // Only support hardware breakpoints
    send_response(Unsupported);
    return;
  }

  const auto start = query.find_first_of(',') + 1;
  const auto end = query.find_first_of(',', start);
  const auto addr_string = query.substr(start, end);

  size_t address = parse_int(addr_string, 16);
  if (is_add) {
    m_breakpoints.insert(address);
  } else {
    m_breakpoints.erase(address);
  }
  send_response("OK");
}

auto RemoteServer::process_H_request() -> void {
  // Requests that commands 'hx' is applied to a certain thread, ignore
  send_response("OK");
}

auto RemoteServer::process_m_request(std::string_view query) -> void {
  const auto delimiter = query.find_first_of(',');
  const auto addr_str = query.substr(0, delimiter);
  const auto size_str = query.substr(delimiter + 1);

  const auto result =
      read_memory(parse_int(addr_str, 16), parse_int(size_str, 16));

  std::string reply;
  for (const auto byte : result) {
    reply.append(encode_as_hex((uint8_t)byte));
  }
  send_response(reply);
}

auto RemoteServer::process_p_request(std::string_view query) -> void {
  auto value = read_register_value(decode_hex_int(query));
  if (value.has_value()) {
    send_response(encode_as_hex(value.value()));
  } else {
    send_response("E44");
  }
}

auto RemoteServer::process_q_request(std::string_view query) -> void {
  if (query == "HostInfo") {
    send_response("triple:" + encode_as_hex("gb-unknown-unknown") +
                  ";ptrsize:2;endian:little;");
    return;
  }

  if (query.starts_with("Supported")) {
    send_response(std::format(
        "PacketSize={};qXfer:memory-map:read-;QStartNoAckMode+;hwbreak+;"
        "qXfer:features:read+;",
        MaxPacketSize - 4));
    return;
  }

  if (query.starts_with("Symbol")) {
    send_response("OK");
    return;
  }

  constexpr std::string_view register_info = "RegisterInfo";
  if (query.starts_with(register_info)) {
    const auto reg_id = parse_int(query.substr(register_info.size()), 16);
    if (reg_id < m_register_names.size()) {
      const auto offset = 16 * reg_id;
      auto response = std::format(
          "name:{};bitsize:16;offset:{};encoding:uint;format:hex;"
          "set:General Purpose Registers;gcc:{};dwarf:{};",
          m_register_names[reg_id], offset, reg_id, reg_id);

      if (m_register_names[reg_id] == "pc") {
        response += "generic:pc;";
      }
      if (m_register_names[reg_id] == "sp") {
        response += "generic:sp";
      }
      send_response(response);
    } else {
      send_response("E45");
    }
    return;
  }

  if (query == "ProcessInfo") {
    if (is_attached()) {
      send_response("pid:1;parent-pid:1;endian:little;ptrsize:2;");
    } else {
      send_response("E44");
    }
    return;
  }

  if (query == "C") {
    if (is_attached()) {
      send_response("QC1");
    } else {
      send_response("E44");
    }
    return;
  }

  if (query == "fThreadInfo") {
    if (is_attached()) {
      // Send a list of threads, terminate with 'l'
      send_response("m1");
      const auto nextPacket = wait_next_packet();
      assert(nextPacket == "qsThreadInfo");
      send_response("l");
    } else {
      send_response("OK");
    }
    return;
  }

  send_response(Unsupported);
}

auto RemoteServer::process_Q_request(std::string_view query) -> void {
  if (query == "StartNoAckMode") {
    send_ack_response();
    send_response("OK");
    return;
  }

  if (query.starts_with("LaunchArch")) {
    if (not query.contains(":gb")) {
      throw std::runtime_error(std::format("Unknown arch request: {}", query));
    }
    send_response("OK");
    return;
  }

  send_response(Unsupported);
}

auto RemoteServer::process_qmark_request() -> void {
  send_response("S02");
}

auto RemoteServer::process_vrun_request(std::string_view query) -> void {
  query.remove_prefix(1);
  const auto file_name = decode_hex_string(query);
  run_elf(file_name);
  send_response("S00");
}

RemoteServer::~RemoteServer() {
  if (m_listen_fd != -1) {
    ::close(m_listen_fd);
  }
  if (m_gdb_connection_fd != 1) {
    ::close(m_gdb_connection_fd);
  }
}

auto RemoteServer::process_c_request(std::string_view query) -> void {
  std::optional<size_t> continue_addr;
  if (query.length() != 0) {
    continue_addr = parse_int(query, 16);
  }
  do_continue(continue_addr);
}

auto RemoteServer::process_s_request() -> void {
  m_is_in_step = true;
  do_continue(std::nullopt);
}

auto RemoteServer::process_request(std::string_view request) -> void {
  if (request.starts_with("vRun")) {
    process_vrun_request(request.substr(4));
    return;
  }
  switch (request.at(0)) {
    case 'Z':
      return process_breakpoint_request(request.substr(1), true);
    case 'z':
      return process_breakpoint_request(request.substr(1), false);
    case 'p':
      return process_p_request(request.substr(1));
    case 'Q':
      return process_Q_request(request.substr(1));
    case 'q':
      return process_q_request(request.substr(1));
    case '?':
      return process_qmark_request();
    case 'H':
      return process_H_request();
    case 'm':
      return process_m_request(request.substr(1));
    case 'c':
      return process_c_request(request.substr(1));
    case 's':
      return process_s_request();
    default:
      return send_response(Unsupported);
  }
}

auto RemoteServer::wait_for_connection(uint16_t port) -> void {
  m_listen_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (m_listen_fd <= 0) {
    throw std::runtime_error(
        std::format("Could not open socket, errno {}", errno));
  }

  const auto listen_addr = sockaddr_in{.sin_family = AF_INET,
                                       .sin_port = htons(port),
                                       .sin_addr = {INADDR_ANY},
                                       .sin_zero = {}};

  if (bind(m_listen_fd, (sockaddr*)&listen_addr, sizeof(listen_addr)) != 0) {
    throw std::runtime_error(
        std::format("Could not bind to port {} (errno {})", port, errno));
  }

  if (listen(m_listen_fd, 1) != 0) {
    throw std::runtime_error(
        std::format("Could not listen to port {} (errno {})", port, errno));
  }

  sockaddr_in connected_socket_addr = {};
  socklen_t connected_socket_addr_len = sizeof(connected_socket_addr);
  m_gdb_connection_fd = accept4(m_listen_fd, (sockaddr*)&connected_socket_addr,
                                &connected_socket_addr_len, SOCK_CLOEXEC);

  if (m_gdb_connection_fd < 0) {
    throw std::runtime_error(
        std::format("Not connection received on port {} (errno)", port, errno));
  }

  assert(connected_socket_addr_len == sizeof(connected_socket_addr) &&
         connected_socket_addr.sin_family == AF_INET);

  std::cout << "Accepted connection from: "
            << format_ip_addr(connected_socket_addr.sin_addr.s_addr)
            << std::endl;
}

auto RemoteServer::process_next_request() -> void {
  const auto request = wait_next_packet();
  if (request.empty() || request == "+") {
    return;  // Ack, nothing to do
  }
  process_request(request);
}

[[nodiscard]] auto RemoteServer::has_remote_interrupt_request() const -> bool {
  std::array<uint8_t, MaxPacketSize> buffer;
  const auto size =
      recv(m_gdb_connection_fd, buffer.data(), buffer.size(), MSG_DONTWAIT);
  if (size == -1) {
    // Nothing waiting in buffer, don't block
    return false;
  }

  const auto request =
      std::string_view{(const char*)buffer.data(), (size_t)size};

  if (request.size() == 1 && request[0] == '\x03') {
    std::cout << "-> ^C" << std::endl;
  } else {
    std::cout << "-> Unrecognized interrupt: " << encode_as_hex(request)
              << std::endl;
  }
  return true;
}

auto RemoteServer::is_active_breakpoint(size_t addr) const -> bool {
  return m_is_in_step || m_breakpoints.find(addr) != m_breakpoints.end();
}

auto RemoteServer::notify_break() -> void {
  m_is_in_step = false;  // Consume step

  std::stringstream ss;
  ss << "T02";  // SIGINT

  // Append register information
  for (size_t register_number = 0;; register_number++) {
    auto value = read_register_value(register_number);
    if (not value.has_value()) {
      break;
    }

    ss << std::hex << register_number << ":"
       << encode_as_hex((uint16_t)value.value()) << ";";
  }
  ss << "reason:breakpoint;";
  send_response(ss.str());
}
