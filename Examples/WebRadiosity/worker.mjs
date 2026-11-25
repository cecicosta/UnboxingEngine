import createSolver from './solver.mjs';

const ready = createSolver();
ready.catch(() => {}); // Report startup failures in response to the first request.

self.onmessage = async ({ data: { version, settings } }) => {
    try {
        const solver = await ready;
        const start = performance.now();
        const summary = solver.solve(settings);
        const positions = solver.positions().slice();
        const colors = solver.colors().slice();
        self.postMessage({ version, summary, positions, colors, elapsed: performance.now() - start },
            [positions.buffer, colors.buffer]);
    } catch (error) {
        self.postMessage({ version, error: String(error) });
    }
};
