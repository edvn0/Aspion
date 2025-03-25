#pragma once

#include <memory>
#include <opentelemetry/trace/tracer.h>
#include <string_view>

auto init_otel_tracer(std::string_view endpoint, std::string_view service_name)
    -> opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer>;
