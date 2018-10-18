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
      return {
        id: page.sort.split('-')[1],
        belongs_to: page.id,
        title: page.title,
        content: page.content,
      };
    })),
  };
};
