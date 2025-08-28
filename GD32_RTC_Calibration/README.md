# Library: RTCCalibration

- [Library: RTCCalibration](#library-rtccalibration)
  - [Summary](#summary)

---

## Summary

The RTCCalibration library provides a structured mechanism for calibrating the Real-Time Clock (RTC) on GD32 microcontrollers, specifically variants such as GD32F50x, GD32F30x, and GD32F10x. It utilizes a Pulse Per Second (PPS) signal, typically sourced from a GPS module, to measure the actual RTC frequency deviation from the nominal 32,768 Hz and apply corrections. The calibration process involves two primary phases: frequency trimming (acquiring 120 PPS pulses to estimate the base frequency) and fine calibration (acquiring 1,280 PPS pulses to compute a correction value within hardware limits, e.g., -128 to +127 for most variants, or 0 to +127 for GD32F10x).
Key features include a state machine for managing calibration stages, interrupt-driven PPS handling with optional user callbacks, timeout detection to prevent stalls, and functions for applying prescaler and calibration values directly to hardware registers. Progress monitoring is available as a percentage, and validity checks ensure corrections are within bounds. The library is encapsulated in a namespace to avoid naming conflicts and uses conditional compilation for hardware-specific behaviors.
The provided example sketch ([GD32_RTC_Calibration.ino](./GD32_RTC_Calibration.ino)) illustrates practical usage: initializing the library with a PPS pin, attaching a callback for visual feedback (e.g., LED toggle), and periodically invoking the calibration function while logging status via serial output. This setup enables developers to integrate high-accuracy RTC calibration into Arduino-based projects with minimal overhead, enhancing timekeeping precision in applications such as data logging or synchronization systems.

---
