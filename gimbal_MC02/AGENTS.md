# Agent instructions

Before changing this project, read `AGENT.MD` completely. Treat it as the
canonical hardware/software handoff. After every material code, wiring,
parameter, protocol, build, or hardware-test change, update `AGENT.MD` in the
same session.

Do not claim hardware behavior from a successful compile. Keep unverified
assumptions explicit. This project uses VSCode/CMake/GCC and J-Link/Ozone;
do not use or repair the copied MDK project.

The default firmware must remain safe for desktop commissioning:
no automatic motor enable, explicit arm before motion, bounded duration,
bounded angle/speed, feedback watchdog, and unconditional STOP.
