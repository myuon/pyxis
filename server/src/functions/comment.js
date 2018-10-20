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

module.exports.create = async (event, context) => {
  const comment = lib.validate(lib.comment, JSON.parse(event.body));
  const ticketInfo = (await db.get({
    TableName: 'tickets',
    Key: {
      id: `${event.pathParameters.projectId}-${event.pathParameters.ticketId}`,
      sort: 'detail'
    }
  }, (err, data) => {
    return err != null ? err : data;
  }).promise())['Item'];
  ticketInfo.detail.comment = (parseInt(ticketInfo.detail.comment, 10) + 1).toString();

  db.put({
    TableName: 'tickets',
    Item: {
      id: `${event.pathParameters.projectId}-${event.pathParameters.ticketId}`,
      sort: `comment-${ticketInfo.detail.comment}`,
      content: comment.content,
      detail: {
        owner: comment.owner,
        created_at: comment.created_at,
      },
    }
  }, (err, data) => {
    return err != null ? err : data;
  }).promise();

  db.update({
    TableName: 'tickets',
    Key: {
      id: `${event.pathParameters.projectId}-${event.pathParameters.ticketId}`,
      sort: 'detail',
    },
    AttributeUpdates: {
      detail: {
        Action: 'PUT',
        Value: ticketInfo.detail,
      }
    }
  }).promise();

  return {
    statusCode: 200,
    body: JSON.stringify(await result),
  };
};

module.exports.list = async (event, context) => {
  const result = db.query({
    TableName: 'tickets',
    KeyConditionExpression: 'id = :id and begins_with(sort, :sort)',
    ExpressionAttributeValues: {
      ':id': `${event.pathParameters.projectId}-${event.pathParameters.ticketId}`,
      ':sort': `comment`,
    }
  }, (err, data) => {
    return err != null ? err : data;
  }).promise();

  return {
    statusCode: 200,
    body: JSON.stringify(lib.validate({ type: 'array', items: lib.comment }, (await result)['Items'].map(item => {
      return {
        id: item.sort.split('-')[1],
        content: item.content,
        created_at: item.detail.created_at,
        owner: item.detail.owner,
        belongs_to: item.id,
      };
    }))),
  };
};
