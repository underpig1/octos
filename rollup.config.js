import resolve from '@rollup/plugin-node-resolve';
import commonjs from '@rollup/plugin-commonjs';
import terser from '@rollup/plugin-terser';

export default {
    input: 'src/js/index.js',
    output: {
        file: 'octos.min.js',
        format: 'umd',
        name: 'octos',
        sourcemap: false
    },
    plugins: [resolve(), commonjs(), terser()]
};