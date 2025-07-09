const fs = require('fs');
const path = require('path');
const glob = require('glob');
const jsdoc2md = require('jsdoc-to-markdown');

const dir = __dirname;
if (!fs.existsSync(dir))
    fs.mkdirSync(dir);
for (const file of glob.sync('src/js/*.js')) {
    const baseName = path.basename(file, '.js').toLowerCase();
    const outputPath = path.join(dir, `${baseName}.md`);
    console.log(outputPath);
    const template = `
    {{>main}}
  `;

    jsdoc2md.render({ files: file }).then((md) => {
        if (md.trim()) fs.writeFileSync(outputPath, md);
    });
}