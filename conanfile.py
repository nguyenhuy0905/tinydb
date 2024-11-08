from conan import ConanFile
from conan.tools.cmake import cmake_layout

# docs: https://docs.conan.io/2/tutorial/consuming_packages/the_flexibility_of_conanfile_py.html

class SampleLibRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"
    options = {
            "install_cmake": [True, False],
            "install_ninja": [True, False],
            "install_ccache": [True, False],
            "install_make": [True, False],
    }

    default_options = {
            "install_cmake": False,
            "install_ninja": False,
            "install_ccache": False,
            "install_make": False,
    }

    # find packages in https://conan.io/center

    def requirements(self):
        # add requirements as needed
        self.requires("gtest/1.14.0")
        self.requires("benchmark/1.8.4")

    def build_requirements(self):
        # bleeding edge stuff.
        if self.options.install_cmake:
            self.tool_requires("cmake/3.30.0")
        
        if self.options.install_ninja:
            self.tool_requires("ninja/1.12.1")

        if self.options.install_ccache:
            self.tool_requires("ccache/4.10")

        if self.options.install_make:
            self.tool_requires("make/4.4.1")

    def layout(self):
        cmake_layout(self)
