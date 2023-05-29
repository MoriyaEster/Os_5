CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -lm

TARGET = st_pipeline
SOURCE = st_pipeline.c

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGET)
