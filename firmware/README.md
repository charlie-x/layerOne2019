# linux
  sudo apt-get install gcc-avr binutils-avr avr-libc -y

# For programming
  sudo apt-get install avrdude


# OSX
  brew tap osx-cross/avr
  brew install avr-libc
  
# For programming
  brew install avrdude --with-usb


# Windows
  Install AVR studio or Atmel Studio , load project, compile and program
  

# OSX / Linux
  avr-gcc -g -Os -mmcu=attiny2313a larson.c -DF_CPU=8000000  -o larson.elf
  avr-objcopy -j .text -j .data -O ihex larson.elf larson.hex
  
  
  # Fuses are default
