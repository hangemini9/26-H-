# DM4310 end-drive crank R50 V1

`DM4310_CRANK_R50_M8_V1.stl` is the recommended replacement for the supplied
low-eccentricity integral-rod parts.

## Frozen dimensions

- output hub: 35 mm diameter;
- part thickness: 10 mm;
- four motor mounting holes: 3.3 mm diameter on a 27 mm pitch circle;
- follower-pin eccentricity: 50 mm;
- follower-pin hole: 8.2 mm diameter;
- follower boss: 21 mm diameter;
- nominal hinge-to-drive distance: 220-230 mm;
- intended usable pipe tilt: approximately +/-10 degrees;
- initial motor travel for the mechanism: approximately +/-55 degrees from
  the installed neutral, subject to ball-free hardware verification.

The four mounting-hole centers are at radius 13.5 mm and angles 60, 120, 240,
and 300 degrees. This is equivalent to approximately
`(x,y)=(+/-6.75,+/-11.69) mm`, measured from the motor-output center.

## Required follower hardware

Do not print a long cantilever rod as part of the crank. Use:

- one M8 steel shoulder bolt or smooth 8 mm steel shaft;
- preferably two 688 bearings (8 x 16 x 5 mm) as the free follower roller;
- spacers and a locknut so the roller contact plane is about 18-20 mm in
  front of the printed crank;
- an upper keeper with 1-2 mm free clearance above the 20 mm OD pipe so the
  pipe cannot leave the follower during vehicle vibration.

If the hinge axis passes through the pipe centerline, a 16 mm OD follower
requires the motor axis to be about 18 mm below the hinge axis when the pipe
is horizontal and the crank points horizontally. Recalculate this height if
the final follower outside diameter or hinge offset changes.

## Printing

- print flat with the 10 mm thickness in Z, as stored in the STL;
- PETG, PA, or another tough material is preferred over brittle PLA;
- use at least 6 walls and 60% infill; use 100% infill around the follower
  boss if the slicer supports modifier regions;
- inspect all four motor holes and the M8 follower hole before installation;
- do not perform the first powered test with a ball installed.

The generator is `generate_dm4310_crank.py`. It requires `trimesh`, `shapely`,
and `mapbox_earcut`.
