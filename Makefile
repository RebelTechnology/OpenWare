OPENWARE = $(CURDIR)

# Debug / Release
ifndef CONFIG
  CONFIG = Release
endif

.PHONY: clean

# To avoid problems on case insensitive filesystems, mark all targets named the same as a directory as phony
.PHONY: alchemist wizard magus lich owlpedal quadfm player prism effectsbox noctua biosignals witch midiboot1 midiboot2 midiboot3 tesseract genius acdc xdk

export OPENWARE CONFIG

all: alchemist wizard magus lich witch owlpedal genius biosignals acdc xdk midiboot1 midiboot2 midiboot3 # effectsbox tesseract prism player quadfm noctua  ## build most targets

midiboot1: ## build MidiBoot1 project
	@$(MAKE) -C MidiBoot1 all

midiboot2: ## build MidiBoot2 project
	@$(MAKE) -C MidiBoot2 all

midiboot3: ## build MidiBoot3 project
	@$(MAKE) -C MidiBoot3 all

tesseract: ## build Tesseract project
	@$(MAKE) -C Tesseract all

genius: ## build Genius project
	@$(MAKE) -C Genius all

acdc: ## build ACDC project
	@$(MAKE) -C ACDC all

xdk: ## build XibecaDevKit project
	@$(MAKE) -C XibecaDevKit all

witch: ## build Witch project
	@$(MAKE) -C Witch all

lich: ## build Lich project
	@$(MAKE) -C Lich all

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
	@$(MAKE) -C Witch clean
	@$(MAKE) -C Lich clean
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
	@$(MAKE) -C Genius clean
	@$(MAKE) -C ACDC clean
	@$(MAKE) -C XibecaDevKit clean
	@$(MAKE) -C MidiBoot1 clean
	@$(MAKE) -C MidiBoot2 clean
	@$(MAKE) -C MidiBoot3 clean

docs: ## generate HTML documentation
	@doxygen Doxyfile

help: ## show this help
	@echo 'Usage: make [target] ...'
	@echo 'Targets:'
	@fgrep -h "##" $(MAKEFILE_LIST) | fgrep -v fgrep | sed -e  's/^\(.*\): .*##\(.*\)/\1:#\2/' | column -t -c 2 -s '#'
