#######################################################################
# Convenient commands
#
# For Windows users: good luck trying to run this without MSYS2/Cygwin.
#######################################################################

override MAKEFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
override MAKEFILE_DIR := $(patsubst %/Makefile,%,$(MAKEFILE_PATH))
override BUILD_DIR := $(MAKEFILE_DIR)/build
BUILD_TYPE := Debug
CONAN_OPTIONS := "install_cmake=False install_ccache=False"
override CONAN_OPTIONS_CMDLINE := $(foreach opt,$(CONAN_OPTIONS),$(patsubst %,-o options/all:%,$(opt)))

COLOUR_OUTPUT := ON

ifeq ("$(COLOUR_OUTPUT)", "ON")
override COLOUR_GREEN := \033[32m
override COLOUR_CYAN := \033[36m
override STYLE_BOLD := \033[1m
override COLOUR_AND_STYLE_RESET := \033[0m
endif

# install packages as configured in conanfile.py
.PHONY: conan-install
conan-install:
	conan install . --build=missing \
		-s build_type=${CMAKE_BUILD_TYPE} \
		$(CONAN_OPTIONS_CMDLINE)
	@make conan-venv-help

# list custom options inside the conanfile.py
.PHONY: conan-option
conan-option:
	@echo
	@printf "$(COLOUR_GREEN)Here are a list of options and their defaults$(COLOUR_AND_STYLE_RESET)\n\n"
	@printf "$(STYLE_BOLD)"
	@cat conanfile.py | \
		tr '\n' ' ' | \
		grep -E -o "(default_)?options = {.*}" | \
	sed -E "s/[ ]{2,}/\\n/g" | \
		sed -E "s/(.*):/\t\1:/g"
	@printf "$(COLOUR_AND_STYLE_RESET)"
	@echo

.PHONY: conan-venv-help
conan-venv-help:
	@echo
	@printf "$(COLOUR_GREEN)To activate Conan's environment, run the script\
$(COLOUR_AND_STYLE_RESET): $(STYLE_BOLD)$(shell find $(MAKEFILE_DIR) -name "conanbuild.*")\n$(COLOUR_AND_STYLE_RESET)"
	@printf "$(COLOUR_GREEN)To deactivate Conan's environment, run the script\
$(COLOUR_AND_STYLE_RESET): $(STYLE_BOLD)$(shell find $(MAKEFILE_DIR) -name "deactivate_conanbuild.*")\n$(COLOUR_AND_STYLE_RESET)"
	@echo
