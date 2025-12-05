CC = gcc
CFLAGS = -Wall -O2
OPENSSL_PATH = /opt/homebrew/opt/openssl@3
INCLUDES = -I$(OPENSSL_PATH)/include
LIBS = -L$(OPENSSL_PATH)/lib -lssl -lcrypto

TARGET = serial_cracker
SOURCE = serial_cracker.c

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $(SOURCE) $(LIBS)

clean:
	rm -f $(TARGET)

.PHONY: clean