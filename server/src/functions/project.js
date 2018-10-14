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

module.exports.get = async (event, context) => {
  const isOffline = 'isOffline' in event && event.isOffline;
  const db = getDBClient(isOffline);
  const project = db.query({
    TableName: 'projects',
    KeyConditionExpression: 'id = :id and sort = :sort',
    ExpressionAttributeValues: {
      ':id': event.pathParameters.id,
      ':sort': 'detail',
    }
  }, (err, data) => {
    return err != null ? err : data;
  }).promise();

  return {
    statusCode: 200,
    body: JSON.stringify((await project)['Items'][0]),
  };
};
