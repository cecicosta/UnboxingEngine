export function extractShader(header, symbol) {
    if (!/^[A-Za-z_]\w*$/.test(symbol)) {
        throw new Error('Invalid shader identifier: ' + symbol);
    }
    const assignment = new RegExp('\\b' + symbol + '\\s*=\\s*R"\\(([\\s\\S]*?)\\)"\\s*;');
    const match = assignment.exec(header);
    if (!match) {
        throw new Error('Native shader not found: ' + symbol);
    }
    const version = /^\s*#version[ \t]+150(?:[ \t]+core)?[ \t]*\r?\n/;
    if (!version.test(match[1])) {
        throw new Error('Expected GLSL 150 for shader: ' + symbol);
    }
    const precision = 'precision highp float;\nprecision highp int;\nprecision highp sampler2D;\n';
    return precision + match[1].replace(version, '');
}
