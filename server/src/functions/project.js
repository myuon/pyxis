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

const ME = '1';

module.exports.listRecent = async (event, context) => {
  const isOffline = 'isOffline' in event && event.isOffline;
  const db = getDBClient(isOffline);
  const project = db.query({
    TableName: 'users',
    KeyConditionExpression: 'id = :id and begins_with(sort, :sort)',
    ExpressionAttributeValues: {
      ':id': ME,
      ':sort': 'project',
    }
  }, (err, data) => {
    return err != null ? err : data;
  }).promise();

  let list = (await project)['Items'];
  return {
    statusCode: 200,
    body: JSON.stringify(list.map(item => {
      return lib.validate(lib.project, {
        id: item.sort.replace('project-', ''),
        title: item.title,
        tickets: item.tickets,
      });
    })),
  };
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
    body: JSON.stringify(lib.validate(lib.project, (await project)['Items'][0])),
  };
};
