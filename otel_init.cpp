#include "otel_init.hpp"

#include <opentelemetry/exporters/otlp/otlp_grpc_exporter.h>
#include <opentelemetry/sdk/trace/simple_processor.h>
#include <opentelemetry/sdk/trace/tracer_provider.h>
#include <opentelemetry/trace/provider.h>

static constexpr auto materialise_views = [](auto &...views) {
  return std::make_tuple(std::string{views}...);
};

auto init_otel_tracer(const std::string_view endpoint_view,
                      const std::string_view service_name_view)
    -> opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer> {
  namespace trace = opentelemetry::trace;
  namespace sdktrace = opentelemetry::sdk::trace;
  namespace otlp = opentelemetry::exporter::otlp;
  auto &&[endpoint, service_name] =
      materialise_views(endpoint_view, service_name_view);
  otlp::OtlpGrpcExporterOptions opts;
  opts.endpoint = endpoint;
  opts.use_ssl_credentials = false;

  auto exporter = std::make_unique<otlp::OtlpGrpcExporter>(opts);

  auto processor =
      std::make_unique<sdktrace::SimpleSpanProcessor>(std::move(exporter));

  auto resource = opentelemetry::sdk::resource::Resource::Create({
      {"service.name", service_name},
  });

  auto provider = opentelemetry::nostd::shared_ptr<trace::TracerProvider>(
      new sdktrace::TracerProvider(std::move(processor), resource));

  trace::Provider::SetTracerProvider(provider);
  return provider->GetTracer(endpoint);
}
