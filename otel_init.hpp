#pragma once

#include <memory>
#include <opentelemetry/trace/tracer.h>
#include <string>

auto init_otel_tracer(const std::string &endpoint,
                      const std::string &service_name)
    -> opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer>;
