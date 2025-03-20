from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout, CMakeToolchain, CMakeDeps


class Aspion(ConanFile):
    dependency_options = {
        "spdlog": {
            "use_std_fmt": True,
        },
        "opentelemetry":{
            "with_otlp_grpc": True,
            "with_otlp_http": True
        }
    }
    
    name = "aspion"
    version = "1.0"
    settings = "os", "compiler", "build_type", "arch"
    build_requires = ["ninja/1.12.1"]
    
    options = {
        "BUILD_TESTS": [True, False, None],
        "WX": [True, False, None],
    }
    default_options = {
        "BUILD_TESTS": True,
        "WX": True,
    }

    def requirements(self):
        self.requires("amqp-cpp/4.3.26")
        self.requires("boost/1.87.0")
        self.requires("lyra/1.6.1")
        self.requires("spdlog/1.15.1", options=self.dependency_options["spdlog"])
        self.requires("opentelemetry-cpp/1.18.0", options=self.dependency_options["opentelemetry"])

    def layout(self) -> None:
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self, generator="Ninja")
        
        tc.cache_variables["CMAKE_COMPILE_WARNING_AS_ERROR"] = "ON" if bool(self.options.WX) else "OFF"
        tc.cache_variables["BUILD_TESTS"] = "ON" if bool(self.options.BUILD_TESTS) else "OFF"
            
        tc.generate()

    def build(self) -> None:
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
