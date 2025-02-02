![WIP](https://img.shields.io/badge/ESP_IDF-V5.3.2-blue)
![WIP](https://img.shields.io/badge/target-ESP32_S3-blue)
![WIP](https://img.shields.io/badge/CLIPS-V6.4.1-blue)
![WIP](https://img.shields.io/badge/status-WIP-orange)

# CLIPS-ArduinoNanoESP32
A porting of [CLIPS](https://sourceforge.net/projects/clipsrules/) to [Arduino Nano ESP32](https://docs.arduino.cc/hardware/nano-esp32/).

# About CLIPS

Developed at NASA’s Johnson Space Center from 1985 to 1996, the ‘C’ Language Integrated Production System (CLIPS) is a rule-based programming language useful for creating expert systems and other programs where a heuristic solution is easier to implement and maintain than an algorithmic solution. Written in C for portability, CLIPS can be installed and used on a wide variety of platforms.

Since its first release in 1986, CLIPS has undergone
continual refinement and improvement and it is now used by thousands of people around the world.
Since 1996, CLIPS has been available as public domain software.

CLIPS is designed to facilitate the development of software to model human knowledge or
expertise.
There are three ways to represent knowledge in CLIPS:
- Rules, which are primarily intended for heuristic knowledge based on experience.
- Deffunctions and generic functions, which are primarily intended for procedural knowledge.
- Object-oriented programming, also primarily intended for procedural knowledge. The five
generally accepted features of object-oriented programming are supported: classes,
message-handlers, abstraction, encapsulation, inheritance, and polymorphism. Rules may
pattern match on objects and facts.
You can develop software using only rules, only objects, or a mixture of objects and rules.

CLIPS programs may be executed in three ways: interactively using a simple Read-Eval-Print Loop (REPL) interface; interactively using an Integrated Development Environment (IDE) interface; or as embedded application in which the user provides a main program and controls execution of the expert system through the CLIPS Application Programming Interface (API).

# About this project

This project is a software porting, a mapping of the Read-Eval-Print Loop with the standard loop operation of Arduino and a COOL wrapper of the Arduino API (work in progress). This enables endless application possibilities in the field of IoT and automation, as well as benefits such as code portability. COOL coding!

> This project is a prototype and is still under development. While it works, it has not been thoroughly tested and may not be stable. **Not recommended for production use.**

> All elements, parts, libraries, interfaces, code snippets, tools, and components used in this software are protected by their respective original copyrights; this also applies to the CLIPS software.

# Arduino COOL functions

- [pin-mode](https://docs.arduino.cc/language-reference/en/functions/digital-io/pinMode/)

    `(pin-mode D5 INPUT_PULLUP)`

    An instance of the class PIN will be created;

- pin-reset

    `(pin-reset D5)`

    The instance will be deleted;

- [digital-read](https://docs.arduino.cc/language-reference/en/functions/digital-io/digitalread/)

    `(digital-read D5)`

- [digital-write](https://docs.arduino.cc/language-reference/en/functions/digital-io/digitalwrite/)

    `(digital-write D5 HIGH)`

- [wifi-status](https://docs.arduino.cc/libraries/wifi/#%60WiFi.status()%60)

    `(wifi-status)`

- wifi-connect

    `(wifi-connect <ssid> <password>)`

    It wraps https://docs.arduino.cc/libraries/wifi/#%60WiFi.begin()%60