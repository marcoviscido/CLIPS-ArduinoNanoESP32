![WIP](https://img.shields.io/badge/ESP_IDF-V5.3.2-blue)
![WIP](https://img.shields.io/badge/target-ESP32_S3-blue)
![WIP](https://img.shields.io/badge/CLIPS-V6.4.1-blue)
![WIP](https://img.shields.io/badge/status-WIP-orange)

# CLIPS-ArduinoNanoESP32
A porting of [CLIPS](https://sourceforge.net/projects/clipsrules/) to [Arduino Nano ESP32](https://docs.arduino.cc/hardware/nano-esp32/).

# About CLIPS

Developed at NASA’s Johnson Space Center from 1985 to 1996, the ‘C’ Language Integrated Production System ([CLIPS](https://ntrs.nasa.gov/api/citations/19960022632/downloads/19960022632.pdf)) is a rule-based programming language useful for creating expert systems and other programs where a heuristic solution is easier to implement and maintain than an algorithmic solution. Written in C for portability, CLIPS can be installed and used on a wide variety of platforms.

Since its first release in 1986, CLIPS has undergone
continual refinement and improvement and it is now used by thousands of people around the world.
Since 1996, CLIPS has been available as public domain software.

CLIPS is designed to facilitate the development of software to model human knowledge or
expertise.
There are three ways to represent knowledge in CLIPS:
- Rules, which are primarily intended for heuristic knowledge based on experience.
- Deffunctions and generic functions, which are primarily intended for procedural knowledge.
- Object-oriented programming ([COOL](https://ntrs.nasa.gov/api/citations/19920004651/downloads/19920004651.pdf)), also primarily intended for procedural knowledge. The five
generally accepted features of object-oriented programming are supported: classes,
message-handlers, abstraction, encapsulation, inheritance, and polymorphism. Rules may
pattern match on objects and facts.
You can develop software using only rules, only objects, or a mixture of objects and rules.

CLIPS programs may be executed in three ways: interactively using a simple Read-Eval-Print Loop (REPL) interface; interactively using an Integrated Development Environment (IDE) interface; or as embedded application in which the user provides a main program and controls execution of the expert system through the CLIPS Application Programming Interface (API).

# About this project

This project is a software porting, a mapping of the Read-Eval-Print Loop with the standard loop operation of Arduino and a COOL wrapper of the Arduino API (work in progress). Functions are provided to interact with other devices on the network, through the main standard network protocols (mainly MQTT). This enables endless application possibilities in the field of IoT and automation, as well as benefits such as code portability. COOL coding!

> This project is a prototype and is still under development. While it works, it has not been thoroughly tested and may not be stable. **Not recommended for production use.**

> All elements, parts, libraries, interfaces, code snippets, tools, and components used in this software are protected by their respective original copyrights; this also applies to the CLIPS software.

# Arduino COOL functions

These functions offer the ability to interact with the underlying functions of the Arduino, providing a way to control and guide the state of peripherals and initiate and control connections. Sometimes the status will be tracked via new fact types or instances.

- [pin-mode](https://docs.arduino.cc/language-reference/en/functions/digital-io/pinMode/)

    `(pin-mode D5 INPUT_PULLUP)`

    arg 1: < symbol > in range D1...D13, LED_RED, LED_GREEN, LED_BLUE; (at the moment only digital pin are supported).

    arg 2: < symbol > in OUTPUT, INPUT, INPUT_PULLDOWN, INPUT_PULLUP.

    An instance of the class PIN will be created:
    ```
    (defclass MAIN::PIN "A generic Arduino GPIO pin."
       (is-a USER)
       (role concrete)
       (pattern-match reactive)
       (slot value
          (access read-write)
          (type SYMBOL NUMBER))
       (slot mode
          (access read-write)
          (type SYMBOL)
          (default nil)
          (allowed-symbols nil INPUT OUTPUT)))
    ```

    This give you the ability to read/write the value through the implicit slot-accessor and its default message handler:
    ```
    (send [LED_RED] put-value LOW)    ; digital-write will be invoked
    (send [LED_RED] put-value HIGH)
    (send [D5] get-value)             ; digital-read will be invoked
    ```
    
- pin-reset

    `(pin-reset D5)`

    Reset the pin state and also the related instance will be deleted;

- [digital-read](https://docs.arduino.cc/language-reference/en/functions/digital-io/digitalread/)

    `(digital-read D5)`

- [digital-write](https://docs.arduino.cc/language-reference/en/functions/digital-io/digitalwrite/)

    `(digital-write D5 HIGH)`

- [wifi-status](https://docs.arduino.cc/libraries/wifi/#%60WiFi.status()%60)

    `(wifi-status)`

- [wifi-begin](https://docs.arduino.cc/libraries/wifi/#%60WiFi.begin()%60)

    option 1:

    `(wifi-begin <ssid-string> <password-string>)`

    option 2:

    ```
    (make-instance wifi of WIFI (ssid <ssid-string>)(pwd <password-string>))
    (wifi-begin [wifi])
    ```

- [wifi-scan](https://docs.arduino.cc/libraries/wifi/#%60WiFi.scanNetworks()%60)

    `(wifi-scan)`

- [wifi-disconnect](https://docs.arduino.cc/libraries/wifi/#%60WiFi.disconnect()%60)

    `(wifi-disconnect)`

- mqtt-connect

    ```
    (make-instance mqtt of MQTT 
        (broker <string>)  ; broker url
        (port <integer>)   ; broker port
        (usr <string>)
        (pwd <string>)
        (topic <string>)
        )

    (mqtt-connect [mqtt])
    (send [mqtt] get-clientid)
    ```
