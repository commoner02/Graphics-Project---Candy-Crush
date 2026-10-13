#pragma once
#include "Mesh.h"

namespace Primitives {

/**
 * 1. CUBE GENERATOR (8 Vertices -> 24 Vertices with Flat Normals -> 12 Triangles per Box)
 * Used for:
 *   - Stepped Pyramid Slabs (Layer 0, 1, 2 Trays)
 *   - Pyramid Foundation Pedestals
 */
Mesh createCube();

/**
 * 2. CYLINDER / PRISM GENERATOR (Pure Triangles: Side Quads -> 2 Tris + Fan Caps)
 * Used for:
 *   - Cylindrical Gumdrops / Architectural Columns
 * 
 * @param sectors Number of angular segments around the circumference
 * @param radius  Radius of the cylinder
 * @param height  Total vertical height of the cylinder
 */
Mesh createCylinder(int sectors = 8, float radius = 0.4f, float height = 0.3f);

/**
 * 3. UV SPHERE GENERATOR (Latitude / Longitude Triangulation)
 * Used for:
 *   - Spherical Candies / Ball Confections
 * 
 * @param rings   Number of horizontal latitude divisions (0 to PI)
 * @param sectors Number of vertical longitude divisions (0 to 2*PI)
 * @param radius  Radius of the sphere
 */
Mesh createSphere(int rings = 16, int sectors = 20, float radius = 0.4f);

/**
 * 4. DIAMOND / BIPYRAMID GENERATOR (Double-cone Triangulation)
 * Used for:
 *   - Diamond Primitives
 * 
 * @param facets Number of equatorial facets
 * @param radius Radius of the equatorial ring
 * @param height Total tip-to-tip vertical height
 */
Mesh createDiamond(int facets = 8, float radius = 0.4f, float height = 0.6f);

// Confectionery Suite (Iconic Solid Candy Crush Sweets)
Mesh createBentCylinder(int spineSegments = 28, int tubeSegments = 20, int capRings = 6, float halfLength = 0.28f, float radius = 0.125f, float bendDepth = 0.12f);
Mesh createTeardrop(int rings = 28, int sectors = 24, float radius = 0.35f, float height = 0.54f);
Mesh createFlyingSaucer(int rings = 24, int sectors = 24, float radius = 0.32f, float flange = 0.14f);
Mesh createPillow(int rings = 20, int sectors = 28, float halfWidth = 0.35f, float halfHeight = 0.18f);

} // namespace Primitives

