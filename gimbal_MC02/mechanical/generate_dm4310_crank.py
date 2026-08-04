"""Generate the DM4310 end-drive crank as a watertight STL.

The generated part retains the mounting pattern measured from the supplied
working STL, but replaces the low-eccentricity integral printed rod with a
reinforced crank arm and an M8 steel follower-pin hole.
"""

from pathlib import Path

import numpy as np
import trimesh
from shapely.geometry import Point
from shapely.ops import unary_union


HUB_RADIUS_MM = 17.5
PART_THICKNESS_MM = 10.0

MOUNT_PCD_MM = 27.0
MOUNT_HOLE_DIAMETER_MM = 3.3

CRANK_RADIUS_MM = 50.0
PIN_BOSS_RADIUS_MM = 10.5
PIN_HOLE_DIAMETER_MM = 8.2


def circle(center: tuple[float, float], radius: float, resolution: int = 96):
    return Point(center).buffer(radius, resolution=resolution)


def build_profile():
    hub = circle((0.0, 0.0), HUB_RADIUS_MM)
    pin_boss = circle((CRANK_RADIUS_MM, 0.0), PIN_BOSS_RADIUS_MM)

    # The convex hull makes a broad, continuously filleted arm between the
    # output hub and follower boss, avoiding a narrow printed stress riser.
    outer = unary_union((hub, pin_boss)).convex_hull

    mount_radius = MOUNT_PCD_MM / 2.0
    mount_holes = [
        circle(
            (
                mount_radius * np.cos(np.deg2rad(angle_deg)),
                mount_radius * np.sin(np.deg2rad(angle_deg)),
            ),
            MOUNT_HOLE_DIAMETER_MM / 2.0,
            resolution=48,
        )
        for angle_deg in (60.0, 120.0, 240.0, 300.0)
    ]
    follower_hole = circle(
        (CRANK_RADIUS_MM, 0.0),
        PIN_HOLE_DIAMETER_MM / 2.0,
        resolution=64,
    )

    return outer.difference(unary_union((*mount_holes, follower_hole)))


def main() -> None:
    output_path = Path(__file__).with_name("DM4310_CRANK_R50_M8_V1.stl")
    profile = build_profile()
    mesh = trimesh.creation.extrude_polygon(
        profile,
        height=PART_THICKNESS_MM,
        engine="earcut",
    )
    mesh.remove_unreferenced_vertices()
    mesh.fix_normals()

    if not mesh.is_watertight:
        raise RuntimeError("Generated crank mesh is not watertight")
    if not mesh.is_volume:
        raise RuntimeError("Generated crank mesh is not a valid solid volume")

    mesh.export(output_path, file_type="stl")

    bounds = mesh.bounds
    extents = bounds[1] - bounds[0]
    print(f"wrote={output_path}")
    print(f"watertight={mesh.is_watertight} volume={mesh.is_volume}")
    print(f"faces={len(mesh.faces)} vertices={len(mesh.vertices)}")
    print(f"bounds_min_mm={bounds[0]}")
    print(f"bounds_max_mm={bounds[1]}")
    print(f"extents_mm={extents}")
    print(f"volume_mm3={mesh.volume:.3f}")


if __name__ == "__main__":
    main()
