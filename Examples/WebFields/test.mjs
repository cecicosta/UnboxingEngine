import assert from 'node:assert/strict';
import test from 'node:test';
import { extractShader } from './extract-shader.mjs';
import shaders from '../WebOctree/dist/fields/shaders.mjs';

test('the extractor matches the exact native GLSL symbol and preserves its body', () => {
    const native = 'char *xLong = R"(#version 150\nignore\n)";\n'
        + 'char *x  = R"(\r\n #version 150 core\r\nvoid main() {\n    // body\n}\n)";';
    assert.match(extractShader(native, 'x'), /^precision highp float;/);
    assert.ok(extractShader(native, 'x').endsWith('void main() {\n    // body\n}\n'));
    assert.throws(() => extractShader(native, 'noSuchShader'), /not found/);
    assert.throws(() => extractShader(native, 'x.*'), /Invalid/);
    assert.throws(() => extractShader('auto x = R"(#version 1500\ncode\n)";', 'x'), /GLSL 150/);
});

test('the generated WebGL shaders retain the native scan and feedback', () => {
    assert.equal(Object.keys(shaders).length, 7);
    for (const shader of Object.values(shaders)) {
        assert.match(shader, /^precision highp float;/);
        assert.doesNotMatch(shader, /^#version/m);
    }
    assert.match(shaders.scanFragment, /gl_FrontFacing/);
    assert.match(shaders.potentialFragment, /float V = 2\.0 \* k \* 1\.0 \/ r_min_pm;/);
    assert.match(shaders.combineFragment, /center \+ diffusionStep \* \(laplacian - nLaplacian\)/);
    assert.match(shaders.presentFragment, /vec4 sampled = texture\(u_texture/);
});
