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
    KeyConditionExpression: 'id = :id and begins_with(sort, :sort)',
    ExpressionAttributeValues: {
      ':id': `${event.pathParameters.projectId}-${event.pathParameters.ticketId}`,
      ':sort': 'page',
    }
  }, (err, data) => {
    return err != null ? err : data;
  }).promise();

  let pages = (await result)['Items'];
  return {
    statusCode: 200,
    body: JSON.stringify(pages.map(page => {
      return lib.validate(lib.page, {
        id: page.sort.split('-')[1],
        belongs_to: page.id,
        title: page.title,
        content: page.content,
      });
    })),
  };
};
