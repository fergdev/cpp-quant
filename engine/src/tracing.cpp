#include "tracing.hpp"

#include <opentelemetry/exporters/otlp/otlp_http_exporter_factory.h>
#include <opentelemetry/sdk/resource/resource.h>
#include <opentelemetry/sdk/trace/batch_span_processor_factory.h>
#include <opentelemetry/sdk/trace/tracer_provider_factory.h>
#include <opentelemetry/trace/provider.h>

#include <memory>
#include <utility>

namespace otel = opentelemetry;
namespace trace = otel::trace;
namespace sdktr = otel::sdk::trace;
namespace sdkrs = otel::sdk::resource;
namespace otlp = otel::exporter::otlp;

namespace tracing {

static inline sdkrs::Resource make_resource(std::string service_name,
                                            std::string environment) {
  return sdkrs::Resource::Create({
      {"service.name", std::move(service_name)},
      {"deployment.environment", std::move(environment)},
  });
}

void init(std::string service_name, std::string environment) {
  if (trace::Provider::GetTracerProvider())
    return;

  auto options = opentelemetry::sdk::trace::BatchSpanProcessorOptions{};
  auto exporter = otlp::OtlpHttpExporterFactory::Create();
  auto processor =
      sdktr::BatchSpanProcessorFactory::Create(std::move(exporter), options);

  auto up = sdktr::TracerProviderFactory::Create(
      std::move(processor),
      make_resource(std::move(service_name), std::move(environment)));

  std::shared_ptr<trace::TracerProvider> sp =
      std::shared_ptr<sdktr::TracerProvider>(std::move(up));

  trace::Provider::SetTracerProvider(sp);
}

void shutdown() {
  std::shared_ptr<trace::TracerProvider> none;
  trace::Provider::SetTracerProvider(none);
}
struct Tracer::Impl {
  explicit Impl(opentelemetry::nostd::shared_ptr<trace::Tracer> t)
      : otel_tracer(std::move(t)) {}
  opentelemetry::nostd::shared_ptr<trace::Tracer> otel_tracer;
};

struct Tracer::Span::Impl {
  explicit Impl(opentelemetry::nostd::shared_ptr<trace::Span> s)
      : span(std::move(s)), scope(std::make_unique<trace::Scope>(span)) {}
  opentelemetry::nostd::shared_ptr<trace::Span> span;
  std::unique_ptr<trace::Scope> scope;
};

std::shared_ptr<tracing::Tracer> get(const std::string &name) {
  auto prov = trace::Provider::GetTracerProvider();

  opentelemetry::nostd::shared_ptr<trace::Tracer> otel_tr =
      prov->GetTracer(name);

  return Tracer::create(std::make_shared<Tracer::Impl>(std::move(otel_tr)));
}

Tracer::Span Tracer::start_span(const std::string &name) {
  auto s = impl_->otel_tracer->StartSpan(name);
  return Tracer::Span(std::make_unique<Tracer::Span::Impl>(std::move(s)));
}

namespace {
template <size_t N, typename IdT> std::string id_to_hex(IdT id) {
  std::array<char, N> buf{};
  opentelemetry::nostd::span<char, N> dest(buf);
  id.ToLowerBase16(dest);
  return std::string(buf.data(), buf.size());
}

} // namespace

static inline std::string to_hex(opentelemetry::trace::TraceId id) {
  constexpr size_t N = opentelemetry::trace::TraceId::kSize * 2;
  return id_to_hex<N>(id);
}

static inline std::string to_hex(opentelemetry::trace::SpanId id) {
  constexpr size_t N = opentelemetry::trace::SpanId::kSize * 2;
  return id_to_hex<N>(id);
}

void Tracer::Span::add_event(const std::string &name) {
  if (impl_ && impl_->span)
    impl_->span->AddEvent(name);
}
std::string Tracer::Span::trace_id_hex() const {
  if (!impl_ || !impl_->span)
    return {};
  return to_hex(impl_->span->GetContext().trace_id());
}
std::string Tracer::Span::span_id_hex() const {
  if (!impl_ || !impl_->span)
    return {};
  return to_hex(impl_->span->GetContext().span_id());
}

Tracer::Span::Span(std::unique_ptr<Impl> p) : impl_(std::move(p)) {}
Tracer::Span::Span(Span &&other) noexcept : impl_(std::move(other.impl_)) {}
Tracer::Span &Tracer::Span::operator=(Span &&other) noexcept {
  if (this != &other)
    impl_ = std::move(other.impl_);
  return *this;
}
Tracer::Span::~Span() {
  if (impl_ && impl_->span)
    impl_->span->End();
}

} // namespace tracing
