# PinIO Input Blackbox Trigger

Start and stop Blackbox logging from an external digital signal, for example a GoPro's record state relayed by an ESP32-S3, so each camera recording gets a matching log file.

When `pinio_input_blackbox` is set to a PinIO configured as an input:

* **Input HIGH**: opens a **new log file** and records continuously, armed or disarmed.
* **Input LOW**: writes the `End of log` marker, flushes and **closes the log**.
* **Next HIGH**: opens the **next new log file**.

The input must hold a new level for **50 ms** before it acts, so contact bounce and boot glitches do not create stray logs.

---

## Quick start (CLI)

```cli
# Free a pad and assign it to PinIO 1 (example: motor 5 pad on A01)
resource MOTOR 5 NONE
resource PINIO 1 A01

# PinIO 1 as input with pull-down, so a disconnected or booting ESP32 reads LOW
set pinio_config = 4,1,1,1

# Use PinIO 1 as the Blackbox trigger
set pinio_input_blackbox = 1

save
```

### Wiring (ESP32-S3)

* ESP32 GPIO -> PinIO pad. Both sides are 3.3 V logic; never feed 5 V into the FC pad.
* ESP32 GND -> FC GND (required).
* Avoid ESP32-S3 strapping pins (GPIO0, GPIO3, GPIO45, GPIO46); they can toggle during boot.

---

## Settings

### `pinio_config` (per PinIO channel)

| Value | Mode | Use |
| :---: | :--- | :--- |
| `1` | Output push-pull (default) | VTX / accessory power switch |
| `2` | Input, floating | Actively driven 3.3 V logic signal |
| `3` | Input, pull-up (idle HIGH) | Switch to GND |
| `4` | Input, pull-down (idle LOW) | Switch to 3.3 V, or an MCU output such as an ESP32 |

Add `128` to invert the level (e.g. `131` = pull-up input that triggers when pulled to GND).

### `pinio_input_blackbox`

`0` = off (stock behaviour), `1`..`4` = PinIO channel used as the trigger.

The trigger is only used when that PinIO has a pin assigned **and** is configured as an input (modes 2-4). Otherwise Blackbox falls back to normal arm-based logging, so a misconfiguration never silently stops all logging.

---

## Behaviour

| Situation | Result |
| :--- | :--- |
| Input HIGH, armed or disarmed | Logging (new file on each LOW -> HIGH) |
| Input LOW, armed | Not logging; the input has priority over arming |
| Disarm while input HIGH | Logging continues until the input goes LOW |
| Input LOW while the log header is still being written, or while paused | Log is closed cleanly |
| `blackbox_mode = ALWAYS` | The input decides; ALWAYS no longer auto-starts |
| `blackbox_mode = MOTOR_TEST` | Trigger ignored; stock motor-test logging |
| `BLACKBOX` AUX mode configured | Still pauses / resumes inside a log opened by the input |
| Serial logger sharing a port with MSP | Logging only while armed, so the configurator keeps its port |
| `BLACKBOX ERASE` AUX mode active while stopped | Erase runs; a log is not opened first |
| PinIO configured as input | `pinio_box` cannot drive it; outputs are never written to input pins |
