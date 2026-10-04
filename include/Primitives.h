#pragma once
#include "Mesh.h"

namespace Primitives {

/**
 * 1. CUBE GENERATOR (8 Vertices -> 24 Vertices with Flat Normals -> 12 Triangles per Box)
 * Used for:
 *   - Stepped Pyramid Slabs (Layer 0, 1, 2 Trays)
 *   - Square Chiclet Candies
 */
Mesh createCube();

/**
 * 2. CYLINDER / PRISM GENERATOR (Pure Triangles: Side Quads -> 2 Tris + Fan Caps)
 * Used for:
 *   - Flat Hexagonal (sectors=6) / Octagonal (sectors=8) Lozenge Candies
 *   - Cylindrical Gumdrop Candies
 * 
 * @param sectors Number of angular segments around the circumference
 * @param radius  Radius of the cylinder
 * @param height  Total vertical height of the cylinder
 */
Mesh createCylinder(int sectors = 8, float radius = 0.4f, float height = 0.3f);

/**
 * 3. UV SPHERE GENERATOR (Latitude / Longitude Triangulation)
 * Used for:
 *   - Spherical Lemon Drop / Ball Candies
 * 
 * @param rings   Number of horizontal latitude divisions (0 to PI)
 * @param sectors Number of vertical longitude divisions (0 to 2*PI)
 * @param radius  Radius of the sphere
 */
Mesh createSphere(int rings = 16, int sectors = 20, float radius = 0.4f);

/**
 * 4. DIAMOND / BIPYRAMID GENERATOR (Double-cone Triangulation)
 * Used for:
 *   - Faceted Gem / Diamond Candies
 * 
 * @param facets Number of equatorial facets
 * @param radius Radius of the equatorial ring
 * @param height Total tip-to-tip vertical height
 */
Mesh createDiamond(int facets = 8, float radius = 0.4f, float height = 0.6f);

} // namespace Primitives
