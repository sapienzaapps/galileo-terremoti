
ARDUINO=/opt/arduino-1.6.0+Intel/arduino

all:
	$(info No default task available - please issue # make gen1  or  # make gen2)

gen1:
	arduino --verify --verbose-build --board intel:i586-uclibc:izmir_fd --pref build.path=build --pref update.check=false galileo-terremoti.ino

gen2:
	arduino --verify --verbose-build --board intel:i586-uclibc:izmir_fg --pref build.path=build --pref update.check=false galileo-terremoti.ino

clean:
	rm -rf build/
