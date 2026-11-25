import * as THREE from 'three';
import shaders from './shaders.mjs';

const cameraDistance = 100;
const cameraFar = 300;
const fieldScale = 1.5e15;
const targetCount = 6;

const uniform = value => ({ value });
const rgba = (r, g, b) => new THREE.Vector4(r, g, b, 1);

function material(vertexShader, fragmentShader, uniforms, options = {}) {
    return new THREE.RawShaderMaterial({
        vertexShader, fragmentShader, uniforms,
        glslVersion: THREE.GLSL3,
        depthTest: false, depthWrite: false, blending: THREE.NoBlending,
        ...options,
    });
}

function attributes(geometry) {
    geometry.setAttribute('i_position', geometry.getAttribute('position'));
    return geometry;
}

function target(size) {
    return new THREE.WebGLRenderTarget(size, size, {
        type: THREE.FloatType, format: THREE.RGBAFormat, internalFormat: 'RGBA32F',
        minFilter: THREE.NearestFilter, magFilter: THREE.NearestFilter,
        depthBuffer: false, generateMipmaps: false,
    });
}

export class FieldsPipeline {
    constructor(renderer) {
        this.renderer = renderer;
        const gl = renderer.getContext();
        if (!gl.getExtension('EXT_color_buffer_float') || !gl.getExtension('EXT_float_blend')) {
            throw new Error('This sample needs WebGL 2 with float texture rendering and blending.');
        }
        this.maxResolution = gl.getParameter(gl.MAX_TEXTURE_SIZE);
        renderer.autoClear = false;
        this.scene = new THREE.Scene();
        this.camera = new THREE.Camera();
        this.targets = [];
        this.reference = 'native';
        this.steps = 0;

        const scanCamera = new THREE.PerspectiveCamera(60, 1, 1, cameraFar);
        scanCamera.position.z = cameraDistance;
        scanCamera.updateMatrixWorld();
        this.scan = material(shaders.scanVertex, shaders.scanFragment, {
            color: uniform(rgba(1, 1, 1)),
            u_camera_world_position: uniform(scanCamera.position),
            u_camera_far: uniform(cameraFar),
            u_model_matrix: uniform(new THREE.Matrix4()),
            u_view_matrix: uniform(scanCamera.matrixWorldInverse),
            u_projection_matrix: uniform(scanCamera.projectionMatrix),
        }, {
            side: THREE.DoubleSide, transparent: true, forceSinglePass: true,
            blending: THREE.CustomBlending, blendEquation: THREE.AddEquation,
            blendSrc: THREE.OneFactor, blendDst: THREE.OneFactor,
        });
        this.potential = material(shaders.quadVertex, shaders.potentialFragment, {
            color: uniform(rgba(1, 1, 1)),
            u_texture: uniform(null),
            u_camera_far: uniform(cameraFar),
            u_visualization_scale: uniform(1),
        });
        this.combine = material(shaders.quadVertex, shaders.combineFragment, {
            color: uniform(rgba(1, 0, 0)),
            u_texture: uniform(null),
            u_electron: uniform(null),
            u_visualization_scale: uniform(1),
        });
        this.flow = material(shaders.quadVertex, shaders.flowFragment, {
            color: uniform(rgba(0, 0, 1)),
            u_texture: uniform(null),
            u_electron: uniform(null),
            u_visualization_scale: uniform(1e16),
        });
        this.display = material(shaders.quadVertex, shaders.presentFragment, {
            color: uniform(rgba(1, 1, 1)),
            u_texture: uniform(null),
            u_visualization_scale: uniform(1),
        }, {
            transparent: true,
            blending: THREE.CustomBlending, blendEquation: THREE.AddEquation,
            blendSrc: THREE.SrcAlphaFactor, blendDst: THREE.OneMinusSrcAlphaFactor,
        });

        this.nucleusSphere = attributes(new THREE.SphereGeometry(40, 32, 16).rotateX(Math.PI / 2));
        this.electronSphere = attributes(new THREE.SphereGeometry(10, 32, 16).rotateX(Math.PI / 2));
        this.quad = attributes(new THREE.PlaneGeometry(1, 1));
        this.mesh = new THREE.Mesh(this.quad, this.potential);
        this.mesh.frustumCulled = false;
        this.scene.add(this.mesh);
    }

    setResolution(size) {
        if (!Number.isInteger(size) || size < 64 || size > Math.min(1024, this.maxResolution)) {
            throw new Error('Unsupported shader resolution: ' + size);
        }
        if (this.resolution !== size) {
            this.renderer.setRenderTarget(null);
            this.targets.forEach(previous => previous.dispose());
            this.targets = Array.from({ length: targetCount }, () => target(size));
            this.resolution = size;
        }
        this.reset();
    }

    draw(targetBuffer, shader, geometry = this.quad, clear = false) {
        const { renderer, mesh, scene, camera } = this;
        renderer.setRenderTarget(targetBuffer);
        if (clear) {
            renderer.setClearColor(0, 0);
            renderer.clear(true, false, false);
        }
        mesh.geometry = geometry;
        mesh.material = shader;
        renderer.render(scene, camera);
    }

    seedPotential(scratch, output, geometry, y) {
        this.scan.uniforms.u_model_matrix.value.makeTranslation(0, y, 0);
        this.draw(scratch, this.scan, geometry, true);
        this.potential.uniforms.u_texture.value = scratch.texture;
        this.draw(output, this.potential, this.quad, true);
    }

    reset() {
        const [scan, nucleus, electron, ping, pong] = this.targets;
        this.renderer.setScissorTest(false);
        this.seedPotential(scan, nucleus, this.nucleusSphere, 0);
        this.seedPotential(scan, electron, this.electronSphere, 20);
        this.combine.uniforms.u_texture.value = nucleus.texture;
        this.combine.uniforms.u_electron.value = electron.texture;
        this.draw(ping, this.combine);
        this.combine.uniforms.u_electron.value = ping.texture;
        this.draw(pong, this.combine);
        this.current = pong;
        this.next = ping;
        this.steps = 0;
        this.updateFlow();
    }

    advance(count = 1) {
        this.renderer.setScissorTest(false);
        for (let i = 0; i < count; i++) {
            this.combine.uniforms.u_electron.value = this.current.texture;
            this.draw(this.next, this.combine);
            [this.current, this.next] = [this.next, this.current];
            this.steps++;
        }
        this.updateFlow();
    }

    setFlowReference(reference) {
        if (reference === this.reference) return;
        this.reference = reference;
        if (this.current) this.updateFlow();
    }

    updateFlow() {
        this.flow.uniforms.u_texture.value = this.reference === 'native'
            ? this.current.texture : this.targets[1].texture;
        this.flow.uniforms.u_electron.value = this.current.texture;
        this.draw(this.targets[5], this.flow, this.quad, true);
    }

    present(potential, flow) {
        const { renderer, display } = this;
        const size = renderer.getSize(new THREE.Vector2());
        renderer.setRenderTarget(null);
        renderer.setScissorTest(false);
        renderer.setViewport(0, 0, size.x, size.y);
        renderer.setClearColor(0x050709, 1);
        renderer.clear(true, false, false);
        const drawPanel = (region, texture, scale) => {
            if (region[2] < 1 || region[3] < 1) return;
            renderer.setViewport(...region);
            display.uniforms.u_texture.value = texture;
            display.uniforms.u_visualization_scale.value = scale;
            this.draw(null, display);
        };
        drawPanel(potential, this.current.texture, fieldScale);
        drawPanel(potential, this.targets[1].texture, fieldScale);
        drawPanel(flow, this.targets[5].texture, 1);
    }
}
