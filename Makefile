OPENWARE = $(CURDIR)

# Debug / Release
ifndef CONFIG
  CONFIG = Debug
endif

export OPENWARE CONFIG

all: tesseract alchemist minilab owlpedal player prism magus effectsbox #quadfm 

tesseract:
	@$(MAKE) -C Tesseract all

alchemist:
	@$(MAKE) -C Alchemist all

minilab:
	@$(MAKE) -C MiniLab all

owlpedal:
	@$(MAKE) -C OwlPedal all

quadfm:
	@$(MAKE) -C QuadFM all

player:
	@$(MAKE) -C PlayerF7 all

prism:
	@$(MAKE) -C Prism all

magus:
	@$(MAKE) -C Magus all

effectsbox:
	@$(MAKE) -C EffectsBox all

clean: ## remove generated files
	@$(MAKE) -C Tesseract clean
	@$(MAKE) -C Alchemist clean
	@$(MAKE) -C OwlPedal clean
	@$(MAKE) -C PlayerF7 clean
	@$(MAKE) -C PrismF7 clean
	@$(MAKE) -C Prism clean
	@$(MAKE) -C Magus clean
	@$(MAKE) -C QuadFM clean
	@$(MAKE) -C EffectsBox clean

docs: ## generate HTML documentation
	@doxygen Doxyfile

help: ## show this help
	@echo 'Usage: make [target] ...'
	@echo 'Targets:'
	@fgrep -h "##" $(MAKEFILE_LIST) | fgrep -v fgrep | sed -e  's/^\(.*\): .*##\(.*\)/\1:#\2/' | column -t -c 2 -s '#'


