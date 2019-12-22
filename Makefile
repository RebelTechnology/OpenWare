OPENWARE = $(CURDIR)

# Debug / Release
ifndef CONFIG
  CONFIG = Debug
endif

.PHONY: clean deploy-midiboot deploy-alchemist

export OPENWARE CONFIG

all: alchemist wizard magus tesseract prism effectsbox owlpedal player #quadfm owlboot ## build (almost) all targets

midiboot: ## build MidiBoot project
	@$(MAKE) -C MidiBoot all

tesseract: ## build Tesseract project
	@$(MAKE) -C Tesseract all

alchemist: ## build Alchemist project
	@$(MAKE) -C Alchemist all

wizard: ## build Wizard project
	@$(MAKE) -C Wizard all

owlpedal: ## build OWL Pedal project
	@$(MAKE) -C OwlPedal all

quadfm: ## build QuadFM project
	@$(MAKE) -C QuadFM all

player: ## build Player project
	@$(MAKE) -C PlayerF7 all

prism: ## build Prism project
	@$(MAKE) -C Prism all

magus: ## build Magus project
	@$(MAKE) -C Magus all

effectsbox: ## build EffectsBox project
	@$(MAKE) -C EffectsBox all

noctua: ## build Noctua project
	@$(MAKE) -C Noctua all

biosignals: ## build BioSignals project
	@$(MAKE) -C BioSignals all

clean: ## remove generated files
	@$(MAKE) -C Tesseract clean
	@$(MAKE) -C Alchemist clean
	@$(MAKE) -C Wizard clean
	@$(MAKE) -C OwlPedal clean
	@$(MAKE) -C PlayerF7 clean
	@$(MAKE) -C PrismF7 clean
	@$(MAKE) -C Prism clean
	@$(MAKE) -C Magus clean
	@$(MAKE) -C QuadFM clean
	@$(MAKE) -C EffectsBox clean
	@$(MAKE) -C Noctua clean
	@$(MAKE) -C BioSignals clean

docs: ## generate HTML documentation
	@doxygen Doxyfile

help: ## show this help
	@echo 'Usage: make [target] ...'
	@echo 'Targets:'
	@fgrep -h "##" $(MAKEFILE_LIST) | fgrep -v fgrep | sed -e  's/^\(.*\): .*##\(.*\)/\1:#\2/' | column -t -c 2 -s '#'

deploy-midiboot: ## flash device with bootloader
	@$(MAKE) -C MidiBoot deploy

deploy-alchemist: ## flash Alchemist firmware
	openocd -f Hardware/stm32f4.cfg -c "program Alchemist/Build/Alchemist.elf verify reset exit"
