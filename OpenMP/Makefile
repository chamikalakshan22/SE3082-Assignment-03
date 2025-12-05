CC = gcc
CFLAGS = -Wall -O2 -Xclang -fopenmp
OPENSSL_PATH = /opt/homebrew/opt/openssl@3
LIBOMP_PATH = /opt/homebrew/opt/libomp
INCLUDES = -I$(OPENSSL_PATH)/include -I$(LIBOMP_PATH)/include
LIBS = -L$(OPENSSL_PATH)/lib -L$(LIBOMP_PATH)/lib -lssl -lcrypto -lomp

TARGET = omp_cracker
SOURCE = omp_cracker.c

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $(SOURCE) $(LIBS)

clean:
	rm -f $(TARGET)

.PHONY: clean