#pragma once
#include <memory>
#include <string>

namespace tracing {

void init(std::string service_name = "quant-engine",
          std::string environment = "dev");

void shutdown();

class Tracer;
std::shared_ptr<Tracer> get(const std::string &name);

class Tracer {
  struct Impl;

public:
  class Span {
  public:
    Span(const Span &) = delete;
    Span &operator=(const Span &) = delete;

    void add_event(const std::string &name);
    std::string trace_id_hex() const;
    std::string span_id_hex() const;

    Span(Span &&) noexcept;
    Span &operator=(Span &&) noexcept;
    ~Span();

  private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    explicit Span(std::unique_ptr<Impl> p);
    friend class Tracer;
  };

  static std::shared_ptr<Tracer> create(std::shared_ptr<Impl> p) {
    return std::shared_ptr<Tracer>(new Tracer(std::move(p)));
  }

  Span start_span(const std::string &name);

private:
  std::shared_ptr<Impl> impl_;
  explicit Tracer(std::shared_ptr<Impl> p) : impl_(std::move(p)) {}
  friend std::shared_ptr<Tracer> get(const std::string &);
};

} // namespace tracing
