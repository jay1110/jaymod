PROJECT/ ?= $(PROJECT)/

PROJECT.rar = $(BUILD/)$(PROJECT.packageBase).rar

###############################################################################

include $(PROJECT/)make/docbook.mk
include $(PROJECT/)make/fn-xterm.mk
include $(PROJECT/)make/fn.mk

include $(PROJECT/)make/platform/$(PROJECT.platformNamef)

ifneq ($(VARIANT),)
include $(PROJECT/)make/variant/$(PROJECT.platformNamef)-$(VARIANT)
endif

include $(MODULES:%=$(PROJECT/)%.defs)

###############################################################################

# Derived from the platform key (PROJECT.platformNamef) rather than the
# human-readable PROJECT.platformName, so that arch-suffixed platforms such as
# linux-aarch64, osx64 and osx-arm64 still select the correct source subdirs.
PROJECT.platspecific = unknown
ifneq ($(filter linux%,$(PROJECT.platformNamef)),)
    PROJECT.platspecific = linux
endif
ifneq ($(filter osx%,$(PROJECT.platformNamef)),)
    PROJECT.platspecific = osx
endif
ifneq ($(filter mingw% windows%,$(PROJECT.platformNamef)),)
    PROJECT.platspecific = win32
endif
# Android (Bionic) and Emscripten both provide a POSIX environment, so they
# reuse the linux platform-specific sources.
ifneq ($(filter android% wasm%,$(PROJECT.platformNamef)),)
    PROJECT.platspecific = linux
endif

###############################################################################

BUILD.output += $(BUILD/)make/project.mk
BUILD.output += $(BUILD/)m4/project.m4
BUILD.output += $(PROJECT.rar)

BUILD.dirs = $(sort $(dir $(BUILD.output)))

###############################################################################

.DELETE_ON_ERROR:
.SUFFIXES:

###############################################################################

.PHONY: clean pak.clean pkg.clean
.PHONY: all default debug release
.PHONY: specials world
.PHONY: pkg doc

###############################################################################

all:: $(BUILD.dirs) $(BUILD/)m4/project.m4

clean::
	rm -fr $(BUILD/)

cleanall::
	rm -rf build*

$(BUILD.dirs):
	$(call fn.mkdir,$@)

###############################################################################

default::
	$(MAKE) pkg VARIANT=

debug::
	$(MAKE) pkg VARIANT=debug

release::
	$(MAKE) pkg VARIANT=release

###############################################################################

doc:: $(BUILD.dirs)

###############################################################################

$(BUILD/)m4/project.m4: $(PROJECT/)project/info.py $(PROJECT/)project/info.db
	$(call print.HEADER,GENERATING,$@)
	@mkdir -p $(dir $@)
	@$< -m4 $(PROJECT/)project/info.db > $@

###############################################################################

include $(MODULES:%=$(PROJECT/)%.rules)
