#pragma once

#include <opentelemetry/trace/tracer.h>
#include <memory>
#include <string>

auto init_otel_tracer(const std::string &endpoint, const std::string &service_name)
    -> std::shared_ptr<opentelemetry::trace::Tracer>;
