'use strict';

const lib = require('pyxis-lib');
const db = require('./db').tables;

module.exports.get = async (event, context) => {
  try {
    const result = await db.page.list(
      event.pathParameters.ticketId,
    );

    return {
      statusCode: 200,
      body: JSON.stringify(result),
    };
  } catch (err) {
    return {
      statusCode: 500,
      body: JSON.stringify(err),
    };
  }
};
