PREFIX = @@prefix@@

.PHONY: all clean install uninstall

all:
	mkdir -p build && cd build && qmake ../player.pro
	$(MAKE) -C build

clean:
	rm -rf build

install:
	install -d ${PREFIX}/bin
	install -s build/@@player_name@@ ${PREFIX}/bin/@@player_name@@
	install -d ${PREFIX}/share/icons
	install -m 644 player.icon ${PREFIX}/share/icons/@@player_name@@.icon
	install -d ${PREFIX}/share/applications
	install -m 644 player.desktop ${PREFIX}/share/applications/@@player_name@@.desktop

uninstall:
	rm -f ${PREFIX}/bin/@@player_name@@
	rm -f ${PREFIX}/share/icons/@@player_name@@.icon
	rm -f ${PREFIX}/share/applications/@@player_name@@.desktop
