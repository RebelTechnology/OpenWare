OPENWARE = $(CURDIR)

export OPENWARE

all: tesseract microlab

tesseract:
	$(MAKE) -C Tesseract all


microlab:
	$(MAKE) -C MicroLab all

clean:
	$(MAKE) -C Tesseract clean
	$(MAKE) -C MicroLab clean


