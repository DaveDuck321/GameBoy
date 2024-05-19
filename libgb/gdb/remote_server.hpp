#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <optional>
#include <set>
#include <string_view>

namespace gb::gdb {

class RemoteServer {
  int m_listen_fd = -1;
  int m_gdb_connection_fd = -1;

  std::vector<std::string_view> m_register_names;

  bool m_is_in_step = false;
  std::set<size_t> m_breakpoints;

  std::function<std::optional<uint16_t>(size_t)> read_register_value;
  std::function<std::vector<uint8_t>(size_t, size_t)> read_memory;
  std::function<void(std::string_view)> run_elf;
  std::function<bool()> is_attached;
  std::function<void(std::optional<size_t>)> do_continue;

  [[nodiscard]] auto wait_next_packet_raw() const -> std::string;
  [[nodiscard]] auto wait_next_packet() const -> std::string;

  auto send_ack_response() const -> void;
  auto send_response(std::string_view data) const -> void;

  auto process_breakpoint_request(std::string_view query, bool is_add) -> void;
  auto process_H_request() -> void;
  auto process_p_request(std::string_view query) -> void;
  auto process_q_request(std::string_view query) -> void;
  auto process_Q_request(std::string_view query) -> void;
  auto process_qmark_request() -> void;
  auto process_vrun_request(std::string_view query) -> void;
  auto process_m_request(std::string_view query) -> void;
  auto process_c_request(std::string_view query) -> void;
  auto process_s_request() -> void;

  auto process_request(std::string_view request) -> void;

 public:
  RemoteServer(std::initializer_list<std::string_view> register_names)
      : m_register_names(register_names){};
  RemoteServer(const RemoteServer&) = delete;
  auto operator=(const RemoteServer&) -> RemoteServer& = delete;
  ~RemoteServer();

  auto wait_for_connection(uint16_t port) -> void;
  auto process_next_request() -> void;
  [[nodiscard]] auto has_remote_interrupt_request() const -> bool;

  [[nodiscard]] auto is_active_breakpoint(size_t addr) const -> bool;
  auto notify_break(bool is_breakpoint) -> void;

  template <typename Fn>
  auto add_read_register_value_callback(Fn&& fn) -> void {
    read_register_value = std::forward<Fn>(fn);
  }

  template <typename Fn>
  auto add_run_elf_callback(Fn&& fn) -> void {
    run_elf = std::forward<Fn>(fn);
  }

  template <typename Fn>
  auto add_is_attached_callback(Fn&& fn) -> void {
    is_attached = std::forward<Fn>(fn);
  }

  template <typename Fn>
  auto add_read_memory_callback(Fn&& fn) -> void {
    read_memory = std::forward<Fn>(fn);
  }

  template <typename Fn>
  auto add_do_continue_callback(Fn&& fn) -> void {
    do_continue = std::forward<Fn>(fn);
  }
};

}  // namespace gb::gdb
