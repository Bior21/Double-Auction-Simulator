# Makefile for Double Auction Simulator
#
#   make          build the game (double_auction)
#   make run      build and play
#   make test     build and run every test
#   make web      build the web version into docs/ for GitHub Pages
#   make clean    remove what make built

CXX = clang++
RAYLIB = $(shell brew --prefix raylib)
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Iheaders -I$(RAYLIB)/include
LIBS = -L$(RAYLIB)/lib -lraylib

# The .h files live in headers/ and the .cpp files live in sources/.
SOURCES = sources/main.cpp sources/App.cpp sources/Drawing.cpp sources/People.cpp \
          sources/Layout.cpp sources/Market.cpp sources/Sale.cpp sources/Trader.cpp \
          sources/Sorting.cpp sources/Days.cpp

all: double_auction

double_auction: $(SOURCES) headers/*.h
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $@ $(LIBS)

run: double_auction
	./double_auction

test: double_auction
	./double_auction --test

# Web version for GitHub Pages. Builds the game into docs/ (index.html,
# index.js, index.wasm, index.data). Needs Emscripten (brew install emscripten)
# and raylib built for the web, kept in RAYLIB_WEB.
RAYLIB_WEB ?= $(HOME)/.local/raylib-web
WEB_FLAGS = -std=c++17 -Os -Wall -Iheaders -I$(RAYLIB_WEB)/include -DPLATFORM_WEB \
            -sUSE_GLFW=3 -sASYNCIFY -sALLOW_MEMORY_GROWTH \
            --preload-file fonts --exclude-file "*.txt" --shell-file web/shell.html

web: $(SOURCES) headers/*.h web/shell.html
	mkdir -p docs
	em++ $(WEB_FLAGS) $(SOURCES) $(RAYLIB_WEB)/lib/libraylib.web.a -o docs/index.html

clean:
	rm -f double_auction

.PHONY: all run test web clean
