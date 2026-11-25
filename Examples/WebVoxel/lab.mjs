import * as THREE from 'three';
import { OrbitControls } from '../vendor/OrbitControls.js';

const byId = id => document.getElementById(id);
const loader = byId('loader');
const status = byId('status');
const dash = '\u2013';

function report(label, kind = '') {
    status.textContent = label;
    status.dataset.kind = kind;
}

function start() {
    const canvas = byId('scene');
    const viewport = byId('viewport');
    const renderer = new THREE.WebGLRenderer({ canvas, antialias: true });
    renderer.setPixelRatio(Math.min(devicePixelRatio, 2));
    const scene = new THREE.Scene();
    scene.background = new THREE.Color(0x10171a);

    const camera = new THREE.OrthographicCamera(-5, 5, 5, -5, 0.1, 80);
    camera.up.set(0, 0, 1);
    camera.position.set(8, -11, 10);
    const controls = new OrbitControls(camera, canvas);
    controls.enablePan = false;
    controls.minZoom = 0.65;
    controls.maxZoom = 3;
    controls.minPolarAngle = 0.1;
    controls.maxPolarAngle = Math.PI - 0.1;
    controls.update();
    controls.saveState();

    const grid = new THREE.GridHelper(12, 12, 0x405255, 0x29383b);
    grid.rotation.x = Math.PI / 2;
    grid.position.z = -3.4;
    scene.add(grid, new THREE.HemisphereLight(0xe4fbf4, 0x3b4c52, 2.0));
    const key = new THREE.DirectionalLight(0xfff4d8, 2.4);
    key.position.set(-5, -7, 10);
    scene.add(key);

    const voxelGeometry = new THREE.BoxGeometry();
    const voxelMaterial = new THREE.MeshStandardMaterial({ roughness: 0.75 });
    const surface = new THREE.Mesh(new THREE.BufferGeometry(), new THREE.MeshStandardMaterial({
        color: 0x40588e, side: THREE.DoubleSide, roughness: 0.85,
        polygonOffset: true, polygonOffsetFactor: 1, polygonOffsetUnits: 1,
    }));
    const wire = new THREE.LineSegments(new THREE.BufferGeometry(), new THREE.LineBasicMaterial({
        color: 0xa9bbff, transparent: true, opacity: 0.7, depthWrite: false,
    }));
    surface.visible = wire.visible = false;
    scene.add(surface, wire);

    const rayGroup = new THREE.Group();
    const ray = new THREE.Line(new THREE.BufferGeometry(), new THREE.LineBasicMaterial({
        color: 0xffdc83, depthTest: false,
    }));
    const points = new THREE.Points(new THREE.BufferGeometry(), new THREE.PointsMaterial({
        color: 0xffead1, size: 7, sizeAttenuation: false, depthTest: false,
    }));
    ray.renderOrder = 3;
    points.renderOrder = 4;
    rayGroup.visible = false;
    rayGroup.add(ray, points);
    scene.add(rayGroup);

    const draw = () => renderer.render(scene, camera);
    controls.addEventListener('change', draw);
    const resize = () => {
        const { clientWidth: width, clientHeight: height } = viewport;
        if (!width || !height) return;
        const aspect = width / height;
        const halfHeight = Math.max(4.85, 4.6 / aspect);
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

    const shape = byId('shape');
    const resolution = byId('resolution');
    const layer = byId('layer');
    const rayVisible = byId('ray-visible');
    const format = value => value.toLocaleString();
    const transform = new THREE.Object3D();
    const base = new THREE.Color(0x4fcabd);
    const shade = new THREE.Color(0x176d6c);
    const highlight = new THREE.Color(0xffa34c);
    const color = new THREE.Color();
    let voxelMesh;
    let cells;
    let summary;
    let selected;
    let visibleOffsets = new Uint32Array();
    let mode = 'voxels';

    function updateInstances() {
        if (!voxelMesh) return;
        const cutoff = layer.valueAsNumber;
        const { cellSize, minZ, maxZ } = summary;
        let count = 0;
        transform.scale.setScalar(cellSize * 0.91);
        for (let i = 0; i < cells.length; i += 3) {
            const x = cells[i], y = cells[i + 1], z = cells[i + 2];
            if (z > cutoff) continue;
            transform.position.set((x + 0.5) * cellSize, (y + 0.5) * cellSize, (z + 0.5) * cellSize);
            transform.updateMatrix();
            voxelMesh.setMatrixAt(count, transform.matrix);
            color.copy(shade).lerp(base, (z - minZ) / Math.max(1, maxZ - minZ));
            if (rayVisible.checked && selected?.x === x && selected.y === y) color.copy(highlight);
            voxelMesh.setColorAt(count, color);
            visibleOffsets[count++] = i;
        }
        voxelMesh.count = count;
        voxelMesh.instanceMatrix.needsUpdate = true;
        if (voxelMesh.instanceColor) voxelMesh.instanceColor.needsUpdate = true;
        byId('layer-value').textContent = cutoff === maxZ ? 'All' : 'Z ' + cutoff;
        byId('occupied').textContent = format(count) + ' / ' + format(summary.cells);
        byId('reset-layer').disabled = cutoff === maxZ;
        draw();
    }

    function updateView() {
        if (voxelMesh) voxelMesh.visible = mode !== 'mesh';
        surface.visible = !!summary && mode === 'mesh';
        wire.visible = !!summary && mode !== 'voxels';
        wire.material.opacity = mode === 'overlay' ? 0.29 : 0.7;
        wire.material.depthTest = mode !== 'overlay';
        rayGroup.visible = !!selected && rayVisible.checked;
        updateInstances();
        draw();
    }

    function populate(data) {
        const geometry = new THREE.BufferGeometry();
        geometry.setAttribute('position', new THREE.BufferAttribute(data.vertices, 3));
        geometry.setIndex(new THREE.BufferAttribute(data.indices, 1));
        geometry.computeVertexNormals();
        surface.geometry.dispose();
        surface.geometry = geometry;
        wire.geometry.dispose();
        wire.geometry = new THREE.WireframeGeometry(geometry);

        if (voxelMesh) {
            scene.remove(voxelMesh);
            voxelMesh.dispose();
        }
        summary = data.summary;
        cells = data.cells;
        selected = null;
        visibleOffsets = new Uint32Array(summary.cells);
        voxelMesh = new THREE.InstancedMesh(voxelGeometry, voxelMaterial, summary.cells);
        // All cells fit inside the fixed sweep volume, including when sliced.
        voxelMesh.boundingSphere = new THREE.Sphere(new THREE.Vector3(), 8);
        voxelMesh.instanceMatrix.setUsage(THREE.DynamicDrawUsage);
        scene.add(voxelMesh);
        layer.disabled = !summary.cells;
        layer.min = summary.minZ;
        layer.max = summary.maxZ;
        layer.value = summary.maxZ;
        byId('rays').textContent = format(summary.rays);
        byId('intersections').textContent = format(summary.intersections);
        byId('column').textContent = dash;
        byId('column-hits').textContent = dash;
        loader.hidden = true;
        updateView();
    }

    const worker = new Worker(new URL('./worker.mjs', import.meta.url), { type: 'module' });
    let version = 0;
    let shownVersion = -1;
    let columnId = 0;
    let busy = false;
    let pending = null;
    let timer = 0;
    let bound = 0;

    function pump() {
        if (busy || !pending) return;
        const request = pending;
        pending = null;
        busy = true;
        worker.postMessage(request);
    }

    function schedule(delay = 130) {
        clearTimeout(timer);
        pending = null;
        version++;
        report('Voxelizing');
        const count = resolution.valueAsNumber;
        byId('resolution-value').textContent = count + '\u00d7' + count;
        timer = setTimeout(() => {
            pending = { type: 'build', version, shape: Number(shape.value), resolution: count };
            pump();
        }, delay);
    }

    function selectColumn(x, y) {
        if (shownVersion !== version || x < -bound || x >= bound || y < -bound || y >= bound) return;
        selected = { x, y };
        byId('column').textContent = x + ', ' + y;
        byId('column-hits').textContent = dash;
        const wx = x * summary.cellSize, wy = y * summary.cellSize;
        ray.geometry.dispose();
        ray.geometry = new THREE.BufferGeometry().setFromPoints([
            new THREE.Vector3(wx, wy, -3.65), new THREE.Vector3(wx, wy, 3.65),
        ]);
        points.geometry.dispose();
        points.geometry = new THREE.BufferGeometry();
        points.visible = false;
        worker.postMessage({ type: 'column', version, id: ++columnId, x, y });
        updateView();
    }

    function selectSampleColumn() {
        const torus = shape.value === '0';
        const targetX = torus ? 2.6 / summary.cellSize : 0;
        const targetY = torus ? 1 : 0;
        let closest = 0;
        let distance = Infinity;
        for (let i = 0; i < cells.length; i += 3) {
            const d = (cells[i] - targetX) ** 2 + (cells[i + 1] - targetY) ** 2;
            if (d < distance) { closest = i; distance = d; }
        }
        if (cells.length) selectColumn(cells[closest], cells[closest + 1]);
    }

    function fail(error) {
        console.error(error);
        report('Voxelization failed', 'error');
        if (!summary) loader.textContent = 'Could not start the voxelization solver.';
    }

    worker.addEventListener('error', event => { busy = false; fail(event.message); });
    worker.addEventListener('message', ({ data }) => {
        if (data.type === 'build') {
            busy = false;
            if (data.version === version) {
                if (data.error) {
                    fail(data.error);
                } else {
                    shownVersion = data.version;
                    bound = Math.floor(resolution.valueAsNumber / 2);
                    populate(data);
                    report('Ready', 'ready');
                    selectSampleColumn();
                }
            }
            pump();
        } else if (data.version === version && data.version === shownVersion && data.id === columnId) {
            if (data.error) {
                fail(data.error);
            } else {
                points.geometry.setAttribute('position', new THREE.BufferAttribute(data.hits, 3));
                points.visible = data.hits.length > 0;
                byId('column-hits').textContent = format(data.hits.length / 3);
                draw();
            }
        }
    });

    const clickRay = new THREE.Raycaster();
    const xy = new THREE.Plane(new THREE.Vector3(0, 0, 1));
    let press;
    canvas.addEventListener('pointerdown', event => {
        press = event.isPrimary && event.button === 0 ? [event.pointerId, event.clientX, event.clientY] : null;
    });
    canvas.addEventListener('pointercancel', () => { press = null; });
    canvas.addEventListener('pointerup', event => {
        if (!press || press[0] !== event.pointerId) return;
        const moved = Math.hypot(event.clientX - press[1], event.clientY - press[2]);
        press = null;
        if (moved > 5 || !summary) return;
        const rect = canvas.getBoundingClientRect();
        clickRay.setFromCamera(new THREE.Vector2(
            2 * (event.clientX - rect.left) / rect.width - 1,
            1 - 2 * (event.clientY - rect.top) / rect.height,
        ), camera);
        if (voxelMesh.visible) {
            const hit = clickRay.intersectObject(voxelMesh, false)[0];
            if (hit?.instanceId !== undefined) {
                const i = visibleOffsets[hit.instanceId];
                selectColumn(cells[i], cells[i + 1]);
                return;
            }
        }
        const position = (surface.visible && clickRay.intersectObject(surface, false)[0]?.point)
            || clickRay.ray.intersectPlane(xy, new THREE.Vector3());
        if (position) selectColumn(Math.round(position.x / summary.cellSize), Math.round(position.y / summary.cellSize));
    });

    shape.addEventListener('change', () => schedule(0));
    resolution.addEventListener('input', () => schedule());
    layer.addEventListener('input', updateInstances);
    rayVisible.addEventListener('change', updateView);
    document.querySelector('.mode').addEventListener('change', event => {
        mode = event.target.value;
        updateView();
    });
    byId('reset-layer').addEventListener('click', () => {
        if (summary) { layer.value = layer.max; updateInstances(); }
    });
    byId('reset-view').addEventListener('click', () => controls.reset());
    schedule(0);
}

try {
    start();
} catch (error) {
    console.error(error);
    report('Viewer failed', 'error');
    loader.textContent = 'Could not start the viewer. WebGL 2 is required.';
}
