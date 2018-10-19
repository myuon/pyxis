'use strict';

const project = {
  type: 'object',
  properties: {
    id: 'string',
    title: 'string',
    tickets: {
      type: 'array',
      items: 'string',
    }
  },
};

const ticket = {
  type: 'object',
  properties: {
    id: 'string',
    title: 'string',
    assigned_to: {
      type: 'array',
      items: {
        type: 'string',
      },
    }
  }
};

const comment = {
  type: 'object',
  properties: {
    id: 'string',
    content: 'string',
    created_at: 'string',
    owner: 'string',
    belongs_to: 'string',
  }
};

const page = {
  type: 'object',
  properties: {
    id: 'string',
    title: 'string',
    content: 'string',
    belongs_to: 'string',
  }
};

const user = {
  type: 'object',
  properties: {
    id: 'string',
    name: 'string',
    display_name: 'string',
    created_at: 'string',
  }
};

const validate = (schema, object) => {
  // if specified type is base type, you can omit { type: ... }
  if (typeof schema === 'string' || typeof schema === 'number') {
    schema = {
      type: schema
    };
  }
  
  if (schema.type === 'string' && typeof object === 'string') {
    return object;
  } else if (schema.type === 'number' && typeof object === 'number') {
    return object;
  } else if (schema.type === 'array' && object instanceof Array) {
    return object.map(item => validate(schema.items, item));
  } else if (schema.type === 'object' && typeof object === 'object') {
    const result = {};
    for (let key in schema.properties) {
      result[key] = validate(schema.properties[key], object[key]);
    }
    return result;
  } else {
    return Error(`Schema check failed: ${JSON.stringify(object)} should be form of ${JSON.stringify(schema)}`);
  }
};

module.exports = {
  project,
  ticket,
  comment,
  page,
  user,

  validate,
};
