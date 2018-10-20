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

module.exports.listRecent = async (event, context) => {
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
