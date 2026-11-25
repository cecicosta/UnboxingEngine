import { WebGLRenderer } from 'three';
import { FieldsPipeline } from './pipeline.mjs?v=2';

const byId = id => document.getElementById(id);
const controls = ['resolution', 'flow-reference', 'run', 'step', 'reset'];
const canvas = byId('scene');
const viewport = byId('viewport');
const loader = byId('loader');
const status = byId('status');

function report(text, kind = '') {
    status.textContent = text;
    status.dataset.kind = kind;
}

function start() {
    const renderer = new WebGLRenderer({ canvas, alpha: false, antialias: false });
    renderer.setPixelRatio(Math.min(devicePixelRatio, 2));
    renderer.debug.onShaderError = (gl, program, vertex, fragment) => {
        throw new Error([gl.getProgramInfoLog(program), gl.getShaderInfoLog(vertex),
            gl.getShaderInfoLog(fragment)].filter(Boolean).join('\n') || 'Shader compilation failed.');
    };
    const pipeline = new FieldsPipeline(renderer);
    const resolution = byId('resolution');
    const reference = byId('flow-reference');
    const run = byId('run');
    let running = false;
    let animation = 0;
    let last = 0;
    let failed = false;
    let width = 0;
    let height = 0;

    function reportRunning() {
        run.textContent = running ? 'Pause' : 'Run';
        byId('step').disabled = running;
        report(running ? 'Running' : 'Paused', running ? 'running' : '');
    }

    function fail(error) {
        cancelAnimationFrame(animation);
        running = false;
        failed = true;
        for (const id of controls) byId(id).disabled = true;
        showError(error);
    }

    function draw() {
        const stage = viewport.getBoundingClientRect();
        const region = id => {
            const rect = byId(id).getBoundingClientRect();
            const size = Math.floor(Math.min(rect.width, rect.height));
            return [
                Math.round(rect.left - stage.left + (rect.width - size) / 2),
                Math.round(stage.bottom - rect.bottom + (rect.height - size) / 2),
                size, size,
            ];
        };
        pipeline.present(region('potential'), region('flow'));
        byId('iterations').textContent = pipeline.steps.toLocaleString();
        byId('target-size').textContent = pipeline.resolution + '\u00d7' + pipeline.resolution;
    }

    function resize() {
        const { clientWidth, clientHeight } = viewport;
        if (failed || !clientWidth || !clientHeight) return;
        if (clientWidth !== width || clientHeight !== height) {
            width = clientWidth;
            height = clientHeight;
            renderer.setSize(width, height, false);
        }
        draw();
    }

    const update = action => () => {
        if (failed) return;
        try {
            action();
            draw();
        } catch (error) {
            fail(error);
        }
    };

    function tick(now) {
        if (!running || failed) return;
        try {
            if (now - last >= 1000 / 30) {
                pipeline.advance(2);
                last = now;
                draw();
            }
            animation = requestAnimationFrame(tick);
        } catch (error) {
            fail(error);
        }
    }

    function syncSettings() {
        pipeline.setFlowReference(reference.value);
        const size = Number(resolution.value);
        if (size !== pipeline.resolution) pipeline.setResolution(size);
    }

    for (const option of [...resolution.options]) {
        if (Number(option.value) > pipeline.maxResolution) option.remove();
    }
    if (!resolution.options.length) throw new Error('No supported render-target sizes.');
    if (!resolution.value) resolution.selectedIndex = resolution.options.length - 1;
    syncSettings();
    resize();
    new ResizeObserver(resize).observe(viewport);

    const applySettings = update(syncSettings);
    resolution.addEventListener('change', applySettings);
    reference.addEventListener('change', applySettings);
    window.addEventListener('pageshow', applySettings);
    byId('step').addEventListener('click', update(() => pipeline.advance()));
    byId('reset').addEventListener('click', update(() => {
        running = false;
        cancelAnimationFrame(animation);
        pipeline.reset();
        reportRunning();
    }));
    run.addEventListener('click', () => {
        running = !running;
        cancelAnimationFrame(animation);
        if (running) {
            last = 0;
            animation = requestAnimationFrame(tick);
        }
        reportRunning();
    });
    canvas.addEventListener('webglcontextlost', () => fail(new Error('The GPU context was lost. Reload to restart.')));
    for (const id of controls) byId(id).disabled = false;
    reportRunning();
    loader.hidden = true;
}

function showError(error) {
    console.error(error);
    loader.hidden = false;
    loader.textContent = error.message;
    report('Unavailable', 'error');
}

try {
    start();
} catch (error) {
    showError(error);
}
