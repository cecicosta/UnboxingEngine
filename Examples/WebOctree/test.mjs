import assert from 'node:assert/strict';
import test from 'node:test';
import createOctree from './dist/octree.mjs';

test('the WebAssembly lab returns a torus hit and its visited boxes', async () => {
    const lab = await createOctree();
    const vertexCount = lab.vertices().length / 3;
    const indices = lab.indices().slice();
    const bounds = lab.nodeBounds().slice();
    assert.equal(indices.length / 3, 1120);
    assert.ok(indices.every(index => index < vertexCount));
    assert.ok(bounds.length > 6);
    assert.equal(bounds.length, 6 * lab.nodeStates().length);

    const hit = lab.shoot(4.17, 0.27, 10, 0, 0, -1);
    assert.ok(hit.triangle >= 0 && hit.triangle < indices.length / 3);
    assert.ok(hit.candidates > 0 && hit.candidates < indices.length / 3);
    assert.ok(hit.visited > 0 && hit.visited < bounds.length / 6);
    assert.ok(Math.abs(hit.x - 4.17) < 0.001 && Math.abs(hit.y - 0.27) < 0.001);
    assert.ok(hit.z > 0 && hit.z <= 1.15);
    const states = lab.nodeStates().slice();
    assert.equal(states.filter(state => state !== 0).length, hit.visited);
    assert.equal(states.filter(state => state === 3).length, 1);

    const hole = lab.shoot(0, 0, 10, 0, 0, -1);
    assert.equal(hole.triangle, -1);
    assert.ok(hole.visited > 0);
    assert.ok(lab.nodeStates().every(state => state !== 3));

    const miss = lab.shoot(20, 20, 10, 0, 0, -1);
    assert.equal(miss.triangle, -1);
    assert.equal(miss.visited, 0);
    assert.equal(miss.candidates, 0);
    assert.ok(lab.nodeStates().every(state => state === 0));
});
