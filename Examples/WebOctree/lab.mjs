import * as THREE from 'three';
import { OrbitControls } from './vendor/OrbitControls.js';
import createOctree from './octree.mjs';

const byId = id => document.getElementById(id);

async function start() {
    const lab = await createOctree();
    const canvas = byId('scene');
    const viewport = byId('viewport');
    const renderer = new THREE.WebGLRenderer({ canvas, antialias: true });
    renderer.setPixelRatio(Math.min(devicePixelRatio, 2));

    const scene = new THREE.Scene();
    scene.background = new THREE.Color(0x10171a);
    const camera = new THREE.OrthographicCamera(-8, 8, 8, -8, 0.1, 80);
    camera.up.set(0, 0, 1);
    const controls = new OrbitControls(camera, canvas);
    controls.enablePan = false;
    controls.minZoom = 0.65;
    controls.maxZoom = 2.6;
    controls.minPolarAngle = 0.1;
    controls.maxPolarAngle = Math.PI - 0.1;

    const grid = new THREE.GridHelper(18, 18, 0x344449, 0x273438);
    grid.rotation.x = Math.PI / 2;
    grid.position.z = -1.6;
    scene.add(grid);
    scene.add(new THREE.HemisphereLight(0xbcf8ec, 0x17262d, 2.0));
    const key = new THREE.DirectionalLight(0xf2f8f4, 2.3);
    key.position.set(-5, -6, 9);
    scene.add(key);

    const torus = new THREE.BufferGeometry();
    const vertices = new THREE.BufferAttribute(lab.vertices().slice(), 3);
    const indices = lab.indices().slice();
    torus.setAttribute('position', vertices);
    torus.setIndex(new THREE.BufferAttribute(indices, 1));
    torus.computeVertexNormals();
    scene.add(new THREE.Mesh(torus, new THREE.MeshStandardMaterial({
        color: 0x216d66, roughness: 0.65, side: THREE.DoubleSide,
        polygonOffset: true, polygonOffsetFactor: 1, polygonOffsetUnits: 1,
    })));
    scene.add(new THREE.LineSegments(new THREE.WireframeGeometry(torus), new THREE.LineBasicMaterial({
        color: 0x78e0d0, transparent: true, opacity: 0.45,
    })));

    const nodeMaterials = [
        new THREE.LineBasicMaterial({ color: 0x678186, transparent: true, opacity: 0.19, depthWrite: false }),
        new THREE.LineBasicMaterial({ color: 0x9bafff, transparent: true, opacity: 0.60, depthTest: false }),
        new THREE.LineBasicMaterial({ color: 0xf0a24b, transparent: true, opacity: 0.66, depthTest: false }),
        new THREE.LineBasicMaterial({ color: 0xff785c, depthTest: false }),
    ];
    const bounds = lab.nodeBounds().slice();
    const boxes = [];
    for (let i = 0; i < bounds.length; i += 6) {
        const min = new THREE.Vector3().fromArray(bounds, i);
        const max = new THREE.Vector3().fromArray(bounds, i + 3);
        const box = new THREE.Box3Helper(new THREE.Box3(min, max));
        box.material.dispose();
        box.material = nodeMaterials[0];
        scene.add(box);
        boxes.push(box);
    }

    const beamPositions = new THREE.BufferAttribute(new Float32Array(6), 3);
    const beamGeometry = new THREE.BufferGeometry().setAttribute('position', beamPositions);
    const beam = new THREE.Line(beamGeometry, new THREE.LineBasicMaterial({
        color: 0xf9dc83, depthTest: false,
    }));
    const pulse = new THREE.Mesh(new THREE.SphereGeometry(0.075, 12, 8), new THREE.MeshBasicMaterial({
        color: 0xffe8a3, depthTest: false,
    }));
    const impact = new THREE.Mesh(new THREE.SphereGeometry(0.105, 16, 8), new THREE.MeshBasicMaterial({
        color: 0xff785c, depthTest: false,
    }));
    const facePositions = new THREE.BufferAttribute(new Float32Array(9), 3);
    const faceGeometry = new THREE.BufferGeometry().setAttribute('position', facePositions);
    const face = new THREE.Mesh(faceGeometry, new THREE.MeshBasicMaterial({
        color: 0xff785c, side: THREE.DoubleSide, depthTest: false,
    }));
    beam.renderOrder = 4;
    pulse.renderOrder = 5;
    face.renderOrder = 6;
    impact.renderOrder = 7;
    beam.frustumCulled = face.frustumCulled = false;
    beam.visible = pulse.visible = impact.visible = face.visible = false;
    scene.add(beam, pulse, impact, face);

    const draw = () => renderer.render(scene, camera);
    controls.addEventListener('change', draw);
    const resize = () => {
        const { clientWidth: width, clientHeight: height } = viewport;
        if (!width || !height) return;
        const aspect = width / height;
        const halfHeight = Math.max(7.0, 7.8 / aspect);
        camera.left = -halfHeight * aspect;
        camera.right = halfHeight * aspect;
        camera.top = halfHeight;
        camera.bottom = -halfHeight;
        camera.updateProjectionMatrix();
        renderer.setSize(width, height, false);
        draw();
    };
    new ResizeObserver(resize).observe(viewport);

    function view(x, y, z) {
        camera.position.set(x, y, z);
        camera.zoom = 1;
        camera.updateProjectionMatrix();
        controls.update();
        draw();
    }

    const states = new Uint8Array(boxes.length);
    const allNodes = byId('all-nodes');
    function recolor() {
        boxes.forEach((box, i) => {
            box.material = nodeMaterials[states[i]];
            box.visible = allNodes.checked || states[i] > 0;
            box.renderOrder = states[i];
        });
    }
    allNodes.addEventListener('change', () => { recolor(); draw(); });

    const outcome = byId('outcome');
    const dash = '\u2013';
    const reducedMotion = matchMedia('(prefers-reduced-motion: reduce)');
    let animation = 0;

    function showResult(result) {
        const hit = result && result.triangle >= 0;
        outcome.textContent = hit ? 'Hit' : result ? 'Miss' : 'No ray';
        outcome.dataset.kind = hit ? 'hit' : '';
        byId('tested').textContent = result ? result.candidates + ' / ' + indices.length / 3 : dash;
        byId('visited').textContent = result ? result.visited + ' / ' + boxes.length : dash;
        byId('triangle').textContent = hit ? '#' + result.triangle : dash;
        byId('distance').textContent = hit ? result.distance.toFixed(2) + ' units' : dash;
        byId('clear').disabled = !result;
    }

    function shoot(origin, direction) {
        cancelAnimationFrame(animation);
        const result = lab.shoot(origin.x, origin.y, origin.z, direction.x, direction.y, direction.z);
        states.set(lab.nodeStates());
        recolor();
        showResult(result);

        const hit = result.triangle >= 0;
        const nearest = Math.max(0, -origin.dot(direction));
        const endDistance = hit ? result.distance : nearest + 7;
        const startDistance = Math.max(0, Math.min(nearest, endDistance) - 7);
        const start = origin.clone().addScaledVector(direction, startDistance);
        const end = hit ? new THREE.Vector3(result.x, result.y, result.z)
            : origin.clone().addScaledVector(direction, endDistance);
        const tip = new THREE.Vector3();
        beamPositions.setXYZ(0, start.x, start.y, start.z);
        beam.visible = true;
        impact.visible = face.visible = false;
        if (hit) {
            impact.position.copy(end);
            const vertex = new THREE.Vector3();
            for (let i = 0; i < 3; ++i) {
                vertex.fromBufferAttribute(vertices, indices[3 * result.triangle + i]);
                facePositions.setXYZ(i, vertex.x, vertex.y, vertex.z);
            }
            facePositions.needsUpdate = true;
        }
        const started = performance.now();
        function animate(time) {
            const progress = reducedMotion.matches ? 1 : Math.min(1, (time - started) / 360);
            tip.lerpVectors(start, end, progress);
            beamPositions.setXYZ(1, tip.x, tip.y, tip.z);
            beamPositions.needsUpdate = true;
            pulse.position.copy(tip);
            pulse.visible = progress < 1;
            impact.visible = face.visible = hit && progress === 1;
            draw();
            if (progress < 1) animation = requestAnimationFrame(animate);
        }
        animate(started);
    }

    function clear() {
        cancelAnimationFrame(animation);
        states.fill(0);
        beam.visible = pulse.visible = impact.visible = face.visible = false;
        showResult(null);
        recolor();
        draw();
    }

    let press;
    canvas.addEventListener('pointerdown', event => {
        press = event.isPrimary && event.button === 0 ? [event.pointerId, event.clientX, event.clientY] : null;
    });
    canvas.addEventListener('pointercancel', () => { press = null; });
    canvas.addEventListener('pointerup', event => {
        if (!press || press[0] !== event.pointerId) return;
        const moved = Math.hypot(event.clientX - press[1], event.clientY - press[2]);
        press = null;
        if (moved > 5) return;
        const rect = canvas.getBoundingClientRect();
        const x = 2 * (event.clientX - rect.left) / rect.width - 1;
        const y = 1 - 2 * (event.clientY - rect.top) / rect.height;
        const origin = new THREE.Vector3(x, y, -1).unproject(camera);
        const direction = camera.getWorldDirection(new THREE.Vector3());
        shoot(origin, direction);
    });

    const sample = () => shoot(new THREE.Vector3(4.17, 0.27, 9), new THREE.Vector3(0, 0, -1));
    byId('isometric').addEventListener('click', () => view(9, -12, 14));
    byId('side').addEventListener('click', () => view(0, -18, 1.8));
    byId('sample').addEventListener('click', sample);
    byId('clear').addEventListener('click', clear);
    for (const id of ['isometric', 'side', 'sample', 'all-nodes']) byId(id).disabled = false;
    view(9, -12, 14);
    resize();
    sample();
    byId('loader').hidden = true;
}

start().catch(error => {
    console.error(error);
    byId('loader').textContent = 'Could not start the lab. WebAssembly and WebGL 2 are required.';
});
