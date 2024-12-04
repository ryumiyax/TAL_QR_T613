all: setup
	@meson compile -C build
	@mkdir -p out/
	@mkdir -p out/plugins
	@cp dist/hidpos.dll out
	@cp build/qr_t613.dll out/plugins

clean-setup:
	@meson setup --wipe build --cross cross-mingw-64.txt

setup:
	@meson setup build --cross cross-mingw-64.txt
