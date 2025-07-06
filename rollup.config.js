import resolve from '@rollup/plugin-node-resolve';
import commonjs from '@rollup/plugin-commonjs';
import terser from '@rollup/plugin-terser';

export default {
    input: 'src/js/index.js',
    output: {
        file: 'out/octos.min.js',
        format: 'umd',
        name: 'octos',
        sourcemap: true
    },
    plugins: [resolve(), commonjs(), terser()]
};