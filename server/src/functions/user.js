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

const ME = '1';

module.exports.getMe = async (event, context) => {
  const result = db.query({
    TableName: 'users',
    KeyConditionExpression: 'id = :id and sort = :sort',
    ExpressionAttributeValues: {
      ':id': ME,
      ':sort': 'detail',
    }
  }, (err, data) => {
    return err != null ? err : data;
  }).promise();

  return {
    statusCode: 200,
    headers: { 'Access-Control-Allow-Origin': '*' },
    body: JSON.stringify(lib.validate(lib.user, (await result)['Items'][0])),
  };
};
