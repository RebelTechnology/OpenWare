OPENWARE = $(CURDIR)

# Debug / Release
ifndef CONFIG
  CONFIG = Debug
endif

export OPENWARE CONFIG

all: tesseract microlab owlpedal player prism #quadfm 

tesseract:
	@$(MAKE) -C Tesseract all


microlab:
	@$(MAKE) -C MicroLab all

owlpedal:
	@$(MAKE) -C OwlPedal all

quadfm:
	@$(MAKE) -C QuadFM all

player:
	@$(MAKE) -C PlayerF7 all

prism:
	@$(MAKE) -C PrismF7 all

clean:
	@$(MAKE) -C Tesseract clean
	@$(MAKE) -C MicroLab clean
	@$(MAKE) -C OwlPedal clean
	@$(MAKE) -C PlayerF7 clean
	@$(MAKE) -C PrismF7 clean
	@$(MAKE) -C QuadFM clean


