# Збірка з каталогу app/  (LAUNCHER_HOME = поточний каталог)

CC         = gcc
CFLAGS     = -Wall -Wextra -std=c11 -O2
INCLUDES   = -Iinclude
SRC_CORE   = src/core/programs.c
SRC_GUI    = src/launcher/gui.c
SRC_CLI    = src/launcher/cli.c
BINDIR     = bin
OUTDIR     = output

GTK_CFLAGS := $(shell pkg-config --cflags gtk+-3.0 2>/dev/null)
GTK_LIBS   := $(shell pkg-config --libs gtk+-3.0 2>/dev/null)

MODULES    = demo gold_stub

.PHONY: all clean run run-cli check-gtk dirs

all: check-gtk dirs $(BINDIR)/launcher $(BINDIR)/launcher-cli \
	$(foreach m,$(MODULES),$(BINDIR)/$(m))

check-gtk:
	@if [ -z "$(GTK_CFLAGS)" ]; then \
		echo "Потрібно: sudo apt install libgtk-3-dev"; \
		exit 1; \
	fi

dirs: $(BINDIR) $(OUTDIR)

$(BINDIR) $(OUTDIR):
	mkdir -p $@

$(BINDIR)/launcher: $(SRC_GUI) $(SRC_CORE) include/programs.h | dirs
	$(CC) $(CFLAGS) $(INCLUDES) $(GTK_CFLAGS) -o $@ $(SRC_GUI) $(SRC_CORE) $(GTK_LIBS)

$(BINDIR)/launcher-cli: $(SRC_CLI) $(SRC_CORE) include/programs.h | dirs
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $(SRC_CLI) $(SRC_CORE)

$(BINDIR)/demo: modules/demo/main.c | dirs
	$(CC) $(CFLAGS) -o $@ $<

$(BINDIR)/gold_stub: modules/gold_stub/main.c | dirs
	$(CC) $(CFLAGS) -o $@ $<

run: all
	LAUNCHER_HOME=$$(pwd) ./$(BINDIR)/launcher

run-cli: dirs $(BINDIR)/launcher-cli $(foreach m,$(MODULES),$(BINDIR)/$(m))
	LAUNCHER_HOME=$$(pwd) ./$(BINDIR)/launcher-cli

clean:
	rm -rf $(BINDIR) $(OUTDIR)
