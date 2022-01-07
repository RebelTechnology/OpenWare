22.2.0
------

* Improved USB Audio FIFO allocation
* Refactored Graphics code
* Move screen handling to separate task
* Added patch text/message handlers
* Changed loop/setup callbacks to onLoop and onSetup
* Support for OWL Pedal mk2
* Prevent invalid characters in reported patch names
* Send resource size and checksum with name
* Improved backwards patch compatibility (CCM linked stack)


22.1.rc2
------

* Process USB messages each audio block, before patch runs (or in loop if no patch is loaded)


22.1.rc1
------

* Fix USBD audio sync issues:
* rx asynchronous mode with explicit feedback
* tx asynchronous mode with variable output size
* Fix USBD audio descriptor issues (Windows support)

Known Issues:
* USB Host does not support reconnecting devices


22.0.0
------

* Prevent rewriting resources and patches
* Provide resource size and CRC with name
* Support SPI flash storage
* Rewritten storage code
* Support for new Witch device

Known Issues:
* USB Host does not support reconnecting devices
* USB Audio might 'click' due to synchronisation with host

21.2.2
------

* Fix mode button and LED issue in Wizard
* Capitalise all USB device names

21.2.1
------

* Change hardware ID for legacy devices to prevent parameter inversion

21.2.0
------

* Display patch slots up to 16 on Lich (in hexadecimal)
* Bootloader flash/lock/unlock functionality
* New Pin class for GPIO handling
* Refactored code with project-specific callbacks
* Magus TRS MIDI output enabled
* Updated Magus LED handling, new LED brightness menu
* Update parameters with MIDI messages from USBH
* USB devices renamed OWL-MAGUS, OWL-WIZARD et c.
* MIDI command to retrieve resource names and sizes
* MIDI command to delete resources


21.1.0
------

* Added MIDI commands for resource SAVE, NAME and DELETE


21.0.0
------

* Updated USB audio driver
* Fix USB audio mic driver Windows support
* Added PATCH_BUTTON_ON/OFF MIDI controls
* Don't raise errors for incomplete SysEx messages (Mac OS)
* Added patch resource support
* Refactor subproject code and Owl class
* Added background tasks
* Created background tasks for MIDI SysEx message batches
* New Magus resource UI


20.10.0
-------

* First public Lich release
* New Library submodule
* Refactored build structure
* Refactored Owl.cpp into subprojects
* Improved USB Host performance

20.9.0
------

* Add USB audio streaming drivers
* New Noctua project
* New Lich project
* New Witch project

20.8.0
------

* Add watchdog support (automatically reset if firmware hangs)

20.7.0
------

* Fix a potential deadlock issue causing the firmware to hang indefinitely on startup (due to disabled interrupts).
* Add encoder sensitivity setting for Magus

20.6.0
------

* Fix an encoder direction reset problem in Magus (thanks to @antisvin)
* Upgrade the supporting CMSIS libraries
* Refactor debug support

20.5.0
------

* Fix longstanding USB Host performance problem (NAK flood).
* First public Magus release

20.4.0
------

* Alchemist and Wizard release
* Fix reset cycle problem caused by erroneous defaul patch

20.3.0
------

* Beta release of OpenWare for OWL Pedal and OWL Modular
* Flexible patch storage with no pre-compiled 'factory' patches

20.2.0
------

* Updates to USB Host implementation, device ID query, code refactor and bug fixes.
* MidiBoot fixed flash erase functions.

20.0.0
------

* First release of the OpenWare firmware as shipped on Alchemist and Wizard Kickstarter rewards
