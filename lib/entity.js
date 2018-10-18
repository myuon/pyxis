'use strict';

const djv = require('djv');
const schema = new djv();

schema.addSchema('project', { "type": "number" });

console.log(schema.validate('project#', '111'));
console.log(schema.validate('project#', 10.4));

