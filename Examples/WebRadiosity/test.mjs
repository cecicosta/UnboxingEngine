import assert from 'node:assert/strict';
import test from 'node:test';
import createSolver from '../WebOctree/dist/radiosity/solver.mjs';

test('radiosity uses wall reflectance, rebuilds for a larger torus, and supports a dark light', async () => {
    const solver = await createSolver();
    const settings = {
        left: { r: 0.78, g: 0.16, b: 0.12 },
        back: { r: 0.68, g: 0.68, b: 0.66 },
        right: { r: 0.15, g: 0.68, b: 0.20 },
        torusScale: 1,
        light: 3,
    };

    const original = solver.solve(settings);
    const positions = solver.positions().slice();
    const colors = solver.colors().slice();
    assert.equal(original.patches, 626);
    assert.ok(original.iterations > 1 && original.formFactors > 0);
    assert.equal(original.rebuiltGeometry, true);
    assert.equal(positions.length, original.patches * 9);
    assert.equal(colors.length, positions.length);
    assert.ok(colors.every(color => Number.isFinite(color) && color >= 0 && color <= 1));
    assert.ok(colors.some(color => color > 0.1));

    settings.left = { r: 0.06, g: 0.16, b: 0.9 };
    const recolored = solver.solve(settings);
    assert.equal(recolored.rebuiltGeometry, false);
    assert.equal(recolored.formFactors, original.formFactors);
    assert.deepEqual(solver.positions(), positions);
    assert.ok(solver.colors().slice(0, 128 * 9).some((color, i) => Math.abs(color - colors[i]) > 0.001),
        'the floor receives different bounced light');

    settings.torusScale = 1.6;
    const resized = solver.solve(settings);
    assert.equal(resized.rebuiltGeometry, true);
    assert.equal(resized.patches, original.patches);
    assert.notDeepEqual(solver.positions(), positions);
    assert.notEqual(resized.formFactors, original.formFactors);

    settings.light = 0;
    assert.equal(solver.solve(settings).rebuiltGeometry, false);
    assert.ok(solver.colors().every(color => color === 0));
});
