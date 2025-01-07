VERSION_FILE   := VERSION
VERSION_STRING := $(file < $(VERSION_FILE))

ifeq ($(VERSION_STRING),)
  VERSION_STRING:=development-unreleased
endif
