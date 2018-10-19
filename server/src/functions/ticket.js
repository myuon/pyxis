'use strict';

const lib = require('pyxis-lib');

const getSDK = (isOffline) => {
  return isOffline
    ? require('aws-sdk')
    : require('aws-xray-sdk-core').captureAWS(require('aws-sdk'));
};

const getDBClient = (isOffline) => {
  const AWS = getSDK(isOffline);

  return isOffline
    ? new AWS.DynamoDB.DocumentClient({
      region: 'localhost',
      endpoint: 'http://localhost:8000'
    })
    : new AWS.DynamoDB.DocumentClient();
};

module.exports.get = async (event, context) => {
  const isOffline = 'isOffline' in event && event.isOffline;
  const db = getDBClient(isOffline);
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
