OPENWARE = $(CURDIR)

# Debug / Release
ifndef CONFIG
  CONFIG = Debug
endif

export OPENWARE CONFIG

all: tesseract microlab minilab owlpedal player prism magus #quadfm 

tesseract:
	@$(MAKE) -C Tesseract all

microlab:
	@$(MAKE) -C MicroLab all

minilab:
	@$(MAKE) -C MiniLab all

owlpedal:
	@$(MAKE) -C OwlPedal all

quadfm:
	@$(MAKE) -C QuadFM all

player:
	@$(MAKE) -C PlayerF7 all

prism:
	@$(MAKE) -C PrismF7 all

magus:
	@$(MAKE) -C Magus all

clean:
	@$(MAKE) -C Tesseract clean
	@$(MAKE) -C MicroLab clean
	@$(MAKE) -C OwlPedal clean
	@$(MAKE) -C PlayerF7 clean
	@$(MAKE) -C PrismF7 clean
	@$(MAKE) -C Magus clean
	@$(MAKE) -C QuadFM clean


