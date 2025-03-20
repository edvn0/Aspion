#include <opentelemetry/exporters/otlp/otlp_grpc_exporter.h>
#include <opentelemetry/sdk/trace/simple_processor.h>
#include <opentelemetry/sdk/trace/tracer_provider.h>
#include <opentelemetry/trace/provider.h>

auto init_otel_tracer(const std::string &endpoint,
                      const std::string &service_name)
    -> opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer> {
  namespace trace = opentelemetry::trace;
  namespace sdktrace = opentelemetry::sdk::trace;
  namespace otlp = opentelemetry::exporter::otlp;
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
  return provider->GetTracer(service_name);
}
