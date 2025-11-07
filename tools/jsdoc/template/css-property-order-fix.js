const fs = require('fs');
const path = require('path');

function parseCSS(cssContent) {
    // Implement your CSS parsing logic here
    // You can use regular expressions, string manipulation, or a CSS parsing library

    // Example: Simple parsing using regular expressions to extract CSS rules
    const cssRules = [];
    const ruleRegex = /(.+?)\s*{\s*(.*?)\s*}/gs;
    let match;

    while ((match = ruleRegex.exec(cssContent)) !== null) {
        const selector = match[1].trim();
        const declarations = match[2].split(';').map((declaration) => {
            const parts = declaration.split(':').map((part) => part.trim());

            return {
                property: parts[0],
                value: parts[1],
            };
        });

        cssRules.push({
            selector,
            declarations: declarations
                .sort((a, b) => a.property.localeCompare(b.property))
                .filter((declaration) => declaration.property !== ''),
        });
    }

    return cssRules;
}

const cssFile = fs.readFileSync(
    path.join('.', 'static', 'styles', 'clean-jsdoc-theme-light.css')
);

const parsed = parseCSS(cssFile);

let output = '';

for (const parse of parsed) {
    output += `${parse.selector} {\n`;

    for (const declaration of parse.declarations) {
        output += `\t${declaration.property}: ${declaration.value};\n`;
    }
    output += '\n}\n\n';
}

fs.writeFileSync(
    path.join('.', 'static', 'styles', 'clean-jsdoc-theme-list.update.css'),
    output
);

