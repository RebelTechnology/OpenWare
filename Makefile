OPENWARE = $(CURDIR)

# Debug / Release
ifndef CONFIG
  CONFIG = Debug
endif

export OPENWARE CONFIG

all: tesseract microlab owlpedal

tesseract:
	@$(MAKE) -C Tesseract all


microlab:
	@$(MAKE) -C MicroLab all

owlpedal:
	@$(MAKE) -C OwlPedal all

clean:
	@$(MAKE) -C Tesseract clean
	@$(MAKE) -C MicroLab clean
	@$(MAKE) -C OwlPedal clean


