#include "otel_init.hpp"

#include <opentelemetry/sdk/trace/simple_processor.h>
#include <opentelemetry/sdk/trace/tracer_provider.h>
#include <opentelemetry/trace/provider.h>

auto init_otel_tracer(const std::string &endpoint, const std::string &service_name)
    -> std::shared_ptr<opentelemetry::trace::Tracer> {
  namespace trace     = opentelemetry::trace;
  namespace sdktrace  = opentelemetry::sdk::trace;
  namespace otlpexport = opentelemetry::exporter::otlp;

  otlpexport::OtlpGrpcExporterOptions options;
  options.endpoint = endpoint;
  options.use_ssl = false;

  auto exporter = std::make_unique<otlpexport::OtlpGrpcExporter>(options);
  auto processor = std::make_unique<sdktrace::SimpleSpanProcessor>(std::move(exporter));
  auto provider = std::make_shared<sdktrace::TracerProvider>(std::move(processor));

  trace::Provider::SetTracerProvider(provider);
  return provider->GetTracer(service_name);
}
