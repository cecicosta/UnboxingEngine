import { readFile, writeFile } from 'node:fs/promises';
import { extractShader } from './extract-shader.mjs';

const samples = await readFile(new URL('../FieldsSimulationShaders.h', import.meta.url), 'utf8');
const library = await readFile(new URL('../../include/ShaderLibrary.h', import.meta.url), 'utf8');
const shaders = {
    scanVertex: extractShader(library, 'transform_vertex'),
    scanFragment: extractShader(samples, 'volumetric_scanning_frag'),
    quadVertex: extractShader(library, 'quad_render_vertex'),
    // GLSL ES does not implicitly promote integer literals in float expressions.
    potentialFragment: extractShader(samples, 'field_tex_render_frag')
        .replace('(thickness*thickness)/4;', '(thickness*thickness)/4.0;')
        .replace('float Z = 1;', 'float Z = 1.0;')
        .replace('(4*PI*E0)', '(4.0*PI*E0)')
        .replaceAll('2 * k * 1/', '2.0 * k * 1.0 / '),
    combineFragment: extractShader(samples, 'combine_fields_tex_frag'),
    flowFragment: extractShader(library, 'vizualize_flow_fragment_shader')
        .replace('finalSample < 0)', 'finalSample < 0.0)'),
    presentFragment: extractShader(library, 'signed_texture_debug_fragment_shader_source')
        .replace('vec4 texture =', 'vec4 sampled =')
        .replace(/\btexture\./g, 'sampled.'),
};

await writeFile(process.argv[2], 'export default ' + JSON.stringify(shaders, null, 2) + ';\n');
