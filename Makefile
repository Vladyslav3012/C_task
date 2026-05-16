# Збірка з каталогу app/  (LAUNCHER_HOME = поточний каталог)

CC         = gcc
CFLAGS     = -Wall -Wextra -std=c11 -O2
INCLUDES   = -Iinclude
SRC_CORE   = src/core/programs.c src/core/process.c
SRC_GUI    = src/launcher/gui.c
SRC_CLI    = src/launcher/cli.c
BINDIR     = bin
OUTDIR     = output

ifeq ($(OS),Windows_NT)
  EXE     = .exe
  RM      = cmd /C del /Q /F
  RMDIR   = cmd /C if exist $(BINDIR) rmdir /S /Q $(BINDIR) & if exist $(OUTDIR) rmdir /S /Q $(OUTDIR)
  MKDIR_P = mkdir
  RUN_LAUNCHER = set LAUNCHER_HOME=%CD%&& $(BINDIR)/launcher$(EXE)
else
  EXE     =
  RM      = rm -f
  RMDIR   = rm -rf
  MKDIR_P = mkdir -p
  RUN_LAUNCHER = LAUNCHER_HOME=$$(pwd) ./$(BINDIR)/launcher$(EXE)
endif

GTK_CFLAGS := $(shell pkg-config --cflags gtk+-3.0 2>/dev/null)
GTK_LIBS   := $(shell pkg-config --libs gtk+-3.0 2>/dev/null)

MODULES    = demo gold_stub

.PHONY: all clean run run-cli check-gtk dirs

all: check-gtk dirs $(BINDIR)/launcher$(EXE) $(BINDIR)/launcher-cli$(EXE) \
	$(foreach m,$(MODULES),$(BINDIR)/$(m)$(EXE))

check-gtk:
ifeq ($(OS),Windows_NT)
	@if [ -z "$(GTK_CFLAGS)" ]; then \
		echo "Windows: відкрийте MSYS2 UCRT64 і виконайте:"; \
		echo "  pacman -S mingw-w64-ucrt-x86_64-toolchain mingw-w64-ucrt-x86_64-gtk3 make"; \
		exit 1; \
	fi
else
	@if [ -z "$(GTK_CFLAGS)" ]; then \
		echo "Linux: sudo apt install libgtk-3-dev"; \
		exit 1; \
	fi
endif

dirs: $(BINDIR) $(OUTDIR)

$(BINDIR) $(OUTDIR):
	$(MKDIR_P) $@

$(BINDIR)/launcher$(EXE): $(SRC_GUI) $(SRC_CORE) include/programs.h include/process.h | dirs
	$(CC) $(CFLAGS) $(INCLUDES) $(GTK_CFLAGS) -o $@ $(SRC_GUI) $(SRC_CORE) $(GTK_LIBS)

$(BINDIR)/launcher-cli$(EXE): $(SRC_CLI) $(SRC_CORE) include/programs.h include/process.h | dirs
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $(SRC_CLI) $(SRC_CORE)

$(BINDIR)/demo$(EXE): modules/demo/main.c | dirs
	$(CC) $(CFLAGS) -o $@ $<

$(BINDIR)/gold_stub$(EXE): modules/gold_stub/main.c | dirs
	$(CC) $(CFLAGS) -o $@ $<

run: all
	$(RUN_LAUNCHER)

run-cli: dirs $(BINDIR)/launcher-cli$(EXE) $(foreach m,$(MODULES),$(BINDIR)/$(m)$(EXE))
ifeq ($(OS),Windows_NT)
	@set LAUNCHER_HOME=%CD%&& $(BINDIR)/launcher-cli$(EXE)
else
	@LAUNCHER_HOME=$$(pwd) ./$(BINDIR)/launcher-cli$(EXE)
endif

clean:
	$(RMDIR) $(BINDIR) $(OUTDIR) 2>/dev/null || true
