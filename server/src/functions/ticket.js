'use strict';

const lib = require('pyxis-lib');

const xray = (sdk) => process.env.IS_OFFLINE ? sdk : require('aws-xray-sdk-core').captureAWS(sdk);
const AWS = xray(require('aws-sdk'));
const db = process.env.IS_OFFLINE
  ? new AWS.DynamoDB.DocumentClient({
    region: 'localhost',
    endpoint: 'http://localhost:8000'
  })
  : new AWS.DynamoDB.DocumentClient();

module.exports.get = async (event, context) => {
  const result = db.query({
    TableName: 'tickets',
    KeyConditionExpression: 'id = :id and sort = :sort',
    ExpressionAttributeValues: {
      ':id': `${event.pathParameters.projectId}-${event.pathParameters.ticketId}`,
      ':sort': 'detail',
    }
  }, (err, data) => {
    return err != null ? err : data;
  }).promise();

  let ticket = (await result)['Items'][0];
  return {
    statusCode: 200,
    body: JSON.stringify(lib.validate(lib.ticket, {
      id: ticket.id,
      title: ticket.title,
      assigned_to: ticket.detail.assigned_to,
    })),
  };
};
