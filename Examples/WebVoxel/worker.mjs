import createSolver from './solver.mjs';

const ready = createSolver();
ready.catch(() => {}); // Reply to the request with the startup error.
let currentVersion = -1;

self.onmessage = async ({ data }) => {
    const { version, type } = data;
    try {
        const solver = await ready;
        if (type === 'build') {
            const summary = solver.voxelize(data.shape, data.resolution);
            currentVersion = version;
            const vertices = solver.vertices().slice();
            const indices = solver.indices().slice();
            const cells = solver.cells().slice();
            self.postMessage({ type, version, summary, vertices, indices, cells },
                [vertices.buffer, indices.buffer, cells.buffer]);
        } else if (type === 'column' && version === currentVersion) {
            const hits = solver.column(data.x, data.y).slice();
            self.postMessage({ type, version, id: data.id, hits }, [hits.buffer]);
        }
    } catch (error) {
        self.postMessage({ type, version, id: data.id, error: String(error) });
    }
};
