#include "util.hpp"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace Util {

auto generate_uuid() -> std::string {
  static boost::uuids::random_generator generator;
  return boost::uuids::to_string(boost::uuids::uuid(generator()));
}

} // namespace Util
