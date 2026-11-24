# METACONFORMER

**An intelligent MIDI router, merger, splitter and processor.**

Metaconformer is a programmable MIDI processor designed to make complex hardware synthesizer setups easier to build.

A standard MIDI channel provides access to 128 notes, but most hardware instruments have a much more limited playable range. Metaconformer solves this problem by intelligently distributing notes across multiple instruments, allowing synthesizers with different note ranges to work together as a single instrument.

For example, you can assign one octave to a drum machine, two octaves to a bass synthesizer and three octaves to a lead synthesizer. These zones can overlap, and each zone has an independent transposition setting, allowing overlapping instruments to play in different octaves rather than competing in the same register.

Metaconformer features two MIDI inputs and four MIDI outputs, with several operating modes.

## Features

### Splitter

Divides the keyboard into up to eight overlapping note zones, each assigned to a different MIDI channel.

Each zone has its own note range and transposition settings. This makes it possible to distribute different parts of a keyboard across multiple synthesizers, combine instruments with different playable ranges, and create complex layered setups.

### Combiner

Combines multiple synthesizers into one large polyphonic instrument.

Metaconformer distributes incoming notes between multiple instruments, allowing them to function together as a single polyphonic synthesizer.

### Translator

Converts incoming MIDI Note messages into MIDI CC messages.

This can be used to implement key tracking and other unconventional ways of controlling synthesizers using a keyboard.

### Sequencer (undocumented)

An experimental operating mode featuring eight independent 16-step sequencers.

Sequencers can be triggered using the device's physical buttons or an external button adapter.

### MIDI Merge and MIDI Thru

MIDI merging and thru functionality are available in all operating modes.

Messages arriving on MIDI channels not used by the active processing algorithm are passed through to the outputs.

## Configuration

Metaconformer uses a MIDI Learn-based configuration procedure.

Instead of manually entering MIDI channels, note numbers or controller assignments, the user follows a short sequence of actions and sends MIDI notes or control messages. The device automatically captures the required information.

For detailed operating instructions, see the [user manual](https://somasynths.com/ru-metaconformer/).

## Hardware

Metaconformer is based on the STM32F446 microcontroller.

Hardware features:

- STM32F446 microcontroller.
- AT45DB081 external flash memory for preset storage and USB mass storage functionality.
- 2 MIDI inputs.
- 4 MIDI outputs.
- One MIDI output can be switched to a DAC output for clock generation.
- Analog switches for selecting TRS MIDI Type A / Type B.
- Physical buttons and status LEDs.
- External control connector electrically connected to the button inputs.

The TRS MIDI Type A / Type B selection switches are hardware-controlled and cannot be configured through firmware.

Hardware schematics are included in this repository.

For more information about the device, including demonstrations and operating instructions, visit the [Metaconformer product page](https://somasynths.com/ru-metaconformer/).

## Firmware

The firmware is written in C and developed using STM32CubeIDE.

It uses the standard STM32 GCC toolchain and does not depend on external software libraries.

The project can also be built using VSCode or another development environment with an appropriate STM32 toolchain.

### Firmware Upload

There are two ways to upload firmware:

- **USB Bootloader:** The device comes with a factory-installed USB bootloader that allows firmware updates without an external programmer.
- **ST-Link:** Firmware can also be uploaded directly using an ST-Link programmer.

**Important: Do not erase or overwrite the factory-installed USB bootloader.**

The bootloader source code is not included in this repository. If the bootloader is erased or overwritten, firmware updates will require an external ST-Link programmer unless you implement and install your own bootloader.

## Firmware Architecture

MIDI input data is received through UART interrupts and placed into dedicated input queues.

MIDI processing takes place in the main loop according to the operating mode selected during device startup. Processed messages are placed into output queues and transmitted through UART interrupts.

Button events are handled using interrupts to ensure minimal latency for sequencer-related functions.

The firmware also handles preset storage and USB mass storage functionality using the external AT45DB081 flash memory.

## Development Status

**Discontinued. No longer actively maintained.**

Metaconformer is a finished hardware project. I have moved on to other projects and no longer plan to develop or maintain its firmware.

The source code is published as-is, in the hope that it will be useful to other developers, hardware enthusiasts and people interested in embedded programming.

The code is not necessarily an example of perfect software architecture, but it is a functional embedded project rather than a collection of disconnected experiments.

If you are learning embedded programming, Metaconformer might be an interesting project to explore. It demonstrates UART communication, interrupt-driven data handling, message queues, MIDI processing, GPIO interrupts and interaction with external hardware.

There are also things that could be improved. For example, combining the Splitter and Combiner algorithms into a single operating mode would open up some interesting possibilities. I have considered doing this, but never got around to implementing it.

Feel free to study, modify and build upon the code. Just don't expect further updates or technical support from me.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
