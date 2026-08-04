# Final printed gimbal crank

User-frozen on 2026-07-30:

```text
PITCH_FINAL_R15P45_R3_20260730.STL
```

Original source:

```text
C:\Users\Han\Desktop\dIANSAI\PITCH_NEW_RPLUS7_R3_CONTAINED.STL
```

Integrity:

```text
size:   768984 bytes
SHA256: 010919C1A376811A5C7BEDD237305554FC9CF96A3D4154B04B5BBF57FD02E3F5
```

The source and project backup were byte/hash verified identical.

## Measured STL geometry

- binary STL, 15,378 triangles;
- one connected watertight component;
- overall bounds approximately 49 x 75 x 49 mm;
- circular mounting base diameter: 49 mm;
- four mounting holes: diameter 3.2 mm on a 27 mm pitch circle;
- integral follower rod: diameter 12 mm;
- follower-axis eccentricity from motor axis: **15.45 mm**;
- R3 contained transition is present;
- mesh volume: approximately 17,263 mm^3.

This is the final physical print selected by the user. Earlier 25 mm and
50 mm crank proposals and `mechanical/DM4310_CRANK_R50_M8_V1.stl` are
superseded and must not be used for firmware geometry.

## Firmware constants still pending installation

Use the fixed crank radius:

```text
crank_radius_mm = 15.45
```

Do not finalize the motor-to-pipe mapping until these installed values are
measured:

- hinge-axis to follower contact distance `L`;
- motor feedback angle at a level pipe;
- installed motion sign;
- usable motor-angle endpoints before contact loss or collision.

Installed measurements reported on 2026-07-30:

- DM4310 axis is 1 mm above the hinge axis;
- pipe midpoint to follower station is 115 mm;
- pipe midpoint to hinge axis is 133 mm;
- direct hinge-to-follower distance was separately reported as 236 mm.

The two midpoint dimensions imply 248 mm, which conflicts with 236 mm by
exactly the nominal 12 mm follower diameter. Do not use either as the final
`L` until the hinge-axis to DM4310-output-axis center distance is directly
remeasured. The physical pipe endpoints are +/-125 mm from O and the scored
positions are +/-50 mm.

The geometric absolute pipe-angle limit is
`asin(15.45 / L)`. Representative values are approximately:

| L | Absolute geometric limit |
|---:|---:|
| 180 mm | 4.92 deg |
| 220 mm | 4.03 deg |
| 230 mm | 3.85 deg |

No firmware behavior is verified merely by freezing this STL.
