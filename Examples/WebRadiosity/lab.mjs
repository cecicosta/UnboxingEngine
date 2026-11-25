import * as THREE from 'three';
import { OrbitControls } from '../vendor/OrbitControls.js';

const byId = id => document.getElementById(id);
const state = byId('state');
const loader = byId('loader');

function setState(text, kind = '') {
    state.textContent = text;
    state.dataset.kind = kind;
}

function start() {
    const canvas = byId('scene');
    const viewport = byId('viewport');
    const renderer = new THREE.WebGLRenderer({ canvas, antialias: true });
    renderer.setPixelRatio(Math.min(devicePixelRatio, 2));

    const scene = new THREE.Scene();
    scene.background = new THREE.Color(0x10171a);
    const camera = new THREE.OrthographicCamera(-4, 4, 4, -4, 0.1, 80);
    camera.up.set(0, 0, 1);
    camera.position.set(5.4, -10, 6.3);
    const controls = new OrbitControls(camera, canvas);
    controls.target.set(0, 0, 1.4);
    controls.enablePan = false;
    controls.minZoom = 0.65;
    controls.maxZoom = 2.5;
    controls.minPolarAngle = 0.1;
    controls.maxPolarAngle = Math.PI - 0.1;
    controls.update();
    controls.saveState();

    const grid = new THREE.GridHelper(16, 16, 0x344449, 0x273438);
    grid.rotation.x = Math.PI / 2;
    grid.position.z = -0.03;
    scene.add(grid);
    const material = new THREE.MeshBasicMaterial({ vertexColors: true, side: THREE.FrontSide });
    const box = new THREE.Mesh(new THREE.BufferGeometry(), material);
    box.visible = false;
    scene.add(box);

    const draw = () => renderer.render(scene, camera);
    controls.addEventListener('change', draw);
    const resize = () => {
        const { clientWidth: width, clientHeight: height } = viewport;
        if (!width || !height) return;
        const aspect = width / height;
        const halfHeight = Math.max(3.1, 3.65 / aspect);
        camera.left = -halfHeight * aspect;
        camera.right = halfHeight * aspect;
        camera.top = halfHeight;
        camera.bottom = -halfHeight;
        camera.updateProjectionMatrix();
        renderer.setSize(width, height, false);
        draw();
    };
    new ResizeObserver(resize).observe(viewport);
    resize();

    const settings = byId('settings');
    const torusScale = byId('torus-scale');
    const light = byId('light');
    const exterior = byId('exterior');
    const rgb = id => {
        const { r, g, b } = new THREE.Color(byId(id).value);
        return { r, g, b };
    };

    function syncOptions() {
        byId('size-value').textContent = torusScale.valueAsNumber.toFixed(2) + '\u00d7';
        byId('light-value').textContent = light.valueAsNumber.toFixed(1);
        const side = exterior.checked ? THREE.DoubleSide : THREE.FrontSide;
        if (material.side !== side) {
            material.side = side;
            material.needsUpdate = true;
        }
        draw();
    }

    const worker = new Worker(new URL('./worker.mjs', import.meta.url), { type: 'module' });
    let busy = false;
    let pending = null;
    let version = 0;
    let timer = 0;
    function pump() {
        if (busy || !pending) return;
        const request = pending;
        pending = null;
        busy = true;
        setState('Solving');
        worker.postMessage(request);
    }

    function queue() {
        pending = { version, settings: { left: rgb('left'), back: rgb('back'), right: rgb('right'),
            torusScale: torusScale.valueAsNumber, light: light.valueAsNumber } };
        pump();
    }

    function fail(error) {
        console.error(error);
        busy = false;
        setState('Solve failed', 'error');
        if (!box.visible) loader.textContent = 'Could not start the radiosity solver.';
    }

    worker.addEventListener('error', event => fail(event.message));
    worker.addEventListener('message', ({ data }) => {
        busy = false;
        if (data.error) {
            fail(data.error);
        } else if (data.version === version || !box.visible) {
            const { summary, positions, colors, elapsed } = data;
            // The worker may have rebuilt geometry for a result that was skipped.
            const geometry = new THREE.BufferGeometry();
            geometry.setAttribute('position', new THREE.BufferAttribute(positions, 3));
            geometry.setAttribute('color', new THREE.BufferAttribute(colors, 3));
            box.geometry.dispose();
            box.geometry = geometry;
            box.visible = true;
            loader.hidden = true;
            byId('patches').textContent = summary.patches.toLocaleString();
            byId('factors').textContent = summary.formFactors.toLocaleString();
            byId('iterations').textContent = summary.iterations.toLocaleString();
            byId('elapsed').textContent = elapsed >= 1000
                ? (elapsed / 1000).toFixed(2) + ' s' : Math.round(elapsed) + ' ms';
            if (data.version === version) setState('Solved', 'ready');
            draw();
        }
        pump();
    });

    function schedule() {
        clearTimeout(timer);
        pending = null;
        version++;
        setState('Solving');
        timer = setTimeout(queue, 120);
    }

    settings.addEventListener('input', event => {
        syncOptions();
        if (event.target !== exterior) schedule();
    });
    settings.addEventListener('reset', () => {
        queueMicrotask(() => { syncOptions(); schedule(); });
    });
    byId('reset-view').addEventListener('click', () => controls.reset());
    queue();
}

try {
    start();
} catch (error) {
    console.error(error);
    loader.textContent = 'Could not start the viewer. WebGL 2 is required.';
    setState('Viewer failed', 'error');
}
