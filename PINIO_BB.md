# PinIO Input & Blackbox Master Trigger Guide

This document describes how to configure and use **GPIO PinIO Input** reading and the **PinIO Blackbox Master Override Trigger** (`pinio_input_blackbox`) in Betaflight.

---

## Overview

When `pinio_input_blackbox` is enabled, Blackbox logging is controlled strictly by an external physical GPIO input signal fed into a Flight Controller pad:

* **Signal HIGH (3.3V)**: Immediately opens a **brand-new log file** (e.g. `LOG00001.TXT`) and records flight data continuously (even if disarmed on the ground).
* **Signal LOW (0V)**: Immediately flushes data buffers, **closes the active log file**, and stops logging.
* **Next Signal HIGH**: Opens the **next new log file** (e.g. `LOG00002.TXT`).
* **`pinio_input_blackbox = 0` (OFF)**: PinIO input trigger is disabled, restoring 100% standard Betaflight logging logic.

---

## Quick-Start CLI Setup Guide

Copy and paste these commands into the Betaflight Configurator **CLI** tab:

```cli
# Step 1: Free your chosen hardware pad (e.g., an unused RX3 pad on pin C08)
resource RX 3 NONE

# Step 2: Assign pad C08 to PinIO 1
resource PINIO 1 C08

# Step 3: Configure PinIO 1 as an Input with Pull-Down resistor (4 = Input Pull-Down)
set pinio_config = 4,1,1,1

# Step 4: Enable PinIO 1 as the Blackbox Master Trigger (1 = PinIO 1)
set pinio_input_blackbox = 1

# Step 5: Save and reboot Flight Controller
save
```

---

## Configuration Reference

### 1. PinIO Input Modes (`pinio_config`)

`pinio_config` configures the input/output mode for each of the 4 PinIO channels (`config[0]` to `config[3]`):

| Value | Mode | Electrical Behavior | Common Application |
| :---: | :--- | :--- | :--- |
| `1` | **Output Push-Pull** *(Default)* | Pin driven HIGH (3.3V) or LOW (0V) as an output | VTX power switch, Bluetooth power toggle |
| `2` | **Input Floating** | High-impedance input without internal pull resistor | External active 0V / 3.3V microcontroller signal |
| `3` | **Input Pull-Up** | Internal pull-up to 3.3V (Active LOW) | Mechanical switch / button connected to Ground (GND) |
| `4` | **Input Pull-Down** | Internal pull-down to GND (Active HIGH) | Mechanical switch / button connected to 3.3V |

> [!NOTE]
> Add `128` to any mode value to invert the logic state (e.g., `4 + 128 = 132` for inverted Input Pull-Down).

---

### 2. Interaction with `blackbox_mode` Settings

`pinio_input_blackbox` interacts with Betaflight's built-in `blackbox_mode` settings as follows:

| `blackbox_mode` | PinIO Override Behavior |
| :--- | :--- |
| **`NORMAL`** | **PinIO Overrides Arming**: PinIO HIGH starts a new log file. PinIO LOW closes the log file, regardless of whether the quad is Armed or Disarmed. |
| **`ALWAYS_ON`** | **PinIO Overrides Always-On**: PinIO HIGH starts a new log file. PinIO LOW closes the log file and prevents `ALWAYS_ON` from auto-starting while LOW. |
| **`MOTOR_TEST`** | **PinIO Bypassed**: PinIO input trigger is automatically **disabled** during motor testing so bench testing in Configurator works normally. |

---

## Behavior Matrix (`pinio_input_blackbox = 1`)

| Armed Status | `blackbox_mode` | AUX Switch | PinIO Input | Blackbox Logging | Action Taken |
| :---: | :---: | :---: | :---: | :---: | :--- |
| **ARMED** | NORMAL | Active | **LOW** | ❌ **OFF** | **Inhibited** (PinIO LOW blocks logging despite being Armed) |
| **ARMED** | NORMAL | Active | **HIGH** | ✅ **ON** | **Logging Active** (Opens new log file) |
| **DISARMED** | NORMAL | Inactive | **HIGH** | ✅ **ON** | **Logging Active** (PinIO HIGH triggers log on ground) |
| **DISARMED** | ALWAYS | Inactive | **LOW** | ❌ **OFF** | **Inhibited** (PinIO LOW blocks ALWAYS_ON mode) |
| Any | NORMAL / ALWAYS | Any | **LOW $\rightarrow$ HIGH** | ✅ **NEW LOG** | Opens `LOG00001.TXT` |
| Any | NORMAL / ALWAYS | Any | **HIGH $\rightarrow$ LOW** | ❌ **CLOSE LOG** | Flushes & closes `LOG00001.TXT` |
| Any | NORMAL / ALWAYS | Any | **LOW $\rightarrow$ HIGH** | ✅ **NEW LOG** | Opens `LOG00002.TXT` |
