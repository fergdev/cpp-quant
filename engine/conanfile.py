from conan import ConanFile
from conan.tools.cmake import CMake, CMakeDeps, cmake_layout

required_conan_version = ">=2.0"


class QuantEngineRecipe(ConanFile):
    name = "quant_engine"
    version = "0.0.0"
    package_type = "application"
    settings = "os", "arch", "compiler", "build_type"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        self.requires("openssl/3.3.2")
        self.requires("nlohmann_json/3.11.3")
        self.requires("spdlog/1.15.3")
        self.requires("gtest/1.17.0")
        self.requires("boost/1.88.0")
        self.requires(
            "opentelemetry-cpp/1.22.0",
            options={
                "with_otlp_http": True,
                "with_otlp_grpc": False,
                "with_zipkin": False,
            },
        )

    def layout(self):
        cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
