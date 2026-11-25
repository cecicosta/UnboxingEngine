import assert from 'node:assert/strict';
import test from 'node:test';
import createSolver from '../WebOctree/dist/voxelization/solver.mjs';

test('voxelization exposes the native sweep, cell origins, and selected column', async () => {
    const solver = await createSolver();
    assert.equal(solver.column(0, 0).length, 0);
    const torus = solver.voxelize(0, 28);
    assert.equal(torus.rays, 28 * 28);
    assert.ok(Math.abs(torus.cellSize - 8 / 28) < 0.000001);
    assert.ok(torus.intersections > 0 && torus.cells > 0);

    const cells = solver.cells().slice();
    const vertices = solver.vertices().slice();
    assert.ok(cells instanceof Int32Array);
    assert.equal(cells.length, 3 * torus.cells);
    assert.equal(vertices.length, 3 * 32 * 12);
    assert.ok(solver.indices().every(index => index < vertices.length / 3));
    assert.ok(torus.minZ < 0 && torus.maxZ >= 0);
    const occupied = new Set();
    for (let i = 0; i < cells.length; i += 3) {
        const [x, y, z] = cells.subarray(i, i + 3);
        assert.ok(x >= -14 && x < 14 && y >= -14 && y < 14);
        assert.ok(z >= torus.minZ && z <= torus.maxZ);
        occupied.add([x, y, z].join(','));
    }
    assert.equal(occupied.size, torus.cells);

    assert.equal(solver.column(0, 0).length, 0, 'the torus hole is empty');
    assert.equal(solver.column(14, 0).length, 0, 'the grid upper edge is excluded');
    const tube = solver.column(9, 1).slice();
    assert.equal(tube.length, 6);
    assert.ok(Math.abs(tube[0] - 9 * torus.cellSize) < 0.00001);
    assert.ok(Math.abs(tube[1] - torus.cellSize) < 0.00001 && tube[2] < 0 && tube[5] > 0);

    const cube = solver.voxelize(2, 44);
    assert.equal(cube.rays, 44 * 44);
    assert.ok(cube.cells > torus.cells);
    assert.equal(solver.indices().length, 36);
    const center = solver.column(0, 0).slice();
    assert.equal(center.length, 6);
    assert.ok(Math.abs(center[2] + 2.75) < 0.0001 && Math.abs(center[5] - 2.75) < 0.0001);

    const sphere = solver.voxelize(1, 1);
    assert.equal(sphere.rays, 12 * 12, 'the lower resolution is clamped');
    assert.ok(sphere.cells > 0);
    assert.equal(solver.column(2, 1).length, 6);
});
