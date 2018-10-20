'use strict';

const lib = require('pyxis-lib');
const db = require('./db').tables;

module.exports.create = async (event, context) => {
  try {
    const comment = lib.validate(lib.comment, JSON.parse(event.body));
    const ticketInfo = await db.comment.get(
      event.pathParameters.projectId,
      event.pathParameters.ticketId,
    );
    ticketInfo.comment = (parseInt(ticketInfo.comment, 10) + 1).toString();

    await db.comment.create(
      event.pathParameters.projectId,
      event.pathParameters.ticketId,
      ticketInfo,
    );
    await db.ticket.update(
      event.pathParameters.projectId,
      event.pathParameters.ticketId,
      'comment',
      ticketInfo.comment,
    );

    return {
      statusCode: 200,
      body: JSON.stringify({}),
    };
  } catch (err) {
    console.error(err);
    return {
      statusCode: 500,
      body: JSON.stringify(err),
    };
  }
};

module.exports.list = async (event, context) => {
  try {
    const result = await db.comment.list(
      event.pathParameters.projectId,
      event.pathParameters.ticketId,
    );

    return {
      statusCode: 200,
      body: JSON.stringify(result),
    };
  } catch (err) {
    console.error(err);
    return {
      statusCode: 500,
      body: JSON.stringify(err),
    };
  }
};
