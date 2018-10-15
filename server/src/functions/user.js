'use strict';

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

const ME = '1';

module.exports.getMe = async (event, context) => {
  const isOffline = 'isOffline' in event && event.isOffline;
  const db = getDBClient(isOffline);
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
    body: JSON.stringify((await result)['Items'][0]),
  };
};
